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
  , Uks(30000.0f)
  , pant1_pos(0.0)
  , pant2_pos(0.0)
  , gv_pos(0.0)
  , gv_return(false)
  , test_lamp(0.0)
  , sig(1)
  , charge_press(0.0)
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
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::initPantographs()
{
    QString pant_cfg_path = config_dir + QDir::separator() + "pantograph.xml";

    for (size_t i = 0; i < NUM_PANTOGRAPHS; ++i)
    {
        pantographs[i] = new Pantograph(pant_cfg_path);
        connect(pantographs[i], &Pantograph::soundPlay, this, &VL60::soundPlay);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::initHighVoltageScheme()
{
    QString gv_cfg_path = config_dir + QDir::separator() + "main-switch.xml";

    main_switch = new MainSwitch(gv_cfg_path);
    connect(main_switch, &MainSwitch::soundPlay, this, &VL60::soundPlay);

    gauge_KV_ks = new Oscillator();
    gauge_KV_ks->read_config("oscillator");

    trac_trans = new TracTransformer();
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
void VL60::initialization()
{
    FileSystem &fs = FileSystem::getInstance();
    QString modules_dir = QString(fs.getModulesDir().c_str());

    initPantographs();

    initHighVoltageScheme();

    initSupplyMachines();

    initBrakeControls(modules_dir);

    initBrakeMechanics();

    initBrakeEquipment(modules_dir);
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

    DebugMsg = QString("t: %1 ЗР: %2 МПа ТЦ1: %3 ТЦ2: %4 Наж. на колодку: %5 кН")
            .arg(t, 10, 'f', 2)
            .arg(supply_reservoir->getPressure(), 4, 'f', 2)
            .arg(trolley_mech[TROLLEY_FWD]->getBrakeCylinderPressure(), 4, 'f', 2)
            .arg(trolley_mech[TROLLEY_BWD]->getBrakeCylinderPressure(), 4, 'f', 2)
            .arg(trolley_mech[TROLLEY_FWD]->getShoeForce() / 1000.0, 5, 'f', 1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::stepPantographsControl(double t, double dt)
{
    for (auto pant : pantographs)
    {
        // Задаем текущее напряжение КС (пока что через константу)
        pant->setUks(WIRE_VOLTAGE);
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
    double k_flow = 5e-3;
    main_reservoir->setFlowCoeff(k_flow);
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
    trolley_mech[TROLLEY_FWD]->step(t, dt);

    // Задняя тележка подключена через тройник от ЗПК
    trolley_mech[TROLLEY_BWD]->setAirFlow(pneumo_splitter->getQ_out2());
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

    // Вольтметр КС
    analogSignal[STRELKA_KV2] = static_cast<float>(gauge_KV_ks->getOutput());

    // Состояние главного выключателя
    analogSignal[GV_POS] = static_cast<float>(main_switch->getKnifePos());

    // Состояние локомотивного светофора
    analogSignal[LS_G] = 1.0f;

    // Состояние контрольных ламп
    analogSignal[SIG_LIGHT_GV] = main_switch->getLampState();
    analogSignal[SIG_LIGHT_GU] = 1.0f;
    analogSignal[SIG_LIGHT_FR] = phase_spliter->isNotReady();
    analogSignal[SIG_LIGHT_0HP] = 1.0f;
    analogSignal[SIG_LIGHT_TR] = cut (motor_fans[MV3]->isNoReady() + motor_fans[MV4]->isNoReady(), 0.0f, 1.0f);
    analogSignal[SIG_LIGHT_VU1] = cut (motor_fans[MV1]->isNoReady() + motor_fans[MV2]->isNoReady(), 0.0f, 1.0f);
    analogSignal[SIG_LIGHT_VU2] = cut (motor_fans[MV5]->isNoReady() + motor_fans[MV6]->isNoReady(), 0.0f, 1.0f);
    analogSignal[SIG_LIGHT_TD] = 1.0;    

    analogSignal[KONTROLLER] = -0.5;

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
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VL60::getHoldingCoilState() const
{
    bool state = true;

    return state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::keyProcess()
{
    // Управление тумблером "Токоприемники"
    if (getKeyState(KEY_O))
    {
        if (isShift())
            pants_tumbler.set();
        else
            pants_tumbler.reset();
    }

    // Подъем/опускание переднего токоприемника
    if (getKeyState(KEY_U))
    {
        // Переводим тумблер в нужное фиксированное положение
        if (isShift())                    
            pant1_tumbler.set();
        else        
            pant1_tumbler.reset();

        // Задаем статус токоприемнику
        pantographs[0]->setState(isShift() && pants_tumbler.getState());
    }

    // Подъем/опускание заднего токоприемника
    if (getKeyState(KEY_I))
    {
        // Переводим тумблер в нужное фиксированное положение
        if (isShift())
            pant2_tumbler.set();
        else
            pant2_tumbler.reset();

        // Задаем статус токоприемнику
        pantographs[1]->setState(isShift() && pants_tumbler.getState());
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
    if (getKeyState(KEY_G))
    {
        if (isShift())
            mv_tumblers[MV3].set();
        else
            mv_tumblers[MV3].reset();
    }

    // МВ4
    if (getKeyState(KEY_Y))
    {
        if (isShift())
            mv_tumblers[MV4].set();
        else
            mv_tumblers[MV4].reset();
    }

    // МВ5
    if (getKeyState(KEY_J))
    {
        if (isShift())
            mv_tumblers[MV5].set();
        else
            mv_tumblers[MV5].reset();
    }

    // МВ6
    if (getKeyState(KEY_M))
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
}

//------------------------------------------------------------------------------
//  Макрос генерации функции loadVehicle() для симулятора
//------------------------------------------------------------------------------
GET_VEHICLE(VL60)
