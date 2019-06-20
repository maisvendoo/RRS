#include    "chs4t.h"


#include    <CfgReader.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CHS4T::CHS4T() : Vehicle()
  , pantograph_config("chs4t-pantograph")
{
    for (size_t i = 0; i < NUM_PANTOGRAPHS; ++i)
    {
        pantographs[i] = new Pantograph();
        pantographs[i]->read_config(pantograph_config);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CHS4T::~CHS4T()
{

}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS4T::step(double t, double dt)
{
    pantographs[0]->setUks(25000);
    pantographs[1]->setUks(25000);


    for (size_t i = 0; i < NUM_PANTOGRAPHS; ++i)
        pantographs[i]->step(t, dt);

    DebugMsg = QString("t = %1 h1 = %2 U1 = %3 h2 = %4 U2 = %5")
            .arg(t, 10, 'f', 1)
            .arg(pantographs[0]->getH(), 4, 'f', 2)
            .arg(pantographs[0]->getUout(), 5, 'f', 0)
            .arg(pantographs[1]->getH(), 4, 'f', 2)
            .arg(pantographs[1]->getUout(), 5, 'f', 0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS4T::loadConfig(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {

    }
}

void CHS4T::keyProcess()
{
    if (getKeyState(KEY_O))
    {
        pantographs[0]->setState(isShift());
    }

    if (getKeyState(KEY_P))
    {
        pantographs[1]->setState(isShift());
    }


}

GET_VEHICLE(CHS4T)
