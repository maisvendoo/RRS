#include    "ep20.h"

#include    <CfgReader.h>
#include    <QDir>

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

//------------------------------------------------------------------------------
// Инициализация
//------------------------------------------------------------------------------
void EP20::initialization()
{
    // Вызваем метод
    initMPCS();

    // Вызываем метод
    initHighVoltageScheme();
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

    // Выводим на экран симулятор, высоту подъема/спуска, выходное напряжение, род ток!
    DebugMsg = QString("t: %1 s, UoutDC: %2, UoutAC: %3, MainSwitch: %4, FastSwitch: %5, TractionTrans: %6, TR: %7")
            .arg(t, 10, 'f', 2)
            .arg(kindSwitch->getUoutDC(), 4, 'f', 2)
            .arg(kindSwitch->getUoutAC(), 4, 'f', 2)
            .arg(mainSwitch->getU_out(), 4, 'f', 2)
            .arg(fastSwitch->getU_out(), 4, 'f', 2)
            .arg(tractionTrans->getTractionVoltage(0))
            .arg(tractionTrans->getVoltageHeatingCoil());
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

    // Передаем данные для ГВ
    mainSwitch->setU_in(kindSwitch->getUoutAC());
    fastSwitch->setU_in(kindSwitch->getUoutDC());

    mpcsInput.Uin_ms = kindSwitch->getUoutAC();
    mpcsInput.current_kind_switch_state = current_kind - 1;
    mpcsInput.isOff_fs = fastSwitch->getState();
    mainSwitch->setReturn(true);
    mainSwitch->setHoldingCoilState(true);
    mainSwitch->setState(mpcsOutput.turn_on_ms);

    mpcsInput.Uin_fs = kindSwitch->getUoutDC();
    mpcsInput.current_kind_switch_state = current_kind - 1;
    mpcsInput.isOff_ms = mainSwitch->getState();
    fastSwitch->setReturn(true);
    fastSwitch->setHoldingCoilState(true);
    fastSwitch->setState(mpcsOutput.turn_on_fs);

    mainSwitch->step(t, dt);
    fastSwitch->step(t, dt);

    tractionTrans->setU1(mainSwitch->getU_out());
    tractionTrans->step(t, dt);
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

void EP20::keyProcess()
{
//    pantograph[PANT_AC1]->setState(getKeyState(KEY_P));
}

GET_VEHICLE(EP20)
