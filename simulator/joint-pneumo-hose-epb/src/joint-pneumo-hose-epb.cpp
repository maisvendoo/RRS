#include    "joint-pneumo-hose-epb.h"

#include    "CfgReader.h"
#include    "physics.h"
#include    "vec3.h"

#include    "device.h"
#include    "core/get_module.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
JointPneumoHoseEPB::JointPneumoHoseEPB() : Joint()
{
    devices.resize(NUM_CONNECTORS);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
JointPneumoHoseEPB::~JointPneumoHoseEPB()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void JointPneumoHoseEPB::step(double t, double dt)
{
    (void) t;
    (void) dt;

    stepConnectionCheck();
    stepFlowEPBCalc();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void JointPneumoHoseEPB::stepConnectionCheck()
{
    // Управление соединением рукавов
    const double sum_command =   devices[FWD]->getOutputSignal(HOSE_OUTPUT_REF_STATE)
                               + devices[BWD]->getOutputSignal(HOSE_OUTPUT_REF_STATE);
    if (sum_command >= 0.0)
    {
        ref_connected = true;
        if (sum_command >= 2.0)
        {
            // Оба рукава готовы соединиться
            is_connected = true;
        }
    }
    else
    {
        ref_connected = false;
    }

    // Если не соединены - то разъединены
    if (!ref_connected)
    {
        reset();
        apply();
        return;
    }

    // Расчёт геометрии рукавов
    const dvec3 new_coord_fwd = { devices[FWD]->getOutputSignal(HOSE_OUTPUT_ROOT_COORD_X),
                                 devices[FWD]->getOutputSignal(HOSE_OUTPUT_ROOT_COORD_Y),
                                 devices[FWD]->getOutputSignal(HOSE_OUTPUT_ROOT_COORD_Z) };
    const dvec3 new_coord_bwd = { devices[BWD]->getOutputSignal(HOSE_OUTPUT_ROOT_COORD_X),
                                 devices[BWD]->getOutputSignal(HOSE_OUTPUT_ROOT_COORD_Y),
                                 devices[BWD]->getOutputSignal(HOSE_OUTPUT_ROOT_COORD_Z) };
    if ((length2(coord_fwd - new_coord_fwd) < Physics::ZERO) && (length2(coord_bwd - new_coord_bwd) < Physics::ZERO))
    {
        // Ничего не изменилось, пересчитывать углы не надо
        return;
    }
    coord_fwd = new_coord_fwd;
    coord_bwd = new_coord_bwd;

    // Расстояния между точками крепления
    const double len_fwd = devices[FWD]->getOutputSignal(HOSE_OUTPUT_LENGTH_MIDDLE);
    const double len_head_fwd = devices[FWD]->getOutputSignal(HOSE_OUTPUT_LENGTH_HEAD);
    const double len_bwd = devices[BWD]->getOutputSignal(HOSE_OUTPUT_LENGTH_MIDDLE);
    const double len_head_bwd = devices[BWD]->getOutputSignal(HOSE_OUTPUT_LENGTH_HEAD);
    const dvec3 coord_delta = coord_bwd - coord_fwd;
    // Считаем головки жёстко соединенными параллельно прямой через точки крепления,
    // сразу вычитаем из расстояния их суммарную длину
    const double len = length(coord_delta) - len_head_fwd - len_head_bwd;

    // Неравенства треугольника - если рукава не дотягиваются, разъединяем
    if (   ((len_fwd + len_bwd - len) < Physics::ZERO)
        || ((len + len_fwd - len_bwd) < Physics::ZERO)
        || ((len_bwd + len - len_fwd) < Physics::ZERO))
    {
        reset();
        apply();
        return;
    }

    // Единичный вектор между точками крепления
    const dvec3 v_unit = coord_delta / (len + len_head_fwd + len_head_bwd);
    // Единичный вектор вниз
    const dvec3 v_down = {0.0, 0.0, -1.0};
    // Единичный вектор вдоль высоты треугольника в направлении вниз
    const dvec3 v_h = normalize(v_down - v_unit * dot(v_down, v_unit));

    // Проекции рукавов
    const double fwd_proj =   (len * len + len_fwd * len_fwd - len_bwd * len_bwd)
                            / (2.0 * len);
    const double bwd_proj =   (len * len + len_bwd * len_bwd - len_fwd * len_fwd)
                            / (2.0 * len);
    // Углы свешивания рукавов, они же углы поворота соединительной головки обратно вверх
    fwd_angle = std::acos(fwd_proj / len_fwd);
    bwd_angle = std::acos(bwd_proj / len_bwd);

    // Высота треугольника, по проекции одного из рукавов
    const double h = std::sqrt(len_fwd * len_fwd - fwd_proj * fwd_proj);
    // Рассчитываем точки крепления соединительных головок, куда будут стремится рукава
    point_fwd = coord_fwd + v_unit * fwd_proj + v_h * h;
    point_bwd = coord_bwd - v_unit * bwd_proj + v_h * h;

    apply();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void JointPneumoHoseEPB::stepFlowEPBCalc()
{
    // Расчёт потоков через рукава
    double p_fwd = devices[FWD]->getOutputSignal(HOSE_OUTPUT_PIPE_PRESSURE);
    double p_bwd = devices[BWD]->getOutputSignal(HOSE_OUTPUT_PIPE_PRESSURE);

    double flow_coeff_fwd = devices[FWD]->getOutputSignal(HOSE_OUTPUT_FLOW_COEFF);
    double flow_coeff_bwd = devices[BWD]->getOutputSignal(HOSE_OUTPUT_FLOW_COEFF);

    // Линии ЭПТ
    double lines_fwd = devices[FWD]->getOutputSignal(HOSE_OUTPUT_EPB_LINES_NUM);
    double lines_bwd = devices[BWD]->getOutputSignal(HOSE_OUTPUT_EPB_LINES_NUM);

    size_t signals_end_fwd = BEGIN_OF_EPB_LINE_SIGNALS
         + static_cast<size_t>(lines_fwd) * SIZE_OF_SIGNALS_PER_EPB_LINE;
    size_t signals_end_bwd = BEGIN_OF_EPB_LINE_SIGNALS
         + static_cast<size_t>(lines_bwd) * SIZE_OF_SIGNALS_PER_EPB_LINE;

    if (is_connected)
    {
        // Поток через соединённые рукава
        double flow_coeff = min(flow_coeff_fwd, flow_coeff_bwd);
        double flow = flow_coeff * (p_fwd - p_bwd);

        devices[FWD]->setInputSignal(HOSE_INPUT_FLOW_TO_PIPE, -flow);
        devices[BWD]->setInputSignal(HOSE_INPUT_FLOW_TO_PIPE, flow);

        // Соединённые контакты линий ЭПТ
        double num = min(lines_fwd, lines_bwd);
        devices[FWD]->setInputSignal(HOSE_INPUT_EPB_CONNECTED_NUM, num);
        devices[BWD]->setInputSignal(HOSE_INPUT_EPB_CONNECTED_NUM, num);
        epb_connected_num = static_cast<size_t>(num);

        size_t signals_end = BEGIN_OF_EPB_LINE_SIGNALS
             + epb_connected_num * SIZE_OF_SIGNALS_PER_EPB_LINE;

        if (epb_connected_num > 0)
            for (size_t i = BEGIN_OF_EPB_LINE_SIGNALS; i < signals_end; ++i)
            {
                devices[FWD]->setInputSignal(i, devices[BWD]->getOutputSignal(i));
                devices[BWD]->setInputSignal(i, devices[FWD]->getOutputSignal(i));
            }

        // Несоединённые контакты линий ЭПТ
        if (signals_end_fwd > signals_end)
            for (size_t i = signals_end; i < signals_end_fwd; ++i)
            {
                devices[FWD]->setInputSignal(i, 0.0);
            }

        if (signals_end_bwd > signals_end)
            for (size_t i = signals_end; i < signals_end_bwd; ++i)
            {
                devices[BWD]->setInputSignal(i, 0.0);
            }
    }
    else
    {
        // Поток из разъединённых рукавов в атмосферу
        double flow = flow_coeff_fwd * p_fwd;
        devices[FWD]->setInputSignal(HOSE_INPUT_FLOW_TO_PIPE, -flow);

        flow = flow_coeff_bwd * p_bwd;
        devices[BWD]->setInputSignal(HOSE_INPUT_FLOW_TO_PIPE, -flow);

        // Линии ЭПТ
        devices[FWD]->setInputSignal(HOSE_INPUT_EPB_CONNECTED_NUM, 0.0);
        devices[BWD]->setInputSignal(HOSE_INPUT_EPB_CONNECTED_NUM, 0.0);
        epb_connected_num = 0;

        // Несоединённые контакты линий ЭПТ
        for (size_t i = BEGIN_OF_EPB_LINE_SIGNALS; i < signals_end_fwd; ++i)
        {
            devices[FWD]->setInputSignal(i, 0.0);
        }

        for (size_t i = BEGIN_OF_EPB_LINE_SIGNALS; i < signals_end_bwd; ++i)
        {
            devices[BWD]->setInputSignal(i, 0.0);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void JointPneumoHoseEPB::reset()
{
    is_connected = false;
    ref_connected = false;
    coord_fwd = {0.0, 0.0, 0.0};
    coord_bwd = {0.0, 0.0, 0.0};
    fwd_angle = 0.0;
    bwd_angle = 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void JointPneumoHoseEPB::apply()
{
    devices[FWD]->setInputSignal(HOSE_INPUT_IS_CONNECTED, static_cast<double>(is_connected));
    devices[FWD]->setInputSignal(HOSE_INPUT_REF_CONNECTED, static_cast<double>(ref_connected));
    devices[FWD]->setInputSignal(HOSE_INPUT_HEAD_ROOT_COORD_X, point_fwd.x);
    devices[FWD]->setInputSignal(HOSE_INPUT_HEAD_ROOT_COORD_Y, point_fwd.y);
    devices[FWD]->setInputSignal(HOSE_INPUT_HEAD_ROOT_COORD_Z, point_fwd.z);
    devices[FWD]->setInputSignal(HOSE_INPUT_HEAD_ROOT_ANGLE_UP, fwd_angle);

    devices[BWD]->setInputSignal(HOSE_INPUT_IS_CONNECTED, static_cast<double>(is_connected));
    devices[BWD]->setInputSignal(HOSE_INPUT_REF_CONNECTED, static_cast<double>(ref_connected));
    devices[BWD]->setInputSignal(HOSE_INPUT_HEAD_ROOT_COORD_X, point_bwd.x);
    devices[BWD]->setInputSignal(HOSE_INPUT_HEAD_ROOT_COORD_Y, point_bwd.y);
    devices[BWD]->setInputSignal(HOSE_INPUT_HEAD_ROOT_COORD_Z, point_bwd.z);
    devices[BWD]->setInputSignal(HOSE_INPUT_HEAD_ROOT_ANGLE_UP, bwd_angle);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void JointPneumoHoseEPB::load_config(CfgReader &cfg)
{
    QString secName = "Joint";
    cfg.getBool(secName, "initConnectionState", is_connected);
    ref_connected = is_connected;
}

GET_MODULE(JointPneumoHoseEPB)
