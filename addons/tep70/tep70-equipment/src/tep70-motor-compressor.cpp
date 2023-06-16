#include    "tep70-motor-compressor.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TEP70MotorCompressor::TEP70MotorCompressor(QObject *parent)
  : DCMotorCompressor(parent)
  , R0(0.089)
  , rd(0.0)
{
    R = 0.0;
    omega0 = 104.7;
    J = 2.0;
    cPhi = 0.85;
    K_pressure = 0.01;
    K_flow = 0.005;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TEP70MotorCompressor::~TEP70MotorCompressor()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70MotorCompressor::step(double t, double dt)
{
    rd = 0.0;

    for (size_t i = 0; i < Rd.size(); ++i)
    {
        rd += static_cast<double>(kontaktor_step[i]) * Rd[i];
    }

    R = R0 + rd;

    DCMotorCompressor::step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70MotorCompressor::setKontaktorState(size_t i, bool state)
{
    if (i < kontaktor_step.size())
        kontaktor_step[i] = !state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70MotorCompressor::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    QString rd_list = "";
    cfg.getString(secName, "Rd", rd_list);

    if (!rd_list.isEmpty())
    {
        QStringList resistors = rd_list.split(' ');

        for (QString res : resistors)
        {
            Rd.push_back(res.toDouble());
            kontaktor_step.push_back(true);
        }
    }

    cfg.getDouble(secName, "omega0", omega0);
    cfg.getDouble(secName, "R", R0);
    cfg.getDouble(secName, "cPhi", cPhi);
    cfg.getDouble(secName, "J", J);
    cfg.getDouble(secName, "Mc", Mxx);

    cfg.getDouble(secName, "K_pressure", K_pressure);
    cfg.getDouble(secName, "K_flow", K_flow);

    cfg.getString(secName, "SoundName", soundName);
    cfg.getBool(secName, "RegulateSoundByOnOff", reg_sound_by_on_off);
    cfg.getBool(secName, "RegulateSoundByPitch", reg_sound_by_pitch);
}
