//------------------------------------------------------------------------------
//
//      Магистральный пассажирский электровоз переменного тока ЧС4т.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Николай Авилкин (avilkin.nick)
//
//      Дата: 16/06/2019
//
//------------------------------------------------------------------------------

#include    "chs4t.h"

//------------------------------------------------------------------------------
// Конструктор
//------------------------------------------------------------------------------
CHS4T::CHS4T() : Vehicle()
  , pantograph_config("chs4t-pantograph")
  , gv_config("chs4t-gv")
{
    for (size_t i = 0; i < NUM_PANTOGRAPHS; ++i)
    {
        pantographs[i] = new Pantograph();
        pantographs[i]->read_config(pantograph_config);
    }
    glavV = new GV();
    glavV->read_config(gv_config);

    autoTrans = new AutoTransformer();
//    autoTrans->read_config();

    km21KR2 = new Km21KR2();
//    km21KR2->read_config();

    stepSwitch = new StepSwitch();
//    stepSwitch->read_config();
}

//------------------------------------------------------------------------------
// Деструктор
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

    glavV->setP0(0.5);
    glavV->setP1(0.3);
    glavV->setUkr(max(pantographs[0]->getUout(), pantographs[1]->getUout()));

    autoTrans->setUin(glavV->getUout());

    stepSwitch->setCtrlState(km21KR2->getCtrlState());

    for (size_t i = 0; i < NUM_PANTOGRAPHS; ++i)
        pantographs[i]->step(t, dt);

    glavV->step(t, dt);

    km21KR2->setControl(keys);
    km21KR2->step(t, dt);

    stepSwitch->step(t, dt);

    DebugMsg = QString("t = %1 h1 = %2 U1 = %3 h2 = %4 U2 = %5 UGV = %6 x = %7")
            .arg(t, 10, 'f', 1)
            .arg(pantographs[0]->getH(), 4, 'f', 2)
            .arg(pantographs[0]->getUout(), 5, 'f', 0)
            .arg(pantographs[1]->getH(), 4, 'f', 2)
            .arg(pantographs[1]->getUout(), 5, 'f', 0)
            .arg(glavV->getUout(), 5, 'f', 0)
            .arg(glavV->getX(), 5, 'f', 0);

    DebugMsg = QString(" A2B2 = %1 C2D2 = %2 E2F2 = %3 I2G2 = %4 J2K2 = %5")
            .arg(km21KR2->getCtrlState().a2b2, 5, 'f', 0)
            .arg(km21KR2->getCtrlState().c2d2, 5, 'f', 0)
            .arg(km21KR2->getCtrlState().e2f2, 5, 'f', 0)
            .arg(km21KR2->getCtrlState().i2g2, 5, 'f', 0)
            .arg(km21KR2->getCtrlState().j2k2, 5, 'f', 0);

}

//------------------------------------------------------------------------------
// Загрузка данных из конфигурационного файла
//------------------------------------------------------------------------------
void CHS4T::loadConfig(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {

    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS4T::keyProcess()
{
    if (getKeyState(KEY_O))
        pantographs[0]->setState(isShift());

    if (getKeyState(KEY_I))
        pantographs[1]->setState(isShift());

    if (getKeyState(KEY_P))
        glavV->setGVState(isShift());

    if (getKeyState(KEY_J))
        glavV->setPhc(isShift());

    glavV->setVZState(getKeyState(KEY_K));


//glavV->setVZState(true);


}

GET_VEHICLE(CHS4T)
