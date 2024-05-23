#include    "joint-pneumo-hose-epb.h"

#include    "CfgReader.h"
#include    "physics.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
JointPneumoHoseEPB::JointPneumoHoseEPB() : Joint()
  , is_connected(false)
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
    Q_UNUSED(t)
    Q_UNUSED(dt)

    stepConnectionCheck();
    stepFlowEPBCalc();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void JointPneumoHoseEPB::stepConnectionCheck()
{
    // Управление соединением рукавов
    if (   (devices[FWD]->getOutputSignal(HOSE_OUTPUT_REF_STATE) == 1.0)
        || (devices[BWD]->getOutputSignal(HOSE_OUTPUT_REF_STATE) == 1.0) )
    {
        is_connected = true;
    }

    // Управление разъединением рукавов
    if (   (devices[FWD]->getOutputSignal(HOSE_OUTPUT_REF_STATE) == -1.0)
        || (devices[BWD]->getOutputSignal(HOSE_OUTPUT_REF_STATE) == -1.0) )
    {
        is_connected = false;
    }

    if (!is_connected)
    {
        devices[FWD]->setInputSignal(HOSE_INPUT_SIDE_ANGLE, 0.0);
        devices[FWD]->setInputSignal(HOSE_INPUT_DOWN_ANGLE, 0.0);
        devices[BWD]->setInputSignal(HOSE_INPUT_SIDE_ANGLE, 0.0);
        devices[BWD]->setInputSignal(HOSE_INPUT_DOWN_ANGLE, 0.0);
        return;
    }

    // Расчёт геометрии рукавов
    double length_fwd = devices[FWD]->getOutputSignal(HOSE_OUTPUT_LENGTH);
    double length_bwd = devices[BWD]->getOutputSignal(HOSE_OUTPUT_LENGTH);

    // Точками крепления рукавов и расстояния в треугольние между ними
    double coord_fwd = devices[FWD]->getOutputSignal(HOSE_OUTPUT_COORD);
    double coord_bwd = devices[BWD]->getOutputSignal(HOSE_OUTPUT_COORD);
    double coord_delta = max(0.001, coord_fwd - coord_bwd);
    double side_shift_fwd = devices[FWD]->getOutputSignal(HOSE_OUTPUT_SIDE);
    double side_shift_bwd = devices[BWD]->getOutputSignal(HOSE_OUTPUT_SIDE);
    double side_shift = side_shift_fwd + side_shift_bwd;

    // Расстояние между точками крепления рукавов по теореме Пифагора
    double length_quad = coord_delta * coord_delta + side_shift * side_shift;
    double length = sqrt(length_quad);

    // Неравенство треугольника
    if ( (length_fwd + length_bwd - length) < Physics::ZERO )
    {
        is_connected = false;
        devices[FWD]->setInputSignal(HOSE_INPUT_SIDE_ANGLE, 0.0);
        devices[FWD]->setInputSignal(HOSE_INPUT_DOWN_ANGLE, 0.0);
        devices[BWD]->setInputSignal(HOSE_INPUT_SIDE_ANGLE, 0.0);
        devices[BWD]->setInputSignal(HOSE_INPUT_DOWN_ANGLE, 0.0);
        return;
    }

    // Угол отклонения рукава в сторону соседнего
    double angle = acos( (length_quad + coord_delta * coord_delta - side_shift * side_shift)
                        / (2.0 * length * coord_delta) );
    devices[FWD]->setInputSignal(HOSE_INPUT_SIDE_ANGLE, sign(side_shift) * angle);
    devices[BWD]->setInputSignal(HOSE_INPUT_SIDE_ANGLE, sign(side_shift) * angle);

    // Угол свешивания рукава вниз
    angle = acos( (length_quad + length_fwd * length_fwd - length_bwd * length_bwd)
                 / (2.0 * length * length_fwd) );
    devices[FWD]->setInputSignal(HOSE_INPUT_DOWN_ANGLE, angle);
    angle = acos( (length_quad + length_bwd * length_bwd - length_fwd * length_fwd)
                 / (2.0 * length * length_bwd) );
    devices[BWD]->setInputSignal(HOSE_INPUT_DOWN_ANGLE, angle);
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
        devices[FWD]->setInputSignal(HOSE_INPUT_IS_CONNECTED, 1.0);
        devices[BWD]->setInputSignal(HOSE_INPUT_IS_CONNECTED, 1.0);

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
        devices[FWD]->setInputSignal(HOSE_INPUT_IS_CONNECTED, 0.0);
        devices[BWD]->setInputSignal(HOSE_INPUT_IS_CONNECTED, 0.0);

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
void JointPneumoHoseEPB::load_config(CfgReader &cfg)
{
    QString secName = "Joint";
    cfg.getBool(secName, "initConnectionState", is_connected);
}

GET_JOINT(JointPneumoHoseEPB)
