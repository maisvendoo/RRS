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
  , vehicle_path("../vehicles/chs2t/")
  , pantograph_config(vehicle_path + "pantograph")
  , gv_config(vehicle_path + "bv")
  , puskrez_config(vehicle_path + "puskrez")
{
    for (size_t i = 0; i < NUM_PANTOGRAPHS; ++i)
    {
        pantographs[i] = new Pantograph();
        pantographs[i]->read_custom_config(pantograph_config);
    }

    bistV = new ProtectiveDevice();
    bistV->read_custom_config(gv_config);

    puskRez = new PuskRez;
    puskRez->read_custom_config(puskrez_config);

    engine = new Engine;

    km21KR2 = new Km21KR2();

    stepSwitch = new StepSwitch();

    Uks = 3000;
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
    pantographs[0]->setUks(Uks);
    pantographs[1]->setUks(Uks);

    bistV->setU_in(max(pantographs[0]->getUout(), pantographs[1]->getUout()));

    stepSwitch->setCtrlState(km21KR2->getCtrlState());

    for (size_t i = 0; i < NUM_PANTOGRAPHS; ++i)
        pantographs[i]->step(t, dt);

    bistV->setHoldingCoilState(true);
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
            .arg(pantographs[0]->getHeight(), 4, 'f', 2)
            .arg(pantographs[0]->getUout(), 5, 'f', 0)
            .arg(pantographs[1]->getHeight(), 4, 'f', 2)
            .arg(pantographs[1]->getUout(), 5, 'f', 0)
            .arg(bistV->getU_out(), 5, 'f', 0)
            .arg(bistV->getKnifePos(), 5, 'f', 0);

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
        bistV->setState(isShift());

    bistV->setReturn(true);
}

GET_VEHICLE(CHS4T)
