#include    "filesystem.h"

#include    "simple-loco.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
SimpleLoco::SimpleLoco(QObject *parent) : Vehicle (parent)
  , U_bat(55.0)
  , pBP_prev(0.5)
  , pBP_temp(0.0)
{

}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
SimpleLoco::~SimpleLoco()
{

}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleLoco::initBrakeDevices(double p0, double pBP, double pFL)
{
    brake_lock->setState(true);

    brake_crane->init(pBP, pFL);
    brake_crane->setChargePressure(p0);

    main_reservoir->setY(0, pFL);
    supply_reservoir->setY(0, pBP);

    air_dist->init(pBP, pFL);

    // Инициализация давления в тормозной магистрали
    brakepipe->setY(0, pBP);
    anglecock_bp_fwd->setPipePressure(pBP);
    anglecock_bp_bwd->setPipePressure(pBP);
    hose_bp_fwd->setPressure(pBP);
    hose_bp_bwd->setPressure(pBP);

    // Состояние рукавов и концевых кранов
    if (hose_bp_fwd->isLinked())
    {
        hose_bp_fwd->connect();
        anglecock_bp_fwd->open();
    }
    else
    {
        anglecock_bp_fwd->close();
    }

    if (hose_bp_bwd->isLinked())
    {
        hose_bp_bwd->connect();
        anglecock_bp_bwd->open();
    }
    else
    {
        anglecock_bp_bwd->close();
    }

    reg = nullptr;
/*    QString name = "simple-loco";
    reg = new Registrator(name, 1e-3);
//    QString line = QString(" t      ; temp    ;");
//    line += QString(" pUK   ; pBP   ; pBC   ; pSR   ; BPsr   ; BPuk   ; SRbc   ; BCatm  ; BPatm  ; BPemer ; v11   ; v12   ; v1 ; v2 ; vb ; vs ; vw ");
    QString line = QString(" t      ; pBP   ;");
    line += QString(" pWORK ; pBC   ; pSR   ; zpk1  ; zpk2  ; SRwork ; SRbc   ; WORKat ; BCatm  ; U     ; f     ; I     ;R;B; u1b ; u2r ");
    reg->print(line, 0, 0);*/
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleLoco::initialization()
{
    initPneumatics();

    initEPB();
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleLoco::initPneumatics()
{
    FileSystem &fs = FileSystem::getInstance();
    QString modules_dir = QString(fs.getModulesDir().c_str());

    press_reg = new PressureRegulator();
    press_reg->read_config("pressure-regulator");

    motor_compressor = new ACMotorCompressor();
//    motor_compressor = new DCMotorCompressor();
    motor_compressor->read_config("motor-compressor-ac");
//    motor_compressor->read_config("motor-compressor-dc");
    connect(motor_compressor, &ACMotorCompressor::soundPlay, this, &SimpleLoco::soundPlay);
    connect(motor_compressor, &ACMotorCompressor::soundStop, this, &SimpleLoco::soundStop);
    connect(motor_compressor, &ACMotorCompressor::soundSetPitch, this, &SimpleLoco::soundSetPitch);

    main_reservoir = new Reservoir(1.2);
    main_reservoir->setFlowCoeff(2e-5);

    brake_lock = new BrakeLock();
    brake_lock->read_config("ubt367m");
    connect(brake_lock, &BrakeLock::soundPlay, this, &SimpleLoco::soundPlay);

    brake_crane = loadBrakeCrane(modules_dir + QDir::separator() + "krm395");
    brake_crane->read_config("krm395");
    connect(brake_crane, &BrakeCrane::soundPlay, this, &SimpleLoco::soundPlay);
    connect(brake_crane, &BrakeCrane::soundSetVolume, this, &SimpleLoco::soundSetVolume);

    loco_crane = loadLocoCrane(modules_dir + QDir::separator() + "kvt254");
    loco_crane->read_config("kvt254");
    connect(loco_crane, &LocoCrane::soundPlay, this, &SimpleLoco::soundPlay);
    connect(loco_crane, &LocoCrane::soundStop, this, &SimpleLoco::soundStop);
    connect(loco_crane, &LocoCrane::soundSetVolume, this, &SimpleLoco::soundSetVolume);

    // Тормозная магистраль
    double volume_bp = length * 0.0343 * 0.0343 * Physics::PI / 4.0;
    brakepipe = new Reservoir(volume_bp);
    brakepipe->setFlowCoeff(5e-6);

    air_dist = loadAirDistributor(modules_dir + QDir::separator() + "vr242");
    air_dist->read_config("vr242");

    electro_air_dist = loadElectroAirDistributor(modules_dir + QDir::separator() + "evr305");
    electro_air_dist->read_config("evr305");

    bc_switch_valve = new SwitchingValve();
    bc_switch_valve->read_config("zpk");

    brake_cylinder = new Reservoir(0.015);

    supply_reservoir = new Reservoir(0.078);

    // Концевые краны
    anglecock_bp_fwd = new PneumoAngleCock();
    anglecock_bp_fwd->read_config("pneumo-anglecock");
    anglecock_bp_fwd->setPipeVolume(volume_bp);
    anglecock_bp_bwd = new PneumoAngleCock();
    anglecock_bp_bwd->read_config("pneumo-anglecock");
    anglecock_bp_bwd->setPipeVolume(volume_bp);

    // Рукава
    hose_bp_fwd = loadPneumoHoseEPB(modules_dir + QDir::separator() + "hose369a");
    hose_bp_fwd->read_config("pneumo-hose-BP369a-loco");
    hose_bp_bwd = loadPneumoHoseEPB(modules_dir + QDir::separator() + "hose369a");
    hose_bp_bwd->read_config("pneumo-hose-BP369a-loco");
    forward_connectors.push_back(hose_bp_fwd);
    backward_connectors.push_back(hose_bp_bwd);
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleLoco::initEPB()
{
    epb_converter = new EPBConverter();
    epb_converter->read_config("epb-converter");

    epb_controller = new EPBControl();
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleLoco::loadConfig(QString cfg_path)
{
    // Создаем экземпляр "читателя" XML-конфигов
    CfgReader cfg;

    // Открываем конфигурационный файл по переданному движком пути
    if (cfg.load(cfg_path))
    {

    }
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleLoco::keyProcess()
{
    if (getKeyState(KEY_1))
        debug_num = 1;
    if (getKeyState(KEY_2))
        debug_num = 2;
    if (getKeyState(KEY_3))
        debug_num = 3;
    if (getKeyState(KEY_4))
        debug_num = 4;
    if (getKeyState(KEY_5))
        debug_num = 5;
    if (getKeyState(KEY_6))
        debug_num = 6;
    if (getKeyState(KEY_7))
        debug_num = 7;
    if (getKeyState(KEY_8))
        debug_num = 8;
    if (getKeyState(KEY_9))
        debug_num = 9;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleLoco::step(double t, double dt)
{
    pBP_temp = (brakepipe->getPressure() - pBP_prev) / dt;
    pBP_prev = brakepipe->getPressure();

    stepPneumatics(t, dt);

    stepEPB(t, dt);

    stepSignalsOutput();

    stepDebugMsg(t, dt);

    if (reg != nullptr)
        stepRegistrator(t, dt);
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleLoco::stepPneumatics(double t, double dt)
{
    press_reg->setFLpressure(main_reservoir->getPressure());
    press_reg->step(t, dt);

    double U = 380.0 * static_cast<double>(press_reg->getState());
//    double U = 3000.0 * static_cast<double>(press_reg->getState());
    motor_compressor->setFLpressure(main_reservoir->getPressure());
    motor_compressor->setPowerVoltage(U);
    motor_compressor->step(t, dt);

    double FL_flow = 0.0;
    FL_flow += motor_compressor->getFLflow();
    FL_flow += brake_lock->getFLflow();
    main_reservoir->setFlow(FL_flow);
    main_reservoir->step(t, dt);

    brake_lock->setFLpressure(main_reservoir->getPressure());
    brake_lock->setBPpressure(brakepipe->getPressure());
    brake_lock->setBCpressure(bc_switch_valve->getPressure2());
    brake_lock->setCraneFLflow(brake_crane->getFLflow() + loco_crane->getFLflow());
    brake_lock->setCraneBPflow(brake_crane->getBPflow());
    brake_lock->setCraneBCflow(loco_crane->getBCflow());
    brake_lock->setControl(keys);
    brake_lock->step(t, dt);

    brake_crane->setFLpressure(brake_lock->getCraneFLpressure());
    brake_crane->setBPpressure(brake_lock->getCraneBPpressure());
    brake_crane->setControl(keys);
    brake_crane->step(t, dt);

    loco_crane->setFLpressure(brake_lock->getCraneFLpressure());
    loco_crane->setBCpressure(brake_lock->getCraneBCpressure());
    loco_crane->setILpressure(0.0);
    loco_crane->setControl(keys);
    loco_crane->step(t, dt);

    double BP_flow = 0.0;
    BP_flow += anglecock_bp_fwd->getFlowToPipe();
    BP_flow += anglecock_bp_bwd->getFlowToPipe();
    BP_flow += air_dist->getBPflow();
    BP_flow += brake_lock->getBPflow();
    brakepipe->setFlow(BP_flow);
    brakepipe->step(t, dt);

    air_dist->setBPpressure(brakepipe->getPressure());
    air_dist->setBCpressure(electro_air_dist->getAirdistBCpressure());
    air_dist->setSRpressure(electro_air_dist->getAirdistSRpressure());
    air_dist->step(t, dt);

    electro_air_dist->setAirdistBCflow(air_dist->getBCflow());
    electro_air_dist->setAirdistSRflow(air_dist->getSRflow());
    electro_air_dist->setBCpressure(bc_switch_valve->getPressure1());
    electro_air_dist->setSRpressure(supply_reservoir->getPressure());
    electro_air_dist->step(t, dt);

    bc_switch_valve->setInputFlow1(electro_air_dist->getBCflow());
    bc_switch_valve->setInputFlow2(brake_lock->getBCflow());
    bc_switch_valve->setOutputPressure(brake_cylinder->getPressure());
    bc_switch_valve->step(t, dt);

    brake_cylinder->setFlow(bc_switch_valve->getOutputFlow());
    brake_cylinder->step(t, dt);

    supply_reservoir->setFlow(electro_air_dist->getSRflow());
    supply_reservoir->step(t, dt);

    anglecock_bp_fwd->setPipePressure(brakepipe->getPressure());
    anglecock_bp_fwd->setHoseFlow(hose_bp_fwd->getFlow());
    anglecock_bp_fwd->step(t, dt);
    anglecock_bp_bwd->setPipePressure(brakepipe->getPressure());
    anglecock_bp_bwd->setHoseFlow(hose_bp_bwd->getFlow());
    anglecock_bp_bwd->step(t, dt);

    hose_bp_fwd->setPressure(anglecock_bp_fwd->getPressureToHose());
    hose_bp_fwd->setFlowCoeff(anglecock_bp_fwd->getFlowCoeff());
    hose_bp_fwd->step(t, dt);
    hose_bp_bwd->setPressure(anglecock_bp_bwd->getPressureToHose());
    hose_bp_bwd->setFlowCoeff(anglecock_bp_bwd->getFlowCoeff());
    hose_bp_bwd->step(t, dt);
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleLoco::stepEPB(double t, double dt)
{
    double epb_work_curr = 0.0;
    epb_work_curr += electro_air_dist->getCurrent(0);
    epb_work_curr += hose_bp_fwd->getCurrent(0);
    epb_work_curr += hose_bp_bwd->getCurrent(0);

    epb_converter->setInputVoltage(U_bat);
    epb_converter->setOutputCurrent(epb_work_curr);
    epb_converter->step(t, dt);

    epb_controller->setInputVoltage(epb_converter->getOutputVoltage());
    epb_controller->setHoldState(brake_crane->isHold());
    epb_controller->setBrakeState(brake_crane->isBrake());
    epb_controller->setControlVoltage(  hose_bp_fwd->getVoltage(1)
                                      + hose_bp_bwd->getVoltage(1) );
    epb_controller->step(t, dt);

    electro_air_dist->setVoltage  (0,  epb_controller->getWorkVoltage()
        + hose_bp_fwd->getVoltage(0) + hose_bp_bwd->getVoltage(0) );
    electro_air_dist->setFrequency(0,  epb_controller->getWorkFrequency()
        + hose_bp_fwd->getFrequency(0) + hose_bp_bwd->getFrequency(0) );

    hose_bp_fwd->setVoltage  (0,
        hose_bp_bwd->getVoltage(0) + epb_controller->getWorkVoltage() );
    hose_bp_fwd->setFrequency(0,
        hose_bp_bwd->getFrequency(0) + epb_controller->getWorkFrequency() );
    hose_bp_fwd->setCurrent  (0,
        hose_bp_bwd->getCurrent(0) + electro_air_dist->getCurrent(0) );
    hose_bp_fwd->setVoltage  (1, hose_bp_bwd->getVoltage(1));
    hose_bp_fwd->setFrequency(1, hose_bp_bwd->getFrequency(1));
    hose_bp_fwd->setCurrent  (1, hose_bp_bwd->getCurrent(1));

    hose_bp_bwd->setVoltage  (0,
        hose_bp_fwd->getVoltage(0) + epb_controller->getWorkVoltage() );
    hose_bp_bwd->setFrequency(0,
        hose_bp_fwd->getFrequency(0) + epb_controller->getWorkFrequency() );
    hose_bp_bwd->setCurrent  (0,
        hose_bp_fwd->getCurrent(0) + electro_air_dist->getCurrent(0) );
    hose_bp_bwd->setVoltage  (1, hose_bp_fwd->getVoltage(1));
    hose_bp_bwd->setFrequency(1, hose_bp_fwd->getFrequency(1));
    hose_bp_bwd->setCurrent  (1, hose_bp_fwd->getCurrent(1));
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleLoco::stepSignalsOutput()
{
    for(size_t i = 0; i < wheel_rotation_angle.size(); ++i)
        analogSignal[WHEEL_1 + i] =
            static_cast<float>(wheel_rotation_angle[i] / 2.0 / Physics::PI);

    // Положение рукоятки комбинированного крана
    analogSignal[KRAN_KOMBIN] = static_cast<float>(brake_lock->getCombCranePosition());
    // Положение рукоятки УБТ
    analogSignal[KLUCH_367] = static_cast<float>(brake_lock->getMainHandlePosition());

    // Положение рукоятки КрМ
    analogSignal[KRAN395_RUK] = static_cast<float>(brake_crane->getHandlePosition());

    // Положение рукоятки КВТ
    analogSignal[KRAN254_RUK] = static_cast<float>(loco_crane->getHandlePosition());
    analogSignal[KRAN254_SHIFT] = static_cast<float>(loco_crane->getHandleShift());

    // Манометр питательной магистрали
    analogSignal[STRELKA_M_HM] = static_cast<float>(main_reservoir->getPressure() / 1.6);
    // Манометр тормозной магистрали
    analogSignal[STRELKA_M_TM] = static_cast<float>(brakepipe->getPressure());
    // Манометр уравнительного резервуара
    analogSignal[STRELKA_M_UR] = static_cast<float>(brake_crane->getERpressure() / 1.0);
    // Манометр давления в ТЦ
    analogSignal[STRELKA_M_TC] = static_cast<float>(brake_cylinder->getPressure() / 1.0);

    // Лампы контроля ЭПТ
    analogSignal[SIG_LIGHT_O] = epb_controller->stateReleaseLamp();
    analogSignal[SIG_LIGHT_P] = epb_controller->stateHoldLamp();
    analogSignal[SIG_LIGHT_T] = epb_controller->stateBrakeLamp();
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleLoco::stepDebugMsg(double t, double dt)
{
    Q_UNUSED(t);
    Q_UNUSED(dt);

    DebugMsg = QString("t %1|")
            .arg(t, 6, 'f', 2);

    switch (debug_num)
    {
    case 1:
    {
        DebugMsg += QString("pFL %1 crane %2|pBP %3 crane %4|pBC %5 crane %6|QFL %7 crane %8|QBP %9 crane %10|QBC %11 crane %12   ")
                .arg(main_reservoir->getPressure(), 7, 'f', 5)
                .arg(brake_lock->getCraneFLpressure(), 7, 'f', 5)
                .arg(brakepipe->getPressure(), 7, 'f', 5)
                .arg(brake_lock->getCraneBPpressure(), 7, 'f', 5)
                .arg(bc_switch_valve->getPressure2(), 7, 'f', 5)
                .arg(brake_lock->getCraneBCpressure(), 7, 'f', 5)
                .arg(brake_lock->getFLflow(), 7, 'f', 5)
                .arg(1000*(brake_crane->getFLflow() + loco_crane->getFLflow()), 9, 'f', 7)
                .arg(1000*brake_lock->getBPflow(), 9, 'f', 7)
                .arg(1000*brake_crane->getBPflow(), 9, 'f', 7)
                .arg(1000*brake_lock->getBCflow(), 9, 'f', 7)
                .arg(1000*loco_crane->getBCflow(), 9, 'f', 7);
        break;
    }
    case 2:
    {
        DebugMsg += QString("pFL %1 (crane %2) |Reg %3 MC %4 (w %5)|QFL %6 %7 (395 %8 254 %9)                ")
                .arg(main_reservoir->getPressure(), 7, 'f', 5)
                .arg(brake_lock->getCraneFLpressure(), 7, 'f', 5)
                .arg(press_reg->getState())
                .arg(motor_compressor->isPowered())
                .arg(motor_compressor->getY(0), 9, 'f', 7)
                .arg(1000*motor_compressor->getFLflow(), 9, 'f', 7)
                .arg(1000*brake_lock->getFLflow(), 9, 'f', 7)
                .arg(1000*brake_crane->getFLflow(), 9, 'f', 7)
                .arg(1000*loco_crane->getFLflow(), 9, 'f', 7);
        break;
    }
    case 3:
    {
        DebugMsg += QString("pER %1 |pBP %2 (crane %3)|QBP %4 (395 %5) 242 %6 fwd %7 bwd %8                     ")
                .arg(brake_crane->getERpressure(), 7, 'f', 5)
                .arg(brakepipe->getPressure(), 7, 'f', 5)
                .arg(brake_lock->getCraneBPpressure(), 7, 'f', 5)
                .arg(1000*brake_lock->getBPflow(), 9, 'f', 7)
                .arg(1000*brake_crane->getBPflow(), 9, 'f', 7)
                .arg(1000*air_dist->getBPflow(), 9, 'f', 7)
                .arg(1000*anglecock_bp_fwd->getFlowToPipe(), 9, 'f', 7)
                .arg(1000*anglecock_bp_bwd->getFlowToPipe(), 9, 'f', 7);
        break;
    }
    case 4:
    {
        DebugMsg += QString("pBP %1 |pSR %2 (QSR %3)|pBC %4 (242 %5 254 %6)|QBC %7 (242 %8 254 %9)               ")
                .arg(brakepipe->getPressure(), 7, 'f', 5)
                .arg(supply_reservoir->getPressure(), 7, 'f', 5)
                .arg(1000*air_dist->getSRflow(), 9, 'f', 7)
                .arg(brake_cylinder->getPressure(), 7, 'f', 5)
                .arg(bc_switch_valve->getPressure1(), 7, 'f', 5)
                .arg(bc_switch_valve->getPressure2(), 7, 'f', 5)
                .arg(1000*bc_switch_valve->getOutputFlow(), 9, 'f', 7)
                .arg(1000*air_dist->getBCflow(), 9, 'f', 7)
                .arg(1000*brake_lock->getBCflow(), 9, 'f', 7);
        break;
    }
    default:
    case 5:
    {
        DebugMsg += QString("hoseF l%1 c%2|acF o%3|pTM %4|QF%5 aux%6 QB%7 |pBC %8 pSR %9|acB o%10|hoseB l%11 c%12                ")
                .arg(hose_bp_fwd->isLinked())
                .arg(hose_bp_fwd->isConnected())
                .arg(anglecock_bp_fwd->isOpened())
                .arg(brakepipe->getPressure(), 8, 'f', 5)
                .arg(1000*anglecock_bp_fwd->getFlowToPipe(), 10, 'f', 7)
                .arg(1000*air_dist->getBPflow(), 10, 'f', 7)
                .arg(1000*anglecock_bp_bwd->getFlowToPipe(), 10, 'f', 7)
                .arg(brake_cylinder->getPressure(), 8, 'f', 5)
                .arg(supply_reservoir->getPressure(), 8, 'f', 5)
                .arg(anglecock_bp_bwd->isOpened())
                .arg(hose_bp_bwd->isLinked())
                .arg(hose_bp_bwd->isConnected());
        break;
    }
    case 6:
    {
        DebugMsg += QString("F: %1/%2 | U0 %3 f0 %4 I0 %5 | U1 %6 f1 %7 I1 %8 |")
                .arg(hose_bp_fwd->getConnectedLinesNumber())
                .arg(hose_bp_fwd->getOutputSignal(5), 1, 'f', 0)
                .arg(hose_bp_fwd->getOutputSignal(6), 6, 'f', 1)
                .arg(hose_bp_fwd->getOutputSignal(7), 6, 'f', 1)
                .arg(hose_bp_fwd->getOutputSignal(8), 6, 'f', 3)
                .arg(hose_bp_fwd->getOutputSignal(9), 6, 'f', 1)
                .arg(hose_bp_fwd->getOutputSignal(10), 6, 'f', 1)
                .arg(hose_bp_fwd->getOutputSignal(11), 6, 'f', 3);
        DebugMsg += QString("B: %1/%2 | U0 %3 f0 %4 I0 %5 | U1 %6 f1 %7 I1 %8 |")
                .arg(hose_bp_bwd->getConnectedLinesNumber())
                .arg(hose_bp_fwd->getOutputSignal(5), 1, 'f', 0)
                .arg(hose_bp_bwd->getOutputSignal(6), 6, 'f', 1)
                .arg(hose_bp_bwd->getOutputSignal(7), 6, 'f', 1)
                .arg(hose_bp_bwd->getOutputSignal(8), 6, 'f', 3)
                .arg(hose_bp_bwd->getOutputSignal(9), 6, 'f', 1)
                .arg(hose_bp_bwd->getOutputSignal(10), 6, 'f', 1)
                .arg(hose_bp_bwd->getOutputSignal(11), 6, 'f', 3);
        DebugMsg += QString("R%1 H%2 B%3                 ")
                .arg(epb_controller->stateReleaseLamp())
                .arg(epb_controller->stateHoldLamp())
                .arg(epb_controller->stateBrakeLamp());
        break;
    }
    }
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleLoco::stepRegistrator(double t, double dt)
{
    Q_UNUSED(t);
    Q_UNUSED(dt);

    QString line = QString("%1;")
            .arg(t, 8, 'f', 3);
/*
    line += QString("%1;")
            .arg(pBP_temp, 9, 'f', 6);
    line += air_dist->getDebugMsg();

    line += QString("%1;")
            .arg(brakepipe->getPressure(), 7, 'f', 5);
    line += electro_air_dist->getDebugMsg();
*/
    reg->print(line, t, dt);
}

GET_VEHICLE(SimpleLoco)
