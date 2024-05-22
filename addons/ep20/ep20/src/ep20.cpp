   #include    "ep20.h"

#include    <CfgReader.h>
#include    <QDir>

#include    "filesystem.h"

//------------------------------------------------------------------------------
// Конструктор
//------------------------------------------------------------------------------
EP20::EP20() : Vehicle()
  , coupling_module_name("sa3")
  , coupling_config_name("sa3")
  , brake_crane_module_name("krm130")
  , brake_crane_config_name("krm130")
  , loco_crane_module_name("kvt224")
  , loco_crane_config_name("kvt224")
  , airdist_module_name("vr292")
  , airdist_config_name("vr292")
  , electro_airdist_module_name("evr305")
  , electro_airdist_config_name("evr305")
  , coupling_fwd(nullptr)
  , coupling_bwd(nullptr)
  , oper_rod_fwd(nullptr)
  , oper_rod_bwd(nullptr)
{
    U_bat = 55.0;
    Uks = 25000.0;
    current_kind = 1;
}

//------------------------------------------------------------------------------
// Деструктор
//------------------------------------------------------------------------------
EP20::~EP20()
{

}

//------------------------------------------------------------------------------
// Инициализация
//------------------------------------------------------------------------------
void EP20::initialization()
{
    FileSystem &fs = FileSystem::getInstance();
    QString modules_dir(fs.getModulesDir().c_str());

    initCouplings(modules_dir);

    initMPCS();

    initHighVoltageScheme();

    initPneumoSupply(modules_dir);

    initBrakesControl(modules_dir);

    initBrakesEquipment(modules_dir);

    initEPB(modules_dir);

    initKMB2();
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
// Инициализация КМБ2
//------------------------------------------------------------------------------
void EP20::initKMB2()
{
    kmb2 = new KMB2();

    kmb2->read_custom_config(config_dir + QDir::separator() + "kmb2");
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
}

//------------------------------------------------------------------------------
// Общие шаги моделирования
//------------------------------------------------------------------------------

void EP20::step(double t, double dt)
{
    stepCouplings(t, dt);

    stepMPCS(t, dt);

    stepHighVoltageScheme(t, dt);

    stepPneumoSupply(t, dt);

    stepBrakesControl(t, dt);

    stepBrakesEquipment(t, dt);

    stepEPB(t, dt);

    stepKMB2(t, dt);

    stepSignals();

    debugOutput(t, dt);
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
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP20::stepKMB2(double t, double dt)
{
    kmb2->setControl(keys);
    kmb2->step(t, dt);
}

//------------------------------------------------------------------------------
// Загрузка конфига
//------------------------------------------------------------------------------
void EP20::loadConfig(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        QString secName = "Vehicle";

        cfg.getString(secName, "CouplingModule", coupling_module_name);
        cfg.getString(secName, "CouplingConfig", coupling_config_name);
        cfg.getString(secName, "BrakeCraneModule", brake_crane_module_name);
        cfg.getString(secName, "BrakeCraneConfig", brake_crane_config_name);
        cfg.getString(secName, "LocoCraneModule", loco_crane_module_name);
        cfg.getString(secName, "LocoCraneConfig", loco_crane_config_name);
        cfg.getString(secName, "AirDistModule", airdist_module_name);
        cfg.getString(secName, "AirDistConfig", airdist_config_name);
        cfg.getString(secName, "ElectroAirDistModule", electro_airdist_module_name);
        cfg.getString(secName, "ElectroAirDistConfig", electro_airdist_config_name);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP20::keyProcess()
{
    //    pantograph[PANT_AC1]->setState(getKeyState(KEY_P));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
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
            main_reservoir->setLeakCoeff(k_flow);
        }

        double ch_press = 0.0;

        if (cfg.getDouble(secName, "ChargingPressure", ch_press))
        {
            charge_press = ch_press;
        }

        int train_crane_pos = 6;

        if (cfg.getInt(secName, "TrainCranePos", train_crane_pos))
        {
            brake_crane->setHandlePosition(train_crane_pos);
        }

        int loco_crane_pos = 0;

        if (cfg.getInt(secName, "LocoCranePos", loco_crane_pos))
        {
            loco_crane->setHandlePosition(loco_crane_pos);
        }
    }
}

GET_VEHICLE(EP20)
