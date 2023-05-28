#include    "filesystem.h"

#include    "simple-loco.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
SimpleLoco::SimpleLoco(QObject *parent) : Vehicle (parent)
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
    //supply_reservoir->setY(0, 0.0);

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
    QString name = "simple-loco";
    reg = new Registrator(name, 1e-3);
    QString line = QString(" t      ; pBP    ; pBC    ; pSR    ; Q_f    ; Q_b    ; Qad    ; Qcrane ");
    reg->print(line, 0, 0);
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleLoco::initialization()
{
    initPneumatics();
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

    double volume = length * 0.0343 * 0.0343 * Physics::PI / 4.0;
    brakepipe = new Reservoir(volume);
    brakepipe->setFlowCoeff(5e-6);

    air_dist = loadAirDistributor(modules_dir + QDir::separator() + "vr242");
    air_dist->read_config("vr242");

    bc_switch_valve = new SwitchingValve();
    bc_switch_valve->read_config("zpk");

    brake_cylinder = new Reservoir(0.008);

    supply_reservoir = new Reservoir(0.078);

    // Концевые краны
    anglecock_bp_fwd = new PneumoAngleCock();
    anglecock_bp_fwd->read_config("pneumo-anglecock");
    anglecock_bp_bwd = new PneumoAngleCock();
    anglecock_bp_bwd->read_config("pneumo-anglecock");

    // Рукава
    hose_bp_fwd = new PneumoHose();
    hose_bp_bwd = new PneumoHose();
    forward_connectors.push_back(hose_bp_fwd);
    backward_connectors.push_back(hose_bp_bwd);
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
    stepPneumatics(t, dt);

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
    air_dist->setBCpressure(bc_switch_valve->getPressure1());
    air_dist->setSRpressure(supply_reservoir->getPressure());
    air_dist->step(t, dt);

    bc_switch_valve->setInputFlow1(air_dist->getBCflow());
    bc_switch_valve->setInputFlow2(brake_lock->getBCflow());
    bc_switch_valve->setOutputPressure(brake_cylinder->getPressure());
    bc_switch_valve->step(t, dt);

    brake_cylinder->setFlow(bc_switch_valve->getOutputFlow());
    brake_cylinder->step(t, dt);

    supply_reservoir->setFlow(air_dist->getSRflow());
    supply_reservoir->step(t, dt);

    hose_bp_fwd->setPressure(anglecock_bp_fwd->getPressureToHose());
    hose_bp_fwd->setFlowCoeff(anglecock_bp_fwd->getFlowCoeff());
    hose_bp_fwd->step(t, dt);
    hose_bp_bwd->setPressure(anglecock_bp_bwd->getPressureToHose());
    hose_bp_bwd->setFlowCoeff(anglecock_bp_bwd->getFlowCoeff());
    hose_bp_bwd->step(t, dt);

    anglecock_bp_fwd->setPipePressure(brakepipe->getPressure());
    anglecock_bp_fwd->setHoseFlow(hose_bp_fwd->getFlow());
    anglecock_bp_fwd->step(t, dt);
    anglecock_bp_bwd->setPipePressure(brakepipe->getPressure());
    anglecock_bp_bwd->setHoseFlow(hose_bp_bwd->getFlow());
    anglecock_bp_bwd->step(t, dt);
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
        DebugMsg += QString("hoseF l%1 c%2|acF o%3|pTM %4|QF %5 aux %6 QB %7 |pBC %8 pSR %9|acB o%10|hoseB l%11 c%12                ")
                .arg(hose_bp_fwd->isLinked())
                .arg(hose_bp_fwd->isConnected())
                .arg(anglecock_bp_fwd->isOpened())
                .arg(brakepipe->getPressure(), 8, 'f', 5)
                .arg(1000*anglecock_bp_fwd->getFlowToPipe(), 9, 'f', 7)
                .arg(1000*air_dist->getBPflow(), 9, 'f', 7)
                .arg(1000*anglecock_bp_bwd->getFlowToPipe(), 9, 'f', 7)
                .arg(brake_cylinder->getPressure(), 8, 'f', 5)
                .arg(supply_reservoir->getPressure(), 8, 'f', 5)
                .arg(anglecock_bp_bwd->isOpened())
                .arg(hose_bp_bwd->isLinked())
                .arg(hose_bp_bwd->isConnected());
        break;
    }
/*    case 8:
    {
        DebugMsg += air_dist->getDebugMsg();
        break;
    }
    case 9:
    {
        DebugMsg += brake_crane->getDebugMsg();
        break;
    }*/
    }
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleLoco::stepRegistrator(double t, double dt)
{
    Q_UNUSED(t);
    Q_UNUSED(dt);

    QString line = QString("%1;%2;%3;%4;%5;%6;%7;%8")
            .arg(t, 8, 'f', 3)
            .arg(brakepipe->getPressure(), 8, 'f', 5)
            .arg(brake_cylinder->getPressure(), 8, 'f', 5)
            .arg(supply_reservoir->getPressure(), 8, 'f', 5)
            .arg(1000*anglecock_bp_fwd->getFlowToPipe(), 8, 'f', 5)
            .arg(1000*anglecock_bp_bwd->getFlowToPipe(), 8, 'f', 5)
            .arg(1000*air_dist->getBPflow(), 8, 'f', 5)
            .arg(1000*brake_lock->getBPflow(), 8, 'f', 5);
    reg->print(line, t, dt);
}

GET_VEHICLE(SimpleLoco)
