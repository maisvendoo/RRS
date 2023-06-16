#include    "field-regulator.h"

#include    <QFile>
#include    <QTextStream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FieldRegulator::FieldRegulator(QObject *parent) : Device(parent)
  , dP(0.0)
  , dP_prev(0.0)
  , dU(0.0)
  , dI(0.0)
  , dI_prev(0.0)
  , U(0.0)
  , I(0.0)
  , omega(0.0)
  , U_fg(0.0)
  , Uf(0.0)
  , Uf_prev(0.0)
  , km_pos(0)
  , Tp(1.0)
  , Ti(0.5)
  , Tu(0.5)
  , u(0.0)
  , T1(0.2)
  , is_active(false)
{
    std::fill(K.begin(), K.end(), 0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FieldRegulator::~FieldRegulator()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FieldRegulator::load_settings(QString file_path)
{
    QFile file(file_path);

    if (file.open(QIODevice::ReadOnly))
    {
        while (!file.atEnd())
        {
            QString line = file.readLine();

            if (line.isEmpty())
                continue;

            if (line[0] == ';')
                continue;

            QTextStream ss(&line);

            reg_settings_t reg_setting;

            ss >> reg_setting.pos_num
               >> reg_setting.n_ref
               >> reg_setting.P_ref
               >> reg_setting.U_max
               >> reg_setting.I_max;

            //reg_settings.insert(reg_setting.pos_num, reg_setting);
            reg_settings.push_back(reg_setting);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FieldRegulator::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    if (!is_active)
    {
        Y[0] = 0;
        u = 0;
        return;
    }

    // Уставки
    double I_max = reg_settings[km_pos].I_max;
    double U_max = reg_settings[km_pos].U_max;
    double P_max = reg_settings[km_pos].P_ref;

    if (U >= reg_settings[km_pos].P_ref * 1000.0 / reg_settings[km_pos].I_max)
    {
        if (I < reg_settings[km_pos].P_ref * 1000.0 / reg_settings[km_pos].U_max)
        {
            P_max = reg_settings[km_pos].U_max * I / 1000.0;
            I_max = I;
        }
        else
        {
            P_max = reg_settings[km_pos].P_ref;
            I_max = reg_settings[km_pos].P_ref * 1000.0 / U;
        }
    }
    else
    {
        I_max = reg_settings[km_pos].I_max;
    }

    // Ограничение интеграторов
    Y[0] = cut(Y[0], 0.0, 1.0);
    Y[1] = cut(Y[1], 0.0, 1.0);
    Y[2] = cut(Y[2], 0.0, 1.0);

    double P = U * I / 1000.0;

    // Рассогласования
    dI_prev = dI;
    dI = I_max - I;
    dU = U_max - U;
    dP_prev = dP;
    dP = P_max - P;

    double s1 = K[0] * dI + Y[0];

    double s2 = K[2] * nf(dP);

    double s3 = K[4] * nf(dU);

    double u_max = 1.0 / (1 + s2 + s3);

    u = cut(s1, 0.0, u_max);

    Uf = u * U_fg;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FieldRegulator::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    dYdt[0] = K[1] * dI;
    dYdt[1] = K[3] * dP;
    dYdt[2] = K[5] * dU;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FieldRegulator::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    for (size_t i = 0; i < K.size(); ++i)
    {
        QString param = QString("K%1").arg(i);
        cfg.getDouble(secName, param, K[i]);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double FieldRegulator::getRefPower(double omega)
{
    if (reg_settings.empty())
        return 0.0;

    if (reg_settings.size() < 2)
        return 0.0;

    double n = omega * 30.0 / Physics::PI;

    if (n < 350.0)
        return 0.0;

    reg_settings_t p1;
    reg_settings_t p0 = findPoint(omega, p1);


    return (p0.P_ref + (p1.P_ref - p0.P_ref) * (n - p0.n_ref) / (p1.n_ref - p0.n_ref));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FieldRegulator::reg_settings_t FieldRegulator::findPoint(double omega,
                                                         FieldRegulator::reg_settings_t &next_point)
{
    reg_settings_t p0;

    double n = omega * 30.0 / Physics::PI;

    int left_idx = 0;
    int right_idx = reg_settings.size() - 1;
    int idx = (left_idx + right_idx) / 2;

    while (idx != left_idx)
    {
        reg_settings_t p = reg_settings[idx];

        if (n <= p.n_ref)
            right_idx = idx;
        else
            left_idx = idx;

        idx = (left_idx + right_idx) / 2;
    }

    p0 = reg_settings[idx];
    next_point = reg_settings[idx + 1];

    return p0;
}
