#include    "ep20.h"

#include    <CfgReader.h>
#include    <QDir>

//------------------------------------------------------------------------------
// Конструктор
//------------------------------------------------------------------------------
EP20::EP20()
{

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
    //
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

//    mtpu = new TaskPantUp();

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
    DebugMsg = QString("t: %1 s, PANT_AC1: %2 m, PANT_AC2: %3 m, PANT_DC1: %4 m, PANT_DC2: %5 m")
            .arg(t, 10, 'f', 2)
            .arg(pantograph[PANT_AC1]->getHeight(), 4, 'f', 2)
            .arg(pantograph[PANT_AC2]->getHeight(), 4, 'f', 2)
            .arg(pantograph[PANT_DC1]->getHeight(), 4, 'f', 2)
            .arg(pantograph[PANT_DC2]->getHeight(), 4, 'f', 2);
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

    // Пускаем цикл по пантографам и задаем начальные значения
    for (size_t i = 0; i < pantograph.size(); ++i)
    {
        mpcsInput.pant_up[i] = pantograph[i]->isUp();
        mpcsInput.pant_down[i] = pantograph[i]->isDown();

        if (pantograph[i]->getCurrentKindOut() > current_kind)
            current_kind = pantograph[i]->getCurrentKindOut();

        pantograph[i]->setState(mpcsOutput.pant_state[i]);

        pantograph[i]->setUks(25000.0);
        pantograph[i]->setCurrentKindIn(2);
        pantograph[i]->step(t, dt);
    }

    mpcsInput.current_kind = current_kind;
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
