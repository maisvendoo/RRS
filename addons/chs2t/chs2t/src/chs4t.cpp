//------------------------------------------------------------------------------
//
//      Магистральный пассажирский электровоз постоянного тока ЧС2т.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Николай Авилкин (avilkin.nick)
//
//      Дата: 21/08/2019
//
//------------------------------------------------------------------------------

#include    "chs4t.h"

//------------------------------------------------------------------------------
// Конструктор
//------------------------------------------------------------------------------
CHS4T::CHS4T() : Vehicle()
  , pantograph_config("chs2t-pantograph")
  , gv_config("chs2t-gv")
  , puskrez_config("chs2t-puskrez")
{
    for (size_t i = 0; i < NUM_PANTOGRAPHS; ++i)
    {
        pantographs[i] = new Pantograph();
        pantographs[i]->read_config(pantograph_config);
    }
    bistV = new BV();
    bistV->read_config(gv_config);

    puskRez = new PuskRez;
    puskRez->read_config(puskrez_config);

    engine = new Engine;

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

    bistV->setP0(0.5);
    bistV->setP1(0.3);
    bistV->setUkr(max(pantographs[0]->getUout(), pantographs[1]->getUout()));

    stepSwitch->setCtrlState(km21KR2->getCtrlState());

    for (size_t i = 0; i < NUM_PANTOGRAPHS; ++i)
        pantographs[i]->step(t, dt);

    bistV->step(t, dt);

    km21KR2->setControl(keys);
    km21KR2->step(t, dt);

    stepSwitch->step(t, dt);

    puskRez->setPoz(stepSwitch->getPoz());
    puskRez->step(t, dt);

    engine->setPoz(stepSwitch->getPoz());
    engine->setR(puskRez->getR());
    engine->step(t, dt);

    DebugMsg = QString("t = %1 h1 = %2 U1 = %3 h2 = %4 U2 = %5 UGV = %6 x = %7")
            .arg(t, 10, 'f', 1)
            .arg(pantographs[0]->getH(), 4, 'f', 2)
            .arg(pantographs[0]->getUout(), 5, 'f', 0)
            .arg(pantographs[1]->getH(), 4, 'f', 2)
            .arg(pantographs[1]->getUout(), 5, 'f', 0)
            .arg(bistV->getUout(), 5, 'f', 0)
            .arg(bistV->getX(), 5, 'f', 0);

//    DebugMsg = QString(" A2B2 = %1 C2D2 = %2 E2F2 = %3 I2G2 = %4 J2K2 = %5 poz = %6 v1 = %7 v2 = %8 shaft_rel = %9")
//    DebugMsg = QString(" poz = %1 R = %2")
//            .arg(stepSwitch->getPoz(), 3)
//            .arg(puskRez->getR(), 6);

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
        bistV->setBVState(isShift());

    if (getKeyState(KEY_J))
        bistV->setPhc(isShift());

    bistV->setVZState(getKeyState(KEY_K));


//glavV->setVZState(true);


}

GET_VEHICLE(CHS4T)
