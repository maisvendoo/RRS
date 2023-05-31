#include    "filesystem.h"

#include    "simple-car.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
SimpleCar::SimpleCar(QObject *parent) : Vehicle (parent)
{

}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
SimpleCar::~SimpleCar()
{

}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleCar::initBrakeDevices(double p0, double pBP, double pFL)
{
    Q_UNUSED(p0);

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
    if (((idx / 10) % 10) == 1)
    {
        QString name = QString("simple-car-%1").arg((idx / 10), 3, 10, QChar('0'));
        reg = new Registrator(name, 1e-3);
        QString line = QString(" t      ; pBP    ; pBC    ; pSR    ; Q_f    ; Q_b    ; Qad    ");
        reg->print(line, 0, 0);
    }
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleCar::initialization()
{
    initPneumatics();
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleCar::initPneumatics()
{
    FileSystem &fs = FileSystem::getInstance();
    QString modules_dir = QString(fs.getModulesDir().c_str());

    // Тормозная магистраль
    double volume_bp = length * 0.0343 * 0.0343 * Physics::PI / 4.0;
    brakepipe = new Reservoir(volume_bp);
    brakepipe->setFlowCoeff(5e-6);

    air_dist = loadAirDistributor(modules_dir + QDir::separator() + "vr242");
    air_dist->read_config("vr242");

    brake_cylinder = new Reservoir(0.016);

    supply_reservoir = new Reservoir(0.078);

    // Концевые краны
    anglecock_bp_fwd = new PneumoAngleCock();
    anglecock_bp_fwd->read_config("pneumo-anglecock");
    anglecock_bp_fwd->setPipeVolume(volume_bp);
    anglecock_bp_bwd = new PneumoAngleCock();
    anglecock_bp_bwd->read_config("pneumo-anglecock");
    anglecock_bp_bwd->setPipeVolume(volume_bp);

    // Рукава
    hose_bp_fwd = new PneumoHose();
    hose_bp_bwd = new PneumoHose();
    forward_connectors.push_back(hose_bp_fwd);
    backward_connectors.push_back(hose_bp_bwd);
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleCar::loadConfig(QString cfg_path)
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
void SimpleCar::step(double t, double dt)
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
void SimpleCar::stepPneumatics(double t, double dt)
{
    double BP_flow = 0.0;
    BP_flow += anglecock_bp_fwd->getFlowToPipe();
    BP_flow += anglecock_bp_bwd->getFlowToPipe();
    BP_flow += air_dist->getBPflow();
    brakepipe->setFlow(BP_flow);
    brakepipe->step(t, dt);

    air_dist->setBPpressure(brakepipe->getPressure());
    air_dist->setBCpressure(brake_cylinder->getPressure());
    air_dist->setSRpressure(supply_reservoir->getPressure());
    air_dist->step(t, dt);

    brake_cylinder->setFlow(air_dist->getBCflow());
    brake_cylinder->step(t, dt);

    supply_reservoir->setFlow(air_dist->getSRflow());
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
void SimpleCar::stepSignalsOutput()
{
    for(size_t i = 0; i < wheel_rotation_angle.size(); ++i)
        analogSignal[WHEEL_1 + i] =
            static_cast<float>(wheel_rotation_angle[i] / 2.0 / Physics::PI);
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleCar::stepDebugMsg(double t, double dt)
{
    Q_UNUSED(t);
    Q_UNUSED(dt);

    DebugMsg = QString("t %1|")
            .arg(t, 6, 'f', 2);

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
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void SimpleCar::stepRegistrator(double t, double dt)
{
    Q_UNUSED(t);
    Q_UNUSED(dt);

    QString line = QString("%1;%2;%3;%4;%5;%6;%7")
            .arg(t, 8, 'f', 3)
            .arg(brakepipe->getPressure(), 8, 'f', 5)
            .arg(brake_cylinder->getPressure(), 8, 'f', 5)
            .arg(supply_reservoir->getPressure(), 8, 'f', 5)
            .arg(1000*anglecock_bp_fwd->getFlowToPipe(), 8, 'f', 5)
            .arg(1000*anglecock_bp_bwd->getFlowToPipe(), 8, 'f', 5)
            .arg(1000*air_dist->getBPflow(), 8, 'f', 5);
    reg->print(line, t, dt);
}

GET_VEHICLE(SimpleCar)
