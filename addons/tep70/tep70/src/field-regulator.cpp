#include    "field-regulator.h"

#include    <QFile>
#include    <QTextStream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FieldRegulator::FieldRegulator(QObject *parent) : Device(parent)
  , dP(0.0)
  , dU(0.0)
  , dI(0.0)
  , U(0.0)
  , I(0.0)
  , omega(0.0)
  , U_fg(0.0)
  , Uf(0.0)
  , km_pos(0)
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

    Y[0] = cut(Y[0], 0.0, 1.0);
    Y[1] = cut(Y[1], 0.0, 1.0);
    Y[2] = cut(Y[2], 0.0, 1.0);

    dP = reg_settings[km_pos].P_ref * 1000.0 - U * I;

    double U_max = reg_settings[km_pos].U_max;

    dU = U - U_max;

    double I_max = reg_settings[km_pos].I_max;

    dI = I - I_max;

    double s = K[0] * dP + Y[0];

    double u = cut(cut(s, 0.0, 1.0) - Y[1] - Y[2], 0.0, 1.0);

    Uf = u * U_fg;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FieldRegulator::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{
    Q_UNUSED(t)

    dYdt[0] = K[1] * dP;
    dYdt[1] = K[2] * dU;
    dYdt[2] = K[3] * dI;
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


    return (p0.P_ref + (p1.P_ref - p0.P_ref) * (n - p0.n_ref) / (p1.n_ref - p0.n_ref)) * 1000.0;
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
