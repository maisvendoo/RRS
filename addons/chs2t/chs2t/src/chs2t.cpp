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
// Инициализация токоприемников
//------------------------------------------------------------------------------
void CHS2T::initPantographs()
{
    for (size_t i = 0; i < NUM_PANTOGRAPHS; ++i)
    {
        pantographs[i] = new Pantograph();
        pantographs[i]->read_custom_config(config_dir + QDir::separator() + "pantograph");
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initBrakesMech()
{
    brakesMech[0] = new CHS2tBrakeMech();
    brakesMech[0]->read_custom_config(config_dir + QDir::separator() + "brake-mech-front");

    brakesMech[1] = new CHS2tBrakeMech();
    brakesMech[1]->read_custom_config(config_dir + QDir::separator() + "brake-mech-back");
}

//------------------------------------------------------------------------------
// Инициализация быстродействующего выключателя
//------------------------------------------------------------------------------
void CHS2T::initFastSwitch()
{
    bistV = new ProtectiveDevice();
    bistV->read_custom_config(config_dir + QDir::separator() + "bv");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initProtection()
{
    overload_relay = new OverloadRelay();
    overload_relay->read_custom_config(config_dir + QDir::separator() + "1RPD6");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initBrakesControl(QString module_path)
{
    brakeCrane = loadBrakeCrane(module_path + QDir::separator() + "krm395");
    brakeCrane->read_config("krm395");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initAirSupplySubsystem()
{
    mainReservoir = new Reservoir(1);
    spareReservoir = new Reservoir(0.078);

    motor_compressor = new DCMotorCompressor();
    motor_compressor->read_custom_config(config_dir + QDir::separator() + "motor-compressor");
    pressReg = new PressureRegulator();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initTractionControl()
{
    km21KR2 = new Km21KR2();

    stepSwitch = new StepSwitch();

    motor = new Engine();
    motor->setCustomConfigDir(config_dir);
    motor->read_custom_config(config_dir + QDir::separator() + "AL-4846dT");

    puskRez = new PuskRez;
    puskRez->read_custom_config(config_dir + QDir::separator() + "puskrez");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initBrakesEquipment(QString module_path)
{
    dako = new Dako();
    dako->read_custom_config(config_dir + QDir::separator() + "dako");

    airDistr = loadAirDistributor(module_path + QDir::separator() + "vr242");
    airDistr->read_config("vr242");

    locoCrane = loadLocoCrane(module_path + QDir::separator() + "kvt254");
    locoCrane->read_config("kvt254");

    zpk = new SwitchingValve();
    zpk->read_config("zpk");

}



//------------------------------------------------------------------------------
// Инициализация регистратора
//------------------------------------------------------------------------------
void CHS2T::initRegistrator()
{
    reg = new Registrator("trackforce", 0.1, Q_NULLPTR);
}


//------------------------------------------------------------------------------
// Общая инициализация локомотива
//------------------------------------------------------------------------------
void CHS2T::initialization()
{
    FileSystem &fs = FileSystem::getInstance();
    QString modules_dir = QString(fs.getModulesDir().c_str());

    Uks = WIRE_VOLTAGE;
    current_kind = 1;

    initPantographs();

    initBrakesMech();

    initFastSwitch();

    initProtection();

    initBrakesControl(modules_dir);

    initAirSupplySubsystem();

    initTractionControl();

    initBrakesEquipment(modules_dir);

    initRegistrator();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initBrakeDevices(double p0, double pTM, double pFL)
{
    charging_press = p0;

    mainReservoir->setY(0, pFL);
    spareReservoir->setY(0, charging_press);
    brakeCrane->init(pTM, pFL);
    locoCrane->init(pTM, pFL);
    airDistr->init(pTM, pFL);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepPantographs(double t, double dt)
{
    pantographs[0]->setUks(Uks);
    pantographs[1]->setUks(Uks);

    for (size_t i = 0; i < NUM_PANTOGRAPHS; ++i)
        pantographs[i]->step(t, dt);
}



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepBrakesMech(double t, double dt)
{
        brakesMech[0]->step(t, dt);
        brakesMech[1]->step(t, dt);

        Q_r[1] = brakesMech[0]->getBrakeTorque();
        Q_r[2] = brakesMech[0]->getBrakeTorque();
        Q_r[3] = brakesMech[0]->getBrakeTorque();

        Q_r[4] = brakesMech[1]->getBrakeTorque();
        Q_r[5] = brakesMech[1]->getBrakeTorque();
        Q_r[6] = brakesMech[1]->getBrakeTorque();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepFastSwitch(double t, double dt)
{
    bistV->setU_in(max(pantographs[0]->getUout(), pantographs[1]->getUout()));
    bistV->setHoldingCoilState(getHoldingCoilState());
    bv_return = getHoldingCoilState() && bv_return;
    bistV->setReturn(bv_return);
    bistV->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepProtection(double t, double dt)
{
    overload_relay->setCurrent(motor->getIa());
    overload_relay->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepTractionControl(double t, double dt)
{
    double ip = 1.75;


    km21KR2->setHod(stepSwitch->getHod());
    km21KR2->setControl(keys);
    km21KR2->step(t, dt);

    stepSwitch->setCtrlState(km21KR2->getCtrlState());
    stepSwitch->setControl(keys);
    stepSwitch->step(t, dt);

    puskRez->setPoz(stepSwitch->getPoz());
    puskRez->step(t, dt);

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
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepAirSupplySubsystem(double t, double dt)
{
    motor_compressor->setU(bistV->getU_out() * static_cast<double>(mk_tumbler.getState()) * pressReg->getState());
    motor_compressor->setPressure(mainReservoir->getPressure());
    motor_compressor->step(t, dt);

    mainReservoir->setAirFlow(motor_compressor->getAirFlow());
    mainReservoir->step(t, dt);

    pressReg->setPressure(mainReservoir->getPressure());
    pressReg->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepBrakesControl(double t, double dt)
{
    brakeCrane->setChargePressure(charging_press);
    brakeCrane->setFeedLinePressure(mainReservoir->getPressure());
    brakeCrane->setBrakePipePressure(pTM);
    brakeCrane->setControl(keys);
    p0 = brakeCrane->getBrakePipeInitPressure();
    brakeCrane->step(t, dt);
}

void CHS2T::stepBrakesEquipment(double t, double dt)
{
    dako->setPgr(mainReservoir->getPressure());
    dako->setPtc(zpk->getPressure1());
    dako->setQvr(airDistr->getBrakeCylinderAirFlow());
    dako->setU(velocity);
    //dako->setQ1(locoCrane->getBrakeCylinderFlow());
    dako->setPkvt(zpk->getPressure2());

    locoCrane->setFeedlinePressure(mainReservoir->getPressure());
    locoCrane->setBrakeCylinderPressure(zpk->getPressure2());
    locoCrane->setControl(keys);

    zpk->setInputFlow1(dako->getQtc());
    zpk->setInputFlow2(locoCrane->getBrakeCylinderFlow());
    zpk->setOutputPressure(brakesMech[0]->getBrakeCylinderPressure());

    brakesMech[0]->setAirFlow(zpk->getOutputFlow());

    airDistr->setBrakeCylinderPressure(dako->getPy());
    airDistr->setBrakepipePressure(pTM);
    airDistr->setAirSupplyPressure(spareReservoir->getPressure());

    spareReservoir->setAirFlow(airDistr->getAirSupplyFlow());

    dako->step(t, dt);
    locoCrane->step(t, dt);
    zpk->step(t, dt);
    airDistr->step(t, dt);
    spareReservoir->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepDebugMsg(double t, double dt)
{
    DebugMsg = QString("t = %1 UGV = %2 poz = %3 Ia = %4  re = %5 press = %6 pQ = %7 pTM = %8 state = %9 K = %10 V = %11" )
        .arg(t, 10, 'f', 1)
        .arg(bistV->getU_out(), 4, 'f', 0)
        .arg(stepSwitch->getPoz(), 2)
        .arg(motor->getIa(), 5, 'f', 2)
        .arg(km21KR2->getReverseState(), 2)
        .arg(mainReservoir->getPressure(), 4, 'f', 2)
        .arg(brakeCrane->getEqReservoirPressure(), 3, 'f', 2)
        .arg(brakeCrane->getBrakePipeInitPressure(), 3, 'f', 2)
        .arg(brakeCrane->getPositionName(), 3)
        .arg(brakesMech[0]->getShoeForce() / 1000, 3, 'f', 2)
        .arg(velocity * Physics::kmh, 3, 'f', 2);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::step(double t, double dt)
{
    stepPantographs(t, dt);

    stepFastSwitch(t, dt);

    stepTractionControl(t, dt);

    stepProtection(t, dt);

    stepAirSupplySubsystem(t, dt);

    stepBrakesControl(t, dt);

    stepBrakesMech(t , dt);

    stepBrakesEquipment(t, dt);

    stepDebugMsg(t, dt);

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
