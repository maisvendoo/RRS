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

#include    "chs2t-signals.h"


//------------------------------------------------------------------------------
// Конструктор
//------------------------------------------------------------------------------
CHS2T::CHS2T() : Vehicle()
{
    tracForce_kN = 0;
    bv_return = false;
    Uks = 3000;

    U_kr = 0;
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
        connect(pantographs[i], &Pantograph::soundPlay, this, &CHS2T::soundPlay);
    }

    pantographs[0]->setUks(Uks);
    pantographs[1]->setUks(Uks);

    for (size_t i = 0; i < NUM_PANTOGRAPHS; ++i)
    {
        pantoSwitcher[i] = new Switcher();
        pantoSwitcher[i]->setKolStates(4);
    }

    pantoSwitcher[0]->setKeyCode(KEY_I);
    pantoSwitcher[1]->setKeyCode(KEY_O);
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

    fastSwitchSw = new Switcher();
    fastSwitchSw->setKeyCode(KEY_P);
    fastSwitchSw->setKolStates(4);
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
    stepSwitch->read_custom_config(config_dir + QDir::separator() + "step-switch");

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

    pnSplit = new PneumoSplitter();
    pnSplit->read_config("pneumo-splitter");

    rd304 = new PneumoReley();
    rd304->read_config("rd304");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initOtherEquipment()
{
    horn = new TrainHorn();
    connect(horn, &TrainHorn::soundSetVolume, this, &CHS2T::soundSetVolume);
}

