#ifndef     SIMPLE_LOCO_H
#define     SIMPLE_LOCO_H

#include    "vehicle-api.h"
#include    "simple-loco-signals.h"

#include    "registrator.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SimpleLoco : public Vehicle
{
public:

    /// Конструктор класса
    SimpleLoco(QObject *parent = Q_NULLPTR);

    /// Деструктор класса
    ~SimpleLoco();

    /// Инициализация тормозных приборов
    void initBrakeDevices(double p0, double pTM, double pFL);

private:

    /// Номер Debug-строки
    size_t debug_num;

    /// Подкачка главного резервуара (вместо полноценного компресора)
    double is_compressor;

    /// Главный резервуар
    Reservoir   *main_reservoir;

    /// Блокировочное устройство
    BrakeLock  *brake_lock;

    /// Поездной кран машиниста
    BrakeCrane  *brake_crane;

    /// Кран впомогательного тормоза
    LocoCrane  *loco_crane;

    /// Тормозная магистраль
    Reservoir   *brakepipe;

    /// Воздухораспределитель
    AirDistributor  *air_dist;

    /// Переключательный клапан магистрали тормозных цилиндров
    SwitchingValve  *bc_switch_valve;

    /// Тормозной цилиндр
    Reservoir   *brake_cylinder;

    /// Запасный резервуар
    Reservoir   *supply_reservoir;

    /// Концевой кран тормозной магистрали спереди
    PneumoAngleCock  *anglecock_bp_fwd;

    /// Концевой кран тормозной магистрали сзади
    PneumoAngleCock  *anglecock_bp_bwd;

    /// Рукав тормозной магистрали спереди
    PneumoHose  *hose_bp_fwd;

    /// Рукав тормозной магистрали сзади
    PneumoHose  *hose_bp_bwd;

    /// Registrator
    Registrator *reg;

    /// Общая инициализация локомотива
    void initialization();

    /// Инициализация новой пневмосхемы
    void initPneumatics();

    /// Загрузка пользовательских параметров из конфига
    void loadConfig(QString cfg_path);

    /// Обработка нажатия клавиш
    void keyProcess();

    /// Шаг моделирования систем единицы ПС
    void step(double t, double dt);

    /// Моделирование новой пневмосхемы
    void stepPneumatics(double t, double dt);

    /// Сигналы для анимации
    void stepSignalsOutput();

    /// Отладочная строка
    void stepDebugMsg(double t, double dt);

    /// Запись в лог-файл
    void stepRegistrator(double t, double dt);
};

#endif // SIMPLE_LOCO_H
