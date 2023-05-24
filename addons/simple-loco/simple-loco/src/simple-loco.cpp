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
void SimpleLoco::initBrakeDevices(double p0, double pTM, double pFL)
{
    // Старая пневмосхема
    old_brake_crane->init(pTM, pFL);
    old_brake_crane->setChargePressure(p0);

    old_main_reservoir->setY(0, pFL);
    old_supply_reservoir->setY(0, pTM);

    old_air_dist->init(pTM, pFL);

    // Новая пневмосхема
    brake_crane->init(pTM, pFL);
    brake_crane->setChargePressure(p0);

    main_reservoir->setY(0, pFL);
    supply_reservoir->setY(0, pTM);

    air_dist->init(pTM, pFL);

    // Инициализация давления в тормозной магистрали
    brakepipe->setY(0, pTM);
    anglecock_tm_fwd->setPipePressure(pTM);
    anglecock_tm_bwd->setPipePressure(pTM);
    hose_tm_fwd->setPressure(pTM);
    hose_tm_bwd->setPressure(pTM);

    // Состояние рукавов и концевых кранов
    if (hose_tm_fwd->isLinked())
    {
        hose_tm_fwd->connect();
        anglecock_tm_fwd->open();
    }
    else
    {
        anglecock_tm_fwd->close();
    }

    if (hose_tm_bwd->isLinked())
    {
        hose_tm_bwd->connect();
        anglecock_tm_bwd->open();
    }
    else
    {
        anglecock_tm_bwd->close();
    }

    //reg = nullptr;
    QString name = "simple-loco";
    reg = new Registrator(name, 1e-3);
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleLoco::initialization()
{
    initPneumatics_old();

    initPneumatics();
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleLoco::initPneumatics_old()
{
    FileSystem &fs = FileSystem::getInstance();
    QString modules_dir = QString(fs.getModulesDir().c_str());

    old_main_reservoir = new Reservoir(1.2);
    old_supply_reservoir = new Reservoir(0.078);
    old_brake_cylinder = new Reservoir(0.008);

    old_air_dist = loadAirDistributor(modules_dir + QDir::separator() + "vr242");
    old_air_dist->read_config("vr242");

    old_brake_crane = loadBrakeCrane(modules_dir + QDir::separator() + "krm395");
    old_brake_crane->read_config("krm395");
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleLoco::initPneumatics()
{
    FileSystem &fs = FileSystem::getInstance();
    QString modules_dir = QString(fs.getModulesDir().c_str());

    main_reservoir = new Reservoir(1.2);
    supply_reservoir = new Reservoir(0.078);
    brake_cylinder = new Reservoir(0.008);

    air_dist = loadAirDistributor(modules_dir + QDir::separator() + "vr242");
    air_dist->read_config("vr242");

    brake_crane = loadBrakeCrane(modules_dir + QDir::separator() + "krm395");
    brake_crane->read_config("krm395");
    connect(brake_crane, &BrakeCrane::soundPlay, this, &SimpleLoco::soundPlay);
    connect(brake_crane, &BrakeCrane::soundSetVolume, this, &SimpleLoco::soundSetVolume);

    // Тормозная магистраль
    double volume = length * 0.0343 * 0.0343 * Physics::PI / 4.0;
    brakepipe = new Reservoir(volume);

    // Концевые краны
    anglecock_tm_fwd = new PneumoAngleCock();
    anglecock_tm_fwd->read_config("pneumo-anglecock");
    anglecock_tm_bwd = new PneumoAngleCock();
    anglecock_tm_bwd->read_config("pneumo-anglecock");

    // Рукава
    hose_tm_fwd = new PneumoHose();
    hose_tm_bwd = new PneumoHose();
    forward_connectors.push_back(hose_tm_fwd);
    backward_connectors.push_back(hose_tm_bwd);
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

}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleLoco::step(double t, double dt)
{
    stepPneumatics_old(t, dt);

    stepPneumatics(t, dt);

    stepSignalsOutput();

    stepDebugMsg(t, dt);

    if (reg != nullptr)
        stepRegistrator(t, dt);
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleLoco::stepPneumatics_old(double t, double dt)
{
    old_brake_crane->setFeedLinePressure(old_main_reservoir->getPressure());
    old_brake_crane->setBrakePipePressure(pTM);
    old_brake_crane->setControl(keys);
    old_brake_crane->step(t, dt);

    old_air_dist->setBrakeCylinderPressure(old_brake_cylinder->getPressure());
    old_air_dist->setAirSupplyPressure(old_supply_reservoir->getPressure());
    old_air_dist->setBrakepipePressure(pTM);
    old_air_dist->step(t, dt);

    old_brake_cylinder->setAirFlow(old_air_dist->getBrakeCylinderAirFlow());
    old_brake_cylinder->step(t, dt);

    old_supply_reservoir->setAirFlow(old_air_dist->getAirSupplyFlow());
    old_supply_reservoir->step(t, dt);

    p0 = old_brake_crane->getBrakePipeInitPressure();
    auxRate = old_air_dist->getAuxRate();
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleLoco::stepPneumatics(double t, double dt)
{
    brake_crane->setFeedLinePressure(main_reservoir->getPressure());
    brake_crane->setBrakePipePressure(brakepipe->getPressure());
    brake_crane->setControl(keys);
    brake_crane->step(t, dt);

    air_dist->setBrakeCylinderPressure(brake_cylinder->getPressure());
    air_dist->setAirSupplyPressure(supply_reservoir->getPressure());
    air_dist->setBrakepipePressure(brakepipe->getPressure());
    air_dist->step(t, dt);

    brake_cylinder->setAirFlow(air_dist->getBrakeCylinderAirFlow());
    brake_cylinder->step(t, dt);

    supply_reservoir->setAirFlow(air_dist->getAirSupplyFlow());
    supply_reservoir->step(t, dt);

    hose_tm_fwd->setPressure(anglecock_tm_fwd->getPressureToHose());
    hose_tm_fwd->setFlowCoeff(anglecock_tm_fwd->getFlowCoeff());
    hose_tm_fwd->step(t, dt);
    hose_tm_bwd->setPressure(anglecock_tm_bwd->getPressureToHose());
    hose_tm_bwd->setFlowCoeff(anglecock_tm_bwd->getFlowCoeff());
    hose_tm_bwd->step(t, dt);

    anglecock_tm_fwd->setPipePressure(brakepipe->getPressure());
    anglecock_tm_fwd->setHoseFlow(hose_tm_fwd->getFlow());
    anglecock_tm_fwd->step(t, dt);
    anglecock_tm_bwd->setPipePressure(brakepipe->getPressure());
    anglecock_tm_bwd->setHoseFlow(hose_tm_bwd->getFlow());
    anglecock_tm_bwd->step(t, dt);

    double brakepipe_flow = 0.0;
    brakepipe_flow += anglecock_tm_fwd->getFlowToPipe();
    brakepipe_flow += anglecock_tm_bwd->getFlowToPipe();
    brakepipe_flow += -0.02 * air_dist->getAuxRate();
    brakepipe_flow += brake_crane->getBrakePipeInitPressure() - brakepipe->getPressure();
    brakepipe->setAirFlow(brakepipe_flow);
    brakepipe->step(t, dt);
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleLoco::stepSignalsOutput()
{
    for(size_t i = 0; i < wheel_rotation_angle.size(); ++i)
        analogSignal[WHEEL_1 + i] =
            static_cast<float>(wheel_rotation_angle[i] / 2.0 / Physics::PI);

    // Положение рукоятки КрМ
    analogSignal[KRAN395_RUK] = static_cast<float>(brake_crane->getHandlePosition());

    // Манометр питательной магистрали
    analogSignal[STRELKA_M_HM] = static_cast<float>(main_reservoir->getPressure() / 1.6);
    // Манометр тормозной магистрали
    analogSignal[STRELKA_M_TM] = static_cast<float>(pTM / 1.0);
    // Манометр уравнительного резервуара
    analogSignal[STRELKA_M_UR] = static_cast<float>(brake_crane->getEqReservoirPressure() / 1.0);
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


    DebugMsg = QString("t: %1 | pTM %2 | pBC %3 | aux %4 | pTM %5 | pBC %6 | aux %7 | Qf %9 | Qb %10 | ")
            .arg(t, 8, 'f', 3)
            .arg(pTM, 8, 'f', 5)
            .arg(old_brake_cylinder->getPressure(), 8, 'f', 5)
            .arg(auxRate, 8, 'f', 5)
            .arg(brakepipe->getPressure(), 8, 'f', 5)
            .arg(brake_cylinder->getPressure(), 8, 'f', 5)
            .arg(air_dist->getAuxRate(), 8, 'f', 5)
            .arg(anglecock_tm_fwd->getFlowToPipe(), 8, 'f', 5)
            .arg(anglecock_tm_bwd->getFlowToPipe(), 8, 'f', 5);
/*
    DebugMsg = QString("hoseF l%1c%2|acF o%3|pTM %4 MPa|QF %5 aux %6 QB %7 |pBC %8 MPa|acB o%9|hoseB l%10c%11| ")
            .arg(hose_tm_fwd->isLinked())
            .arg(hose_tm_fwd->isConnected())
            .arg(anglecock_tm_fwd->isOpened())
            .arg(brakepipe->getPressure(), 8, 'f', 5)
            .arg(anglecock_tm_fwd->getFlowToPipe(), 8, 'f', 5)
            .arg(air_dist->getAuxRate(), 8, 'f', 5)
            .arg(anglecock_tm_bwd->getFlowToPipe(), 8, 'f', 5)
            .arg(brake_cylinder->getPressure(), 8, 'f', 5)
            .arg(anglecock_tm_bwd->isOpened())
            .arg(hose_tm_bwd->isLinked())
            .arg(hose_tm_bwd->isConnected());
*/
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleLoco::stepRegistrator(double t, double dt)
{
    QString line = QString("t: %1; pTM %2; pBC %3; aux %4; pTM %5; pBC %6; aux %7; ")
            .arg(t, 8, 'f', 3)
            .arg(pTM, 8, 'f', 5)
            .arg(old_brake_cylinder->getPressure(), 8, 'f', 5)
            .arg(auxRate, 8, 'f', 5)
            .arg(brakepipe->getPressure(), 8, 'f', 5)
            .arg(brake_cylinder->getPressure(), 8, 'f', 5)
            .arg(air_dist->getAuxRate(), 8, 'f', 5);
    reg->print(line, t, dt);
}

GET_VEHICLE(SimpleLoco)