//------------------------------------------------------------------------------
// Инициализация регистратора
//------------------------------------------------------------------------------
void CHS2T::initRegistrator()
{
    //reg = new Registrator("trackforce", 0.1, Q_NULLPTR);
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

    initOtherEquipment();

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
    // Управление разъединителями токоприемников
    for (size_t i = 0; i < NUM_PANTOGRAPHS; ++i)
    {
        pantoSwitcher[i]->setControl(keys);

        if (pantoSwitcher[i]->getState() == 3)
            pant_switch[i].set();

        if (pantoSwitcher[i]->getState() == 0)
            pant_switch[i].reset();

        if (pantoSwitcher[i]->getState() == 2 && pant_switch[i].getState())
            pantup_trigger[i].set();

        if (pantoSwitcher[i]->getState() == 1)
            pantup_trigger[i].reset();

        pantoSwitcher[i]->step(t, dt);

        // Подъем/опускание ТП
        pantographs[i]->setState(pant_switch[i].getState() && pantup_trigger[i].getState());

        pantographs[i]->step(t, dt);
    }
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
    U_kr = max(pantographs[0]->getUout() * pant_switch[0].getState() ,
            pantographs[1]->getUout() * pant_switch[1].getState());

    bistV->setU_in(U_kr);

    bistV->setState(fast_switch_trigger.getState());
    bistV->setHoldingCoilState(getHoldingCoilState());
    bv_return = getHoldingCoilState() && bv_return;
    bistV->setReturn(bv_return);
    bistV->step(t, dt);

    if (fastSwitchSw->getState() == 3)
    {
        fast_switch_trigger.set();
        bv_return = true;
    }

    if (fastSwitchSw->getState() == 1)
    {
        fast_switch_trigger.reset();
        bv_return = false;
    }

    fastSwitchSw->setControl(keys);
    fastSwitchSw->step(t, dt);
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
    motor->setAmpermetersState(stepSwitch->getAmpermetersState());
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
    dako->setPkvt(zpk->getPressure2());

    locoCrane->setFeedlinePressure(mainReservoir->getPressure());
    locoCrane->setBrakeCylinderPressure(zpk->getPressure2());
    locoCrane->setControl(keys);

    zpk->setInputFlow1(dako->getQtc());
    zpk->setInputFlow2(locoCrane->getBrakeCylinderFlow());
    zpk->setOutputPressure(pnSplit->getP_in());

    brakesMech[0]->setAirFlow(pnSplit->getQ_out1());

    airDistr->setBrakeCylinderPressure(dako->getPy());
    airDistr->setBrakepipePressure(pTM);
    airDistr->setAirSupplyPressure(spareReservoir->getPressure());

    spareReservoir->setAirFlow(airDistr->getAirSupplyFlow());

    rd304->setBrakeCylPressure(brakesMech[1]->getBrakeCylinderPressure());
    rd304->setPipelinePressure(mainReservoir->getPressure());
    rd304->setWorkAirFlow(pnSplit->getQ_out2());

    brakesMech[1]->setAirFlow(rd304->getBrakeCylAirFlow());

    pnSplit->setP_out2(rd304->getWorkPressure());
    pnSplit->setP_out1(brakesMech[0]->getBrakeCylinderPressure());
    pnSplit->setQ_in(zpk->getOutputFlow());

    dako->step(t, dt);
    locoCrane->step(t, dt);
    zpk->step(t, dt);
    airDistr->step(t, dt);
    spareReservoir->step(t, dt);
    pnSplit->step(t, dt);
    rd304->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepDebugMsg(double t, double dt)
{
    Q_UNUSED(dt)

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
void CHS2T::stepSignals()
{
    analogSignal[STRELKA_POS] = static_cast<float>(stepSwitch->getPoz()) / 42.0f;

    analogSignal[STRELKA_AMP1] = static_cast<float>(motor->getI12() / 1000.0);
    analogSignal[STRELKA_AMP2] = static_cast<float>(motor->getI34() / 1000.0);
    analogSignal[STRELKA_AMP3] = static_cast<float>(motor->getI56() / 1000.0);

    analogSignal[STRELKA_PM] = static_cast<float>(mainReservoir->getPressure() / 1.6);
    analogSignal[STRELKA_TC] = static_cast<float>(brakesMech[0]->getBrakeCylinderPressure() / 1.0);
    analogSignal[STRELKA_EDT] = 0.0f;
    analogSignal[STRELKA_UR] = static_cast<float>(brakeCrane->getEqReservoirPressure() / 1.0);
    analogSignal[STRELKA_TM] = static_cast<float>(pTM / 1.0);

    analogSignal[STRELKA_UKS] = static_cast<float>(U_kr / 4000.0);

    analogSignal[KRAN395_RUK] = static_cast<float>(brakeCrane->getHandlePosition());
    analogSignal[KRAN254_RUK] = static_cast<float>(locoCrane->getHandlePosition());

    analogSignal[KONTROLLER] = static_cast<float>(km21KR2->getMainShaftPos());
    analogSignal[REVERSOR] = static_cast<float>(km21KR2->getReverseState());

    analogSignal[SIGLIGHT_P] = static_cast<float>(stepSwitch->isParallel());
    analogSignal[SIGLIGHT_SP] = static_cast<float>(stepSwitch->isSeriesParallel());
    analogSignal[SIGLIGHT_S] = static_cast<float>(stepSwitch->isSeries());
    analogSignal[SIGLIGHT_ZERO] = static_cast<float>(stepSwitch->isZero());

    analogSignal[SIGLIGHT_NO_BRAKES_RELEASE] = static_cast<float>(brakesMech[0]->getBrakeCylinderPressure() >= 0.1);

    analogSignal[PANT1] = static_cast<float>(pantographs[0]->getHeight());
    analogSignal[PANT2] = static_cast<float>(pantographs[1]->getHeight());

    analogSignal[SW_PNT1] = pantoSwitcher[0]->getHandlePos();
    analogSignal[SW_PNT2] = pantoSwitcher[1]->getHandlePos();

    analogSignal[SIGLIGHT_RAZED] = static_cast<float>( !pant_switch[0].getState() && !pant_switch[1].getState() );

    analogSignal[INDICATOR_BV] = static_cast<float>(bistV->getLampState());

    analogSignal[SW_BV] = fastSwitchSw->getHandlePos();

    analogSignal[WHEEL_1] = static_cast<float>(dir * wheel_rotation_angle[0] / 2.0 / Physics::PI);
    analogSignal[WHEEL_2] = static_cast<float>(dir * wheel_rotation_angle[1] / 2.0 / Physics::PI);
    analogSignal[WHEEL_3] = static_cast<float>(dir * wheel_rotation_angle[2] / 2.0 / Physics::PI);
    analogSignal[WHEEL_4] = static_cast<float>(dir * wheel_rotation_angle[3] / 2.0 / Physics::PI);
    analogSignal[WHEEL_5] = static_cast<float>(dir * wheel_rotation_angle[4] / 2.0 / Physics::PI);
    analogSignal[WHEEL_6] = static_cast<float>(dir * wheel_rotation_angle[5] / 2.0 / Physics::PI);
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

    stepSignals();

    //registrate(t, dt);

    horn->setControl(keys);
    horn->step(t, dt);
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
