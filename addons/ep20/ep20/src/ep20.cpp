 #include    "ep20.h"

#include    <CfgReader.h>
#include    <QDir>

#include    "filesystem.h"

//------------------------------------------------------------------------------
// Конструктор
//------------------------------------------------------------------------------
EP20::EP20()
{
    Uks = 25000.0;
    current_kind = 1;
}

//------------------------------------------------------------------------------
// Деструктор
//------------------------------------------------------------------------------
EP20::~EP20()
{

}

void EP20::initBrakeDevices(double p0, double pTM, double pFL)
{
    main_reservoir->setY(0, pFL);
    charge_press = p0;

    load_brakes_config(config_dir + QDir::separator() + "brakes-init.xml");

    krm->init(pTM, pFL);
    kvt->init(pTM, pFL);
    spareReservoir->setY(0, pTM);
}

//------------------------------------------------------------------------------
// Инициализация
//------------------------------------------------------------------------------
void EP20::initialization()
{
    FileSystem &fs = FileSystem::getInstance();
    QString modules_dir(fs.getModulesDir().c_str());

    // Вызваем метод
    initMPCS();

    // Вызываем метод
    initHighVoltageScheme();

    // Вызываем метод
    initBrakeControls(modules_dir);
}

//------------------------------------------------------------------------------
// Инициализация МПСУ
//------------------------------------------------------------------------------
void EP20::initMPCS()
{
    mpcs = new MPCS();

    mpcs->read_custom_config(config_dir + QDir::separator() + "mpcs");

    mpcs->setStoragePath(config_dir + QDir::separator() + "storage" + QDir::separator());

    mpcs->init();
}

//------------------------------------------------------------------------------
// Инициализация высоковольтной схемы
//------------------------------------------------------------------------------
void EP20::initHighVoltageScheme()
{
    // Вызываем цикл по числу пантографоф
    for (size_t i = 0; i < pantograph.size(); ++i)
    {
        pantograph[i] = new Pantograph();
        // Читаем конфиг
        pantograph[i]->read_custom_config(config_dir + QDir::separator() + "pantograph");
    }

    kindSwitch = new CurrentKindSwitch();

    mainSwitch = new ProtectiveDevice();

    fastSwitch = new ProtectiveDevice();

    tractionTrans = new TractionTransformer();

    for (size_t i = 0; i < trac_conv.size(); ++i)
        trac_conv[i] = new TractionConverter();

    for (size_t i = 0; i <auxConv.size(); ++i)
        auxConv[i] = new AuxiliaryConverter();

    main_reservoir = new Reservoir(1);

    spareReservoir = new Reservoir(0.078);

    for (size_t i = 0; i < motorCompAC.size(); ++i)
    {
        motorCompAC[i] = new ACMotorCompressor();
        motorCompAC[i]->read_custom_config(config_dir + QDir::separator() + "ac-motor-compressor");
    }
}

void EP20::initBrakeControls(QString modules_dir)
{
    krm = loadBrakeCrane(modules_dir + QDir::separator() + "krm130");
    krm->read_config("krm130");

    kvt = loadLocoCrane(modules_dir + QDir::separator() + "kvt224");
    kvt->read_config("kvt224");

    zpk = new SwitchingValve();
    zpk->read_config("zpk");

    airDistr = loadAirDistributor(modules_dir + QDir::separator() + "vr242");
    airDistr->read_config("vr242");

    electroAirDistr = loadElectroAirDistributor(modules_dir + QDir::separator() + "evr305");
    electroAirDistr->read_config("evr305");
}


//------------------------------------------------------------------------------
// Общие шаги моделирования
//------------------------------------------------------------------------------

