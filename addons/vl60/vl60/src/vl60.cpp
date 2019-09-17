//------------------------------------------------------------------------------
//
//      Магистральный электровоз переменного тока ВЛ60.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Роман Бирюков (РомычРЖДУЗ)
//
//      Дата: 28/03/2019
//
//------------------------------------------------------------------------------

#include    "vl60.h"
#include    "filesystem.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VL60::VL60() : Vehicle ()
  , pant1_pos(0.0)
  , pant2_pos(0.0)
  , gv_pos(0.0)
  , gv_return(false)  
  , charge_press(0.0)
  , ip(2.73)
{
    pants_tumbler.setOnSoundName("K_Tumbler_On");
    pants_tumbler.setOffSoundName("K_Tumbler_Off");
    connect(&pants_tumbler, &Trigger::soundPlay, this, &VL60::soundPlay);

    pant1_tumbler.setOnSoundName("K_Tumbler_On");
    pant1_tumbler.setOffSoundName("K_Tumbler_Off");
    connect(&pant1_tumbler, &Trigger::soundPlay, this, &VL60::soundPlay);

    pant2_tumbler.setOnSoundName("K_Tumbler_On");
    pant2_tumbler.setOffSoundName("K_Tumbler_Off");
    connect(&pant2_tumbler, &Trigger::soundPlay, this, &VL60::soundPlay);

    gv_tumbler.setOnSoundName("K_Tumbler_On");
    gv_tumbler.setOffSoundName("K_Tumbler_Off");
    connect(&gv_tumbler, &Trigger::soundPlay, this, &VL60::soundPlay);

    gv_return_tumbler.setOnSoundName("K_Tumbler_Nofixed_On");
    gv_return_tumbler.setOffSoundName("K_Tumbler_Nofixed_Off");
    connect(&gv_return_tumbler, &Trigger::soundPlay, this, &VL60::soundPlay);

    fr_tumbler.setOnSoundName("K_Tumbler_On");
    fr_tumbler.setOffSoundName("K_Tumbler_Off");
    connect(&fr_tumbler, &Trigger::soundPlay, this, &VL60::soundPlay);

    for (size_t i = 0; i < mv_tumblers.size(); ++i)
    {
        mv_tumblers[i].setOnSoundName("K_Tumbler_On");
        mv_tumblers[i].setOffSoundName("K_Tumbler_Off");
        connect(&mv_tumblers[i], &Trigger::soundPlay, this, &VL60::soundPlay);
    }

    mk_tumbler.setOnSoundName("K_Tumbler_On");
    mk_tumbler.setOffSoundName("K_Tumbler_Off");
    connect(&mk_tumbler, &Trigger::soundPlay, this, &VL60::soundPlay);

    cu_tumbler.setOnSoundName("K_Tumbler_On");
    cu_tumbler.setOffSoundName("K_Tumbler_Off");
    connect(&cu_tumbler, &Trigger::soundPlay, this, &VL60::soundPlay);

    for (size_t i = 0; i < rb.size(); ++i)
    {
        rb[i].setOnSoundName("RB_Down");
        rb[i].setOffSoundName("RB_Up");
        connect(&rb[i], &Trigger::soundPlay, this, &VL60::soundPlay);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VL60::~VL60()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::initBrakeDevices(double p0, double pTM, double pFL)
{
    main_reservoir->setY(0, pFL);
    charge_press = p0;

    load_brakes_config(config_dir + QDir::separator() + "brakes-init.xml");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::initPantographs()
{
    QString pant_cfg_path = config_dir + QDir::separator() + "pantograph";

    for (size_t i = 0; i < NUM_PANTOGRAPHS; ++i)
    {
        pantographs[i] = new Pantograph();
        pantographs[i]->read_custom_config(pant_cfg_path);
        connect(pantographs[i], &Pantograph::soundPlay, this, &VL60::soundPlay);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::initHighVoltageScheme()
{
    QString gv_cfg_path = config_dir + QDir::separator() + "main-switch";

    main_switch = new ProtectiveDevice();
    main_switch->read_custom_config(gv_cfg_path);
    connect(main_switch, &ProtectiveDevice::soundPlay, this, &VL60::soundPlay);

    gauge_KV_ks = new Oscillator();
    gauge_KV_ks->read_config("oscillator");

    trac_trans = new TracTransformer();
    trac_trans->read_custom_config(config_dir + QDir::separator() + "trac-transformer");
    connect(trac_trans, &TracTransformer::soundSetVolume, this, &VL60::soundSetVolume);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::initSupplyMachines()
{
    phase_spliter = new PhaseSplitter();
    connect(phase_spliter, &PhaseSplitter::soundSetPitch, this, &VL60::soundSetPitch);

    for (size_t i = 0; i < motor_fans.size(); ++i)
    {
        motor_fans[i] = new MotorFan(i + 1);
        connect(motor_fans[i], &MotorFan::soundSetPitch, this, &VL60::soundSetPitch);
    }

    main_reservoir = new Reservoir(static_cast<double>(MAIN_RESERVOIR_VOLUME) / 1000.0);

    QString mk_cfg_path = config_dir + QDir::separator() + "motor-compressor.xml";
    motor_compressor = new MotorCompressor(mk_cfg_path);
    connect(motor_compressor, &MotorCompressor::soundSetPitch, this, &VL60::soundSetPitch);

    press_reg = new PressureRegulator();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::initBrakeControls(QString modules_dir)
{
    ubt = new BrakeLock();
    ubt->read_config("ubt367m");
    connect(ubt, &BrakeLock::soundPlay, this, &VL60::soundPlay);

    brake_crane = loadBrakeCrane(modules_dir + QDir::separator() + "krm395");
    brake_crane->read_config("krm395");
    connect(brake_crane, &BrakeCrane::soundPlay, this, &VL60::soundPlay);

    loco_crane = loadLocoCrane(modules_dir + QDir::separator() + "kvt254");
    loco_crane->read_config("kvt254");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::initBrakeMechanics()
{
    trolley_mech[TROLLEY_FWD] = new TrolleyBrakeMech(config_dir +
                                           QDir::separator() +
                                           "fwd-trolley-brake-mech.xml");

    trolley_mech[TROLLEY_BWD] = new TrolleyBrakeMech(config_dir +
                                           QDir::separator() +
                                           "bwd-trolley-brake-mech.xml");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::initBrakeEquipment(QString modules_dir)
{
    switch_valve = new SwitchingValve();
    switch_valve->read_config("zpk");

    pneumo_relay = new PneumoReley();
    pneumo_relay->read_config("rd304");

    pneumo_splitter = new PneumoSplitter();
    pneumo_splitter->read_config("pneumo-splitter");

    supply_reservoir = new Reservoir(0.078);

    air_disr = loadAirDistributor(modules_dir + QDir::separator() + "vr242");
    air_disr->read_config("vr242");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::initTractionControl()
{
    controller = new ControllerKME_60_044();

    main_controller = new EKG_8G();
    main_controller->read_custom_config(config_dir + QDir::separator() + "ekg-8g");
    connect(main_controller, &EKG_8G::soundPlay, this, &VL60::soundPlay);

    for (size_t i = 0; i < vu.size(); ++i)
    {
        vu[i] = new Rectifier();
        vu[i]->read_custom_config(config_dir + QDir::separator() + "VU");
    }

    gauge_KV_motors = new Oscillator();
    gauge_KV_motors->read_custom_config(config_dir + QDir::separator() + "KV1-osc");

    for (size_t i = 0; i < motor.size(); ++i)
    {
        motor[i] = new DCMotor();
        motor[i]->setCustomConfigDir(config_dir);
        motor[i]->read_custom_config(config_dir + QDir::separator() + "HB-412K");
        connect(motor[i], &DCMotor::soundSetPitch, this, &VL60::soundSetPitch);
        connect(motor[i], &DCMotor::soundSetVolume, this, &VL60::soundSetVolume);

        overload_relay[i] = new OverloadRelay();
        overload_relay[i]->read_custom_config(config_dir + QDir::separator() + "PT-140A");
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::initOtherEquipment()
{
    speed_meter = new SL2M();
    speed_meter->setWheelDiameter(wheel_diameter);
    speed_meter->read_custom_config(config_dir + QDir::separator() + "3SL-2M");
    connect(speed_meter, &SL2M::soundSetVolume, this, &VL60::soundSetVolume);

    horn = new TrainHorn();
    connect(horn, &TrainHorn::soundSetVolume, this, &VL60::soundSetVolume);

    reg = new Registrator("vl60");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::initTriggers()
{
    triggers.push_back(&pants_tumbler);
    triggers.push_back(&pant2_tumbler);
    triggers.push_back(&gv_tumbler);
    triggers.push_back(&gv_return_tumbler);
    triggers.push_back(&fr_tumbler);
    triggers.push_back(&mk_tumbler);

    for (size_t i = 0; i < mv_tumblers.size(); ++i)
        triggers.push_back(&mv_tumblers[i]);

    triggers.push_back(&cu_tumbler);

    autoStartTimer = new Timer(0.5);
    connect(autoStartTimer, &Timer::process, this, &VL60::slotAutoStart);
    start_count = 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::initialization()
{
    FileSystem &fs = FileSystem::getInstance();
    QString modules_dir = QString(fs.getModulesDir().c_str());

    Uks = WIRE_VOLTAGE;
    current_kind = 1;

    initPantographs();

    initHighVoltageScheme();

    initSupplyMachines();

    initBrakeControls(modules_dir);

    initBrakeMechanics();

    initBrakeEquipment(modules_dir);

    initTractionControl();

    initOtherEquipment();

    initTriggers();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::debugPrint(double t)
{
    DebugMsg = QString("t: %1 x: %12 км v: %11 км/ч ЗР: %2 МПа ТЦ1: %3 ТЦ2: %4 Наж. на колодку: %5 кН Uву: %10 В Uтэд: %6 В Поз.: %7 Iя: %8 А Iв: %9 А")

            .arg(t, 10, 'f', 2)
            .arg(supply_reservoir->getPressure(), 4, 'f', 2)
            .arg(trolley_mech[TROLLEY_FWD]->getBrakeCylinderPressure(), 4, 'f', 2)
            .arg(trolley_mech[TROLLEY_BWD]->getBrakeCylinderPressure(), 4, 'f', 2)
            .arg(trolley_mech[TROLLEY_FWD]->getShoeForce() / 1000.0, 5, 'f', 1)
            .arg(motor[TED1]->getUd(), 6, 'f', 1)
            .arg(trac_trans->getPosName(), 2)
            .arg(motor[TED1]->getIa(), 6,'f',1)
            .arg(motor[TED1]->getIf(), 6,'f',1)
            .arg(vu[VU1]->getU_out(), 6, 'f', 1)
            .arg(velocity * Physics::kmh, 6, 'f', 1)
            .arg(railway_coord / 1000.0, 7, 'f', 2);
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::stepOtherEquipment(double t, double dt)
{
    speed_meter->setOmega(wheel_omega[TED1]);
    speed_meter->step(t, dt);

    horn->setControl(keys);
    horn->step(t, dt);

    debugPrint(t);

    registration(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::step(double t, double dt)
{
    stepPantographsControl(t, dt);

    stepMainSwitchControl(t, dt);

    stepTracTransformer(t, dt);

    stepPhaseSplitter(t, dt);

    stepMotorFans(t, dt);

    stepMotorCompressor(t, dt);

    stepBrakeControl(t, dt);

    stepTrolleysBrakeMech(t, dt);

    stepAirDistributors(t, dt);

    stepSignalsOutput();

    stepTractionControl(t, dt);

    stepLineContactors(t, dt);

    stepOtherEquipment(t, dt);

    if (control_signals.analogSignal[0].is_active)
    {
        if (control_signals.analogSignal[0].value > 0.9f)
            gv_tumbler.set();
        else
            gv_tumbler.reset();
    }

    autoStartTimer->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::stepPantographsControl(double t, double dt)
{
    pantographs[0]->setState(pant1_tumbler.getState() && pants_tumbler.getState());
    pantographs[1]->setState(pant2_tumbler.getState() && pants_tumbler.getState());

    for (auto pant : pantographs)
    {
        // Задаем текущее напряжение КС (пока что через константу)
        pant->setUks(Uks);
        // Моделируем работу токоприемников
        pant->step(t, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::stepMainSwitchControl(double t, double dt)
{
    // Подаем на вход напряжение с крышевой шины, на которую включены
    // оба токоприемника
    main_switch->setU_in(max(pantographs[0]->getUout(), pantographs[1]->getUout()));

    // Задаем состояние органов управления ГВ
    main_switch->setState(gv_tumbler.getState());
    main_switch->setReturn(gv_return_tumbler.getState());

    // Подаем питание на удерживающую катушку ГВ
    main_switch->setHoldingCoilState(getHoldingCoilState());

    gauge_KV_ks->setInput(main_switch->getU_out() / 30000.0);

    // Моделируем работу ГВ
    main_switch->step(t, dt);

    gauge_KV_ks->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::stepTracTransformer(double t, double dt)
{
    // Задаем напряжение на первичной обмотке (с выхода ГВ)
    trac_trans->setU1(main_switch->getU_out());
    trac_trans->setPosition(main_controller->getPosition());

    trac_trans->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::stepPhaseSplitter(double t, double dt)
{
    double U_power = trac_trans->getU_sn() * static_cast<double>(fr_tumbler.getState());
    phase_spliter->setU_power(U_power);

    phase_spliter->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::stepMotorFans(double t, double dt)
{
    for (size_t i = 0; i < NUM_MOTOR_FANS; ++i)
    {
        MotorFan *mf = motor_fans[i];
        mf->setU_power(phase_spliter->getU_out() * static_cast<double>(mv_tumblers[i].getState()));
        mf->step(t, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::stepMotorCompressor(double t, double dt)
{
    //double k_flow = 5e-3;
    //main_reservoir->setFlowCoeff(k_flow);
    main_reservoir->setAirFlow(motor_compressor->getAirFlow());
    main_reservoir->step(t, dt);

    motor_compressor->setExternalPressure(main_reservoir->getPressure());
    motor_compressor->setU_power(phase_spliter->getU_out() * static_cast<double>(mk_tumbler.getState()) * press_reg->getState());
    motor_compressor->step(t, dt);

    press_reg->setPressure(main_reservoir->getPressure());
    press_reg->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::stepBrakeControl(double t, double dt)
{
    // Подключаем к УБТ трубопровод от ГР
    ubt->setLocoFLpressure(main_reservoir->getPressure());
    // Подключаем к УБТ трубопровод ТМ от КрМ
    ubt->setCraneTMpressure(brake_crane->getBrakePipeInitPressure());    
    ubt->setControl(keys);
    // Задаем давление в начале ТМ
    p0 = ubt->getLocoTMpressure();
    ubt->step(t, dt);

    brake_crane->setFeedLinePressure(ubt->getCraneFLpressure());
    brake_crane->setChargePressure(charge_press);
    brake_crane->setBrakePipePressure(pTM);
    brake_crane->setControl(keys);
    brake_crane->step(t, dt);

    loco_crane->setFeedlinePressure(ubt->getCraneFLpressure());
    loco_crane->setBrakeCylinderPressure(switch_valve->getPressure2());
    loco_crane->setAirDistributorFlow(0.0);
    loco_crane->setControl(keys);
    loco_crane->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::stepTrolleysBrakeMech(double t, double dt)
{
    switch_valve->setInputFlow1(air_disr->getBrakeCylinderAirFlow());
    switch_valve->setInputFlow2(loco_crane->getBrakeCylinderFlow());
    switch_valve->setOutputPressure(pneumo_splitter->getP_in());
    switch_valve->step(t, dt);

    // Тройник подключен к ЗПК
    pneumo_splitter->setQ_in(switch_valve->getOutputFlow());
    pneumo_splitter->setP_out1(pneumo_relay->getWorkPressure());
    pneumo_splitter->setP_out2(trolley_mech[TROLLEY_BWD]->getBrakeCylinderPressure());
    pneumo_splitter->step(t, dt);

    pneumo_relay->setPipelinePressure(main_reservoir->getPressure());
    pneumo_relay->setWorkAirFlow(pneumo_splitter->getQ_out1());
    pneumo_relay->setBrakeCylPressure(trolley_mech[TROLLEY_FWD]->getBrakeCylinderPressure());
    pneumo_relay->step(t, dt);

    // Передняя тележка наполняется через реле давления 304
    trolley_mech[TROLLEY_FWD]->setAirFlow(pneumo_relay->getBrakeCylAirFlow());
    trolley_mech[TROLLEY_FWD]->setVelocity(velocity);
    trolley_mech[TROLLEY_FWD]->step(t, dt);

    // Задняя тележка подключена через тройник от ЗПК
    trolley_mech[TROLLEY_BWD]->setAirFlow(pneumo_splitter->getQ_out2());
    trolley_mech[TROLLEY_FWD]->setVelocity(velocity);
    trolley_mech[TROLLEY_BWD]->step(t, dt);

    Q_r[1] = trolley_mech[TROLLEY_FWD]->getBrakeTorque();
    Q_r[2] = trolley_mech[TROLLEY_FWD]->getBrakeTorque();
    Q_r[3] = trolley_mech[TROLLEY_FWD]->getBrakeTorque();

    Q_r[4] = trolley_mech[TROLLEY_BWD]->getBrakeTorque();
    Q_r[5] = trolley_mech[TROLLEY_BWD]->getBrakeTorque();
    Q_r[6] = trolley_mech[TROLLEY_BWD]->getBrakeTorque();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::stepAirDistributors(double t, double dt)
{
    supply_reservoir->setAirFlow(air_disr->getAirSupplyFlow());
    supply_reservoir->step(t, dt);

    air_disr->setBrakeCylinderPressure(switch_valve->getPressure1());
    air_disr->setAirSupplyPressure(supply_reservoir->getPressure());
    air_disr->setBrakepipePressure(pTM);
    auxRate = air_disr->getAuxRate();
    air_disr->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::stepTractionControl(double t, double dt)
{
    controller->setControl(keys);
    controller->step(t, dt);

    main_controller->enable(cu_tumbler.getState() && static_cast<bool>(ubt->getState()));
    main_controller->setKMstate(controller->getState());
    main_controller->step(t, dt);

    gauge_KV_motors->setInput(vu[VU1]->getU_out());
    gauge_KV_motors->step(t, dt);

    //double ip = 2.73;

    motor[TED1]->setU(vu[VU1]->getU_out() * static_cast<double>(line_contactor[TED1].getState()));
    motor[TED2]->setU(vu[VU1]->getU_out() * static_cast<double>(line_contactor[TED2].getState()));
    motor[TED3]->setU(vu[VU1]->getU_out() * static_cast<double>(line_contactor[TED3].getState()));

    motor[TED4]->setU(vu[VU2]->getU_out() * static_cast<double>(line_contactor[TED4].getState()));
    motor[TED5]->setU(vu[VU2]->getU_out() * static_cast<double>(line_contactor[TED5].getState()));
    motor[TED6]->setU(vu[VU2]->getU_out() * static_cast<double>(line_contactor[TED6].getState()));

    km_state_t km_state = controller->getState();

    double I_vu = 0;

    for (size_t i = 0; i < motor.size(); ++i)
    {
        motor[i]->setDirection(km_state.revers_ref_state);
        motor[i]->setOmega(ip * wheel_omega[i]);
        motor[i]->setBetaStep(km_state.field_loosen_pos);
        Q_a[i+1] = motor[i]->getTorque() * ip;
        motor[i]->step(t, dt);

        I_vu += motor[i]->getIa();

        overload_relay[i]->setCurrent(motor[i]->getIa());
        overload_relay[i]->step(t, dt);
    }

    for (auto v : vu)
    {
        v->setI_out(I_vu);
        v->setU_in(trac_trans->getTracVoltage());
        v->step(t, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::stepLineContactors(double t, double dt)
{
    km_state_t km_state = controller->getState();

    bool motor_fans_state = true;

    for (auto mv: motor_fans)
    {
        motor_fans_state = motor_fans_state && !static_cast<bool>(mv->isNoReady());
    }

    bool lk_state = !km_state.pos_state[POS_BV] &&
                    !km_state.pos_state[POS_ZERO] &&
                    motor_fans_state && main_controller->isReady();

    lineContactorsControl(lk_state);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::lineContactorsControl(bool state)
{
    for (size_t i = 0; i < line_contactor.size(); ++i)
    {
        if (state)
            line_contactor[i].set();
        else
            line_contactor[i].reset();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float VL60::isLineContactorsOff()
{
    bool state = true;

    for (size_t i = 0; i < line_contactor.size(); ++i)
    {
        state = state && line_contactor[i].getState();
    }

    return 1.0f - static_cast<float>(state);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::stepSignalsOutput()
{
    // Состояние токоприемников
    analogSignal[PANT1_POS] = static_cast<float>(pantographs[0]->getHeight());
    analogSignal[PANT2_POS] = static_cast<float>(pantographs[1]->getHeight());

    // Состояние тумблеров на пульте машиниста
    analogSignal[TUMBLER_PNT] = static_cast<float>(pants_tumbler.getState());
    analogSignal[TUMBLER_PNT1] = static_cast<float>(pant1_tumbler.getState());
    analogSignal[TUMBLER_PNT2] = static_cast<float>(pant2_tumbler.getState());

    analogSignal[TUMBLER_GV_ON] = static_cast<float>(gv_return_tumbler.getState());
    analogSignal[TUMBLER_GV_ON_OFF] = static_cast<float>(gv_tumbler.getState());

    analogSignal[TUMBLER_FR] = static_cast<float>(fr_tumbler.getState());

    analogSignal[TUMBLER_MV1] = static_cast<float>(mv_tumblers[MV1].getState());
    analogSignal[TUMBLER_MV2] = static_cast<float>(mv_tumblers[MV2].getState());
    analogSignal[TUMBLER_MV3] = static_cast<float>(mv_tumblers[MV3].getState());
    analogSignal[TUMBLER_MV4] = static_cast<float>(mv_tumblers[MV4].getState());
    analogSignal[TUMBLER_MV5] = static_cast<float>(mv_tumblers[MV5].getState());
    analogSignal[TUMBLER_MV6] = static_cast<float>(mv_tumblers[MV6].getState());

    analogSignal[TUMBLER_MK] = static_cast<float>(mk_tumbler.getState());

    analogSignal[TUMBLER_CU] = static_cast<float>(cu_tumbler.getState());

    // Вольтметр КС
    analogSignal[STRELKA_KV2] = static_cast<float>(main_switch->getU_out() / 30000.0);

    // Вольтметр ТЭД
    analogSignal[STRELKA_KV1] = static_cast<float>(gauge_KV_motors->getOutput() / 3000.0);

    // Состояние главного выключателя
    analogSignal[GV_POS] = static_cast<float>(main_switch->getKnifePos());

    // Состояние локомотивного светофора
    analogSignal[LS_G] = 1.0f;

    // Состояние контрольных ламп
    analogSignal[SIG_LIGHT_GV] = main_switch->getLampState();
    analogSignal[SIG_LIGHT_GU] = phase_spliter->isNotReady();
    analogSignal[SIG_LIGHT_FR] = phase_spliter->isNotReady();
    analogSignal[SIG_LIGHT_0HP] = static_cast<float>(main_controller->isLongMotionPos());
    analogSignal[SIG_LIGHT_TR] = cut (motor_fans[MV3]->isNoReady() + motor_fans[MV4]->isNoReady(), 0.0f, 1.0f);
    analogSignal[SIG_LIGHT_VU1] = cut (motor_fans[MV1]->isNoReady() + motor_fans[MV2]->isNoReady(), 0.0f, 1.0f);
    analogSignal[SIG_LIGHT_VU2] = cut (motor_fans[MV5]->isNoReady() + motor_fans[MV6]->isNoReady(), 0.0f, 1.0f);
    analogSignal[SIG_LIGHT_TD] = isLineContactorsOff();

    analogSignal[KONTROLLER] = controller->getMainHandlePos();
    analogSignal[REVERS] = controller->getReversHandlePos();
    analogSignal[STRELKA_SELSIN] = main_controller->getSelsinPosition();

    // Положение рукоятки комбинированного крана
    analogSignal[KRAN_KOMBIN] = ubt->getCombCranePos();
    // Положение рукоятки УБТ
    analogSignal[KLUCH_367] = ubt->getMainHandlePos();

    analogSignal[STRELKA_AMP_EPT] = 0;

    // Манометр питательной магистрали
    analogSignal[STRELKA_M_HM] = static_cast<float>(main_reservoir->getPressure() / 1.6);
    // Манометр тормозной магистрали
    analogSignal[STRELKA_M_TM] = static_cast<float>(pTM / 1.0);
    // Манометр уравнительного резервуара
    analogSignal[STRELKA_M_UR] = static_cast<float>(brake_crane->getEqReservoirPressure() / 1.0);
    // Манометр давления в ТЦ
    analogSignal[STRELKA_M_TC] = static_cast<float>(trolley_mech[TROLLEY_FWD]->getBrakeCylinderPressure() / 1.0);

    // Положение рукоятки КрМ
    analogSignal[KRAN395_RUK] = static_cast<float>(brake_crane->getHandlePosition());

    // Положение рукоятки КВТ
    analogSignal[KRAN254_RUK] = static_cast<float>(loco_crane->getHandlePosition());

    analogSignal[STRELKA_AMP1] = static_cast<float>(motor[TED1]->getIa() / 1500.0);
    analogSignal[STRELKA_AMP2] = static_cast<float>(motor[TED6]->getIa() / 1500.0);    

    analogSignal[STRELKA_SPEED] = speed_meter->getArrowPos();
    analogSignal[VAL_PR_SKOR1] = speed_meter->getShaftPos();
    analogSignal[VAL_PR_SKOR2] = speed_meter->getShaftPos();

    analogSignal[KNOPKA_RB_1] = static_cast<float>(rb[RB_1].getState());
    analogSignal[KNOPKA_RBS] = static_cast<float>(rb[RBS].getState());
    analogSignal[KNOPKA_RBP] = static_cast<float>(rb[RBP].getState());

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
void VL60::registration(double t, double dt)
{
    if (next_vehicle == Q_NULLPTR)
        return;

    double dx = railway_coord  - next_vehicle->getRailwayCoord();

    QString line = QString("%1 %2 %3")
            .arg(t)
            .arg(railway_coord)
            .arg(velocity);


    reg->print(line, t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VL60::getTractionForce()
{
    double ip = 2.73;

    double sumTorque = 0;

    for (auto m : motor)
    {
        sumTorque += m->getTorque();
    }

    double force = sumTorque * ip * 2.0 / wheel_diameter;

    return force;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VL60::getHoldingCoilState() const
{
    km_state_t km_state = controller->getState();

    bool no_overload = true;

    for (auto ov_relay : overload_relay)
    {
        no_overload = no_overload && (!static_cast<bool>(ov_relay->getState()));
    }

    bool state = !km_state.pos_state[POS_BV] && no_overload;

    return state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::keyProcess()
{
    if (autoStartTimer->isStarted())
        return;

    // Управление тумблером "Токоприемники"
    if (getKeyState(KEY_U))
    {
        if (isShift())
            pants_tumbler.set();
        else
            pants_tumbler.reset();
    }

    // Подъем/опускание переднего токоприемника
    if (getKeyState(KEY_I))
    {
        // Переводим тумблер в нужное фиксированное положение
        if (isShift())                    
            pant1_tumbler.set();
        else        
            pant1_tumbler.reset();

        // Задаем статус токоприемнику
        //pantographs[0]->setState(pants_tumbler.getState());
    }

    // Подъем/опускание заднего токоприемника
    if (getKeyState(KEY_O))
    {
        // Переводим тумблер в нужное фиксированное положение
        if (isShift())
            pant2_tumbler.set();
        else
            pant2_tumbler.reset();

        // Задаем статус токоприемнику
        //pantographs[1]->setState(pants_tumbler.getState());
    }

    // Включение/выключение ГВ
    if (getKeyState(KEY_P))
    {
        if (isShift())
            gv_tumbler.set();
        else
            gv_tumbler.reset();
    }

    // Возврат защиты
    if (getKeyState(KEY_K))
        gv_return_tumbler.set();
    else
        gv_return_tumbler.reset();

    // Включение/выключение расщепителя фаз
    if (getKeyState(KEY_T))
    {
        if (isShift())
            fr_tumbler.set();
        else
            fr_tumbler.reset();
    }

    // Включение/выключение мотор-верниляторов

    // МВ1
    if (getKeyState(KEY_R))
    {
        if (isShift())
            mv_tumblers[MV1].set();
        else
            mv_tumblers[MV1].reset();
    }

    // МВ2
    if (getKeyState(KEY_F))
    {
        if (isShift())
            mv_tumblers[MV2].set();
        else
            mv_tumblers[MV2].reset();
    }

    // МВ3
    if (getKeyState(KEY_Y))
    {
        if (isShift())
            mv_tumblers[MV3].set();
        else
            mv_tumblers[MV3].reset();
    }

    // МВ4
    if (getKeyState(KEY_5))
    {
        if (isShift())
            mv_tumblers[MV4].set();
        else
            mv_tumblers[MV4].reset();
    }

    // МВ5
    if (getKeyState(KEY_6))
    {
        if (isShift())
            mv_tumblers[MV5].set();
        else
            mv_tumblers[MV5].reset();
    }

    // МВ6
    if (getKeyState(KEY_7))
    {
        if (isShift())
            mv_tumblers[MV6].set();
        else
            mv_tumblers[MV6].reset();
    }

    // Включение/выключение мотор-компрессора
    if (getKeyState(KEY_E))
    {
        if (isShift())
            mk_tumbler.set();
        else
            mk_tumbler.reset();
    }

    // Включение/выключение цепей управления
    if (getKeyState(KEY_J))
    {
        if (isShift())
            cu_tumbler.set();
        else
            cu_tumbler.reset();
    }

    // Нажатие РБ-1
    if (getKeyState(KEY_Z))
        rb[RB_1].set();
    else
        rb[RB_1].reset();

    // Нажатие РБС
    if (getKeyState(KEY_M))
        rb[RBS].set();
    else
        rb[RBS].reset();

    // Нажатие РБП
    if (getKeyState(KEY_Q))
        rb[RBP].set();
    else
        rb[RBP].reset();

    if (getKeyState(KEY_R))
    {
        if (isAlt() && !autoStartTimer->isStarted())
            autoStartTimer->start();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::load_brakes_config(QString path)
{
    CfgReader cfg;

    if (cfg.load(path))
    {
        QString secName = "BrakesState";

        double pFL = 0.0;

        if (cfg.getDouble(secName, "MainReservoirPressure", pFL))
        {
            main_reservoir->setY(0, pFL);
        }

        double k_flow = 0.0;

        if (cfg.getDouble(secName, "MainReservoirFlow", k_flow))
        {
            main_reservoir->setFlowCoeff(k_flow);
        }

        double ch_press = 0.0;

        if (cfg.getDouble(secName, "ChargingPressure", ch_press))
        {
            charge_press = ch_press;
        }

        int train_crane_pos = 6;

        if (cfg.getInt(secName, "TrainCranePos", train_crane_pos))
        {
            brake_crane->setPosition(train_crane_pos);
        }

        int loco_crane_pos = 0;

        if (cfg.getInt(secName, "LocoCranePos", loco_crane_pos))
        {
            loco_crane->setHandlePosition(loco_crane_pos);
        }

        int brake_lock = 0;

        int combine_crane_pos = -1;

        if (cfg.getInt(secName, "CombineCranePos", combine_crane_pos))
        {
            ubt->setCombineCranePos(combine_crane_pos);
        }

        if (cfg.getInt(secName, "BrakeLockDevice", brake_lock))
        {
            ubt->setState(brake_lock);

            if (brake_lock == 1)
            {
                ubt->setY(0, charge_press);
                brake_crane->init(charge_press, pFL);
                supply_reservoir->setY(0, charge_press);
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::loadConfig(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        QString secName = "Vehicle";

        cfg.getDouble(secName, "ReductorCoeff", ip);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::slotAutoStart()
{
    if (start_count < triggers.size())
    {
        triggers[start_count]->set();

        if (!pantographs[0]->isUp() && !pantographs[1]->isUp() &&
                (triggers[start_count] == &gv_tumbler))
            return;

        if (!static_cast<bool>(main_switch->getLampState()))
            gv_return_tumbler.reset();

        start_count++;
    }
    else
    {
        autoStartTimer->stop();
        controller->setReversPos(REVERS_FORWARD);
        start_count = 0;
    }
}

//------------------------------------------------------------------------------
//  Макрос генерации функции loadVehicle() для симулятора
//------------------------------------------------------------------------------
GET_VEHICLE(VL60)
