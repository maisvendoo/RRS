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

            reg_settings.insert(reg_setting.pos_num, reg_setting);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FieldRegulator::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    dP = getRefPower(omega) - U * I;

    double U_max = 0.0;

    dU = nf(U - U_max);

    double I_max = 0.0;

    dI = nf(I - I_max);

    double s = K[0] * dP + K[1] * Y[0] - K[2] * dU - K[3] * dI;

    double u = cut(s, 0.0, 1.0);

    Uf = u * U_fg;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FieldRegulator::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{
    Q_UNUSED(t)

    dYdt[0] = dP;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FieldRegulator::load_config(CfgReader &cfg)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double FieldRegulator::getRefPower(double omega)
{
    return 0.0;
}
