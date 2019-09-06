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

    for (size_t i = 0; i < NUM_PANTOGRAPHS; ++i)
    {
        pantographs[i] = new Pantograph();
        pantographs[i]->read_custom_config(pantograph_config);
    }
    for (size_t i = 0; i < NUM_BRAKE_MECH; ++i)
    {
        brakeMechs[i] = new CHS2tBrakeMech();
    }

    brakeMechs[0]->read_custom_config(config_dir + QDir::separator() + "brake-mech-front");
    brakeMechs[1]->read_custom_config(config_dir + QDir::separator() + "brake-mech-back");

    bistV = new ProtectiveDevice();
    bistV->read_custom_config(gv_config);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initBrakeDevices(double p0, double pTM, double pFL)
{
    charging_press = p0;
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

        mainReservoir = new Reservoir(1);
        motor_compressor = new DCMotorCompressor();
        motor_compressor->read_custom_config(config_dir + QDir::separator() + "motor-compressor");
        pressReg = new PressureRegulator();

        FileSystem &fs = FileSystem::getInstance();
        QString module_path = QString(fs.getModulesDir().c_str());

        brakeCrane = loadBrakeCrane(module_path + QDir::separator() + "krm395");
        brakeCrane->read_config("krm395");
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

    motor_compressor->setU(bistV->getU_out() * static_cast<double>(mk_tumbler.getState()) * pressReg->getState());
    motor_compressor->setPressure(mainReservoir->getPressure());
    motor_compressor->step(t, dt);


    mainReservoir->setAirFlow(motor_compressor->getAirFlow());
    mainReservoir->step(t, dt);

    pressReg->setPressure(mainReservoir->getPressure());
    pressReg->step(t, dt);


    brakeCrane->setChargePressure(charging_press);
    brakeCrane->setFeedLinePressure(mainReservoir->getPressure());
    brakeCrane->setBrakePipePressure(pTM);
    brakeCrane->setControl(keys);
    p0 = brakeCrane->getBrakePipeInitPressure();
    brakeCrane->step(t, dt);


    for (size_t i = 0; i < NUM_BRAKE_MECH; ++i)
        brakeMechs[i]->step(t, dt);

    Q_r[1] = brakeMechs[0]->getBrakeTorque();
    Q_r[2] = brakeMechs[0]->getBrakeTorque();
    Q_r[3] = brakeMechs[0]->getBrakeTorque();

    Q_r[4] = brakeMechs[1]->getBrakeTorque();
    Q_r[5] = brakeMechs[1]->getBrakeTorque();
    Q_r[6] = brakeMechs[1]->getBrakeTorque();


    DebugMsg = QString("t = %1 UGV = %2 poz = %3 Ia = %4  re = %5 press = %6 eqResPress = %7 pipePress = %8 state = %9 shForce = %10" )
            .arg(t, 10, 'f', 1)
//            .arg(pantographs[0]->getHeight(), 3, 'f', 2)
//            .arg(pantographs[0]->getUout(), 4, 'f', 0)
//            .arg(pantographs[1]->getHeight(), 3, 'f', 2)
//            .arg(pantographs[1]->getUout(), 4, 'f', 0)
            .arg(bistV->getU_out(), 4, 'f', 0)
            .arg(stepSwitch->getPoz(), 2)
            .arg(motor->getIa(), 5)
//            .arg(puskRez->getR(), 5)
//            .arg(motor->getBeta(), 5)
            .arg(km21KR2->getReverseState(), 2)
            .arg(mainReservoir->getPressure(), 4, 'f', 2)
            .arg(brakeCrane->getEqReservoirPressure(), 5, 'f', 2)
            .arg(brakeCrane->getBrakePipeInitPressure(), 5, 'f', 2)
            .arg(brakeCrane->getPositionName(), 3)
            .arg(brakeMechs[0]->getShoeForce() / 1000, 5, 'f', 2);




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

    if (getKeyState(KEY_8))
    {
        if (isShift())
            mk_tumbler.set();
        else
        {
            mk_tumbler.reset();
        }
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