void EP20::step(double t, double dt)
{
    // Вызываем метод
    stepMPCS(t, dt);

    // Вызываем метод
    stepHighVoltageScheme(t, dt);

    stepBrakeControls(t, dt);

    // Выводим на экран симулятор, высоту подъема/спуска, выходное напряжение, род ток!
    DebugMsg = QString("t: %1 s, U2_4: %2, Q: %3, pUR: %4, pTM: %5, KrM: %6, pTC_2: %7, pTC_1: %8 pZR: %9")
            .arg(t, 10, 'f', 2)
            .arg(auxConv[3]->getU2(), 5, 'f', 1)
            .arg(main_reservoir->getPressure(), 4, 'f', 2)
            .arg(krm->getEqReservoirPressure(), 4, 'f', 2)
            .arg(pTM, 4, 'f', 2)
            .arg(krm->getPositionName(), 4)
            .arg(zpk->getPressure2(), 4, 'f', 2)
            .arg(zpk->getPressure1(), 4, 'f', 2)
            .arg(spareReservoir->getPressure(), 4, 'f', 2);

    stepSignals();
    //________________________________________________

//    .arg(t, 10, 'f', 2)
//    .arg(kindSwitch->getUoutDC(), 4, 'f', 2)
//    .arg(kindSwitch->getUoutAC(), 4, 'f', 2)
//    .arg(mainSwitch->getU_out(), 4, 'f', 2)
//    .arg(fastSwitch->getU_out(), 4, 'f', 2)
//    .arg(tractionTrans->getTractionVoltage(0))
//    .arg(auxConv[1]->getU2());
}

//------------------------------------------------------------------------------
// Шаг моделирования МПСУ
//------------------------------------------------------------------------------
void EP20::stepMPCS(double t, double dt)
{
    // Задаем начальные значения
    mpcs->setSignalInputMPCS(mpcsInput);
    mpcs->setControl(keys);
    mpcsOutput = mpcs->getSignalOutputMPCS();
    mpcs->step(t, dt);
}

//------------------------------------------------------------------------------
// Шаг моделирования высоковольтной схемы
//------------------------------------------------------------------------------
void EP20::stepHighVoltageScheme(double t, double dt)
{
    int current_kind = 0;
    double Ukr_in = 0;

    double Uout_ac = 0;
    double Uout_dc = 0;

    // Пускаем цикл по пантографам и задаем начальные значения
    for (size_t i = 0; i < pantograph.size(); ++i)
    {
        mpcsInput.pant_up[i] = pantograph[i]->isUp();
        mpcsInput.pant_down[i] = pantograph[i]->isDown();

        if (pantograph[i]->getCurrentKindOut() > current_kind)
            current_kind = pantograph[i]->getCurrentKindOut();

        if (pantograph[i]->getUout() > Ukr_in)
            Ukr_in = pantograph[i]->getUout();

        pantograph[i]->setState(mpcsOutput.pant_state[i]);
        pantograph[i]->setUks(Uks);
        pantograph[i]->setCurrentKindIn(this->current_kind);
        pantograph[i]->step(t, dt);
    }

    mpcsInput.current_kind = current_kind;

    // Передаем данные для Переключателя рода тока!
    kindSwitch->setUkrIn(Ukr_in);
    kindSwitch->setState(current_kind - 1);
    kindSwitch->step(t, dt);

    // Передаем данные для ГВ/БВ
    mainSwitch->setU_in(kindSwitch->getUoutAC());
    fastSwitch->setU_in(kindSwitch->getUoutDC());


    // Передаем данные для ГВ
    mpcsInput.Uin_ms = kindSwitch->getUoutAC();
    mpcsInput.current_kind_switch_state = current_kind - 1;
    mpcsInput.isOff_fs = fastSwitch->getState();
    mainSwitch->setReturn(true);
    mainSwitch->setHoldingCoilState(true);
    mainSwitch->setState(mpcsOutput.turn_on_ms);

    // Передаем данные для БВ
    mpcsInput.Uin_fs = kindSwitch->getUoutDC();
    mpcsInput.current_kind_switch_state = current_kind - 1;
    mpcsInput.isOff_ms = mainSwitch->getState();
    fastSwitch->setReturn(true);
    fastSwitch->setHoldingCoilState(true);
    fastSwitch->setState(mpcsOutput.turn_on_fs);

    mainSwitch->step(t, dt);
    fastSwitch->step(t, dt);

    // Передаем данные для тягового трансформатора
    tractionTrans->setU1(mainSwitch->getU_out());
    tractionTrans->step(t, dt);

    // Передаем данные для тягового преобразователя
    for (size_t i = 0; i < trac_conv.size(); ++i)
    {
        trac_conv[i]->setUdcIn(fastSwitch->getU_out());
        trac_conv[i]->setUt(tractionTrans->getTractionVoltage(2 * i), 0);
        trac_conv[i]->setUt(tractionTrans->getTractionVoltage(2 * i + 1), 1);
        trac_conv[i]->step(t, dt);
    }

    // Передаем данные для преобразователя собственных нужд
    auxConv[0]->setU4(trac_conv[0]->getU4(0));
    auxConv[1]->setU4(trac_conv[1]->getU4(0));
    auxConv[2]->setU4(trac_conv[1]->getU4(1));
    auxConv[3]->setU4(trac_conv[2]->getU4(0));
    for (size_t i = 0; i < auxConv.size(); ++i)
    {
        mpcsInput.aux_const_U[i] = auxConv[i]->getU2();
        auxConv[i]->step(t, dt);
    }

    // Передаем данные на Мотор-компрессоры и Главный резервуар!
    main_reservoir->setAirFlow(motorCompAC[0]->getAirFlow() + motorCompAC[1]->getAirFlow());
    main_reservoir->step(t, dt);

    mpcsInput.PressMR = main_reservoir->getPressure();

    for(size_t i = 0; i < motorCompAC.size(); ++i)
    {
        motorCompAC[i]->setExternalPressure(main_reservoir->getPressure());
        motorCompAC[i]->setU_power(auxConv[3]->getU2() * static_cast<double>(mpcsOutput.toggleSwitchMK[i]) * mpcsOutput.MKstate);
        motorCompAC[i]->step(t, dt);
    }
}

