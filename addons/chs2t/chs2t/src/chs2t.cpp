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

#include    "chs2t.h"
#include    "filesystem.h"

#include    <QDir>


//------------------------------------------------------------------------------
// Конструктор
//------------------------------------------------------------------------------
CHS2T::CHS2T() : Vehicle()
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

    tracForce_kN = 0;
    bv_return = false;

    Uks = 3000;
}

//------------------------------------------------------------------------------
// Деструктор
//------------------------------------------------------------------------------
CHS2T::~CHS2T()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initialization()
{
    FileSystem &fs = FileSystem::getInstance();
    QString modules_dir = QString(fs.getModulesDir().c_str());

    Uks = WIRE_VOLTAGE;
    current_kind = 1;

//    initPantographs();

//    initHighVoltageScheme();

//    initSupplyMachines();

//    initBrakeControls(modules_dir);

//    initBrakeMechanics();

//    initBrakeEquipment(modules_dir);

    initTractionControl();

//    initOtherEquipment();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initTractionControl()
{
    km21KR2 = new Km21KR2();

    stepSwitch = new StepSwitch();
//    stepSwitch->read_custom_config(config_dir + QDir::separator() + "ekg-8g");
//    connect(stepSwitch, &StepSwitch::soundPlay, this, &Engine::soundPlay);

//    gauge_KV_motors = new Oscillator();
//    gauge_KV_motors->read_custom_config(config_dir + QDir::separator() + "KV1-osc");

        motor = new Engine();
        motor->setCustomConfigDir(config_dir);
        motor->read_custom_config(config_dir + QDir::separator() + "AL-4846dT");

        puskRez = new PuskRez;
        puskRez->read_custom_config(config_dir + QDir::separator() + "puskrez");

        reg = new Registrator("trackforce", 0.1, Q_NULLPTR);
//        connect(motor[i], &Engine::soundSetPitch, this, &Engine::soundSetPitch);
//        connect(motor[i], &Engine::soundSetVolume, this, &Engine::soundSetVolume);

        overload_relay = new OverloadRelay();
        overload_relay->read_custom_config(config_dir + QDir::separator() + "1RPD6");
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::step(double t, double dt)
{
    pantographs[0]->setUks(Uks);
    pantographs[1]->setUks(Uks);

    bistV->setU_in(max(pantographs[0]->getUout(), pantographs[1]->getUout()));

    stepSwitch->setCtrlState(km21KR2->getCtrlState());

    for (size_t i = 0; i < NUM_PANTOGRAPHS; ++i)
        pantographs[i]->step(t, dt);

    bistV->setHoldingCoilState(getHoldingCoilState());
    bv_return = getHoldingCoilState() && bv_return;
    bistV->setReturn(bv_return);
    bistV->step(t, dt);

    km21KR2->setHod(stepSwitch->getHod());
    km21KR2->setControl(keys);
    km21KR2->step(t, dt);

    stepSwitch->setControl(keys);
    stepSwitch->step(t, dt);


    puskRez->setPoz(stepSwitch->getPoz());
    puskRez->step(t, dt);

    double ip = 1.75;

    motor->setDirection(km21KR2->getReverseState());
    motor->setBetaStep(km21KR2->getFieldStep());
    motor->setPoz(stepSwitch->getPoz());
    motor->setR(puskRez->getR());
    motor->setU(bistV->getU_out() * stepSwitch->getSchemeState());
    motor->setOmega(wheel_omega[0] * ip);
    motor->step(t, dt);

    tracForce_kN = 0;

    for (size_t i = 0; i <= Q_a.size(); ++i)
    {
        Q_a[i] = motor->getTorque() * ip;
        tracForce_kN += 2.0 * Q_a[i] / wheel_diameter / 1000.0;
    }

    overload_relay->setCurrent(motor->getIa());
    overload_relay->step(t, dt);



    DebugMsg = QString("t = %1 h1 = %2 U1 = %3 h2 = %4 U2 = %5 UGV = %6 poz = %7 Ia = %8 R = %9 beta = %10 re = %11" )
            .arg(t, 10, 'f', 1)
            .arg(pantographs[0]->getHeight(), 3, 'f', 2)
            .arg(pantographs[0]->getUout(), 4, 'f', 0)
            .arg(pantographs[1]->getHeight(), 3, 'f', 2)
            .arg(pantographs[1]->getUout(), 4, 'f', 0)
            .arg(bistV->getU_out(), 4, 'f', 0)
            .arg(stepSwitch->getPoz(), 2)
            .arg(motor->getIa(), 5)
            .arg(puskRez->getR(), 5)
            .arg(motor->getBeta(), 5)
            .arg(km21KR2->getReverseState(), 3);



//    DebugMsg = QString(" A2B2 = %1 C2D2 = %2 E2F2 = %3 I2G2 = %4 J2K2 = %5 poz = %6 v1 = %7 v2 = %8 shaft_rel = %9")
//    DebugMsg = QString(" poz = %1 R = %2")
//            .arg(stepSwitch->getPoz(), 3)
//            .arg(puskRez->getR(), 6);
    registrate(t, dt);
}

//------------------------------------------------------------------------------
// Загрузка данных из конфигурационного файла
//------------------------------------------------------------------------------
void CHS2T::loadConfig(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {

    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::keyProcess()
{
    if (getKeyState(KEY_O))
        pantographs[0]->setState(isShift());

    if (getKeyState(KEY_I))
        pantographs[1]->setState(isShift());

    if (getKeyState(KEY_P))
    {
        bistV->setState(isShift());
        bv_return = isShift();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::registrate(double t, double dt)
{
    QString msg = QString("%1 %2 %3 %4")
            .arg(t)
            .arg(velocity * Physics::kmh)
            .arg(tracForce_kN)
            .arg(motor->getIa());

    reg->print(msg, t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CHS2T::getHoldingCoilState() const
{
    bool no_overload = true;

    no_overload = no_overload && (!static_cast<bool>(overload_relay->getState()));

    bool state = no_overload;

    return state;
}

GET_VEHICLE(CHS2T)