void EP20::stepBrakeControls(double t, double dt)
{
    krm->setChargePressure(charge_press);
    krm->setFeedLinePressure(main_reservoir->getPressure());
    krm->setBrakePipePressure(pTM);
    krm->setControl(keys);
    krm->step(t, dt);

    p0 = krm->getBrakePipeInitPressure();

    kvt->setFeedlinePressure(main_reservoir->getPressure());
    kvt->setBrakeCylinderPressure(zpk->getPressure2());
    kvt->setControl(keys);
    kvt->step(t, dt);

    zpk->setInputFlow1(electroAirDistr->getQbc_out());
    zpk->setInputFlow2(kvt->getBrakeCylinderFlow());
    zpk->step(t, dt);

//    electroAirDistr->setPbc_in(airSplit->getP_in());

    electroAirDistr->setPbc_in(zpk->getPressure1());

    // Устанавливаем подачу давления с запасного резервуара
    electroAirDistr->setSupplyReservoirPressure(spareReservoir->getPressure());

    // Устанавливаем входной поток воздуха Qзр
    electroAirDistr->setInputSupplyReservoirFlow(airDistr->getAirSupplyFlow());
    // Устанавливаем воздушный поток Qтц
    electroAirDistr->setQbc_in(airDistr->getBrakeCylinderAirFlow());

//    electroAirDistr->setControlLine(handleEDT->getControlSignal() + ept_control[0]);
    electroAirDistr->step(t, dt);



    spareReservoir->setAirFlow(electroAirDistr->getOutputSupplyReservoirFlow());
    spareReservoir->step(t, dt);



    airDistr->setAirSupplyPressure(electroAirDistr->getSupplyReservoirPressure());
    airDistr->setBrakeCylinderPressure(electroAirDistr->getPbc_out());
    airDistr->setBrakepipePressure(pTM);
    airDistr->step(t, dt);
}

//------------------------------------------------------------------------------
// Загрузка конфига
//------------------------------------------------------------------------------
void EP20::loadConfig(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {

    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP20::keyProcess()
{
    //    pantograph[PANT_AC1]->setState(getKeyState(KEY_P));
}

void EP20::load_brakes_config(QString path)
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
            krm->setPosition(train_crane_pos);
        }

        int loco_crane_pos = 0;

        /*if (cfg.getInt(secName, "LocoCranePos", loco_crane_pos))
        {
            loco_crane->setHandlePosition(loco_crane_pos);
        }*/
    }
}

GET_VEHICLE(EP20)
