//------------------------------------------------------------------------------
//
//      Магистральный пассажирский тепловоз ТЭП70.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Роман Бирюков (РомычРЖДУЗ)
//
//      Дата: 12/05/2019
//
//------------------------------------------------------------------------------
#ifndef     TEP70BS_H
#define     TEP70BS_H

#include    "vehicle-api.h"
#include    "tep70bs-signals.h"

#include    "battery.h"
#include    "relay.h"
#include    "fuel-tank.h"
#include    "electric-fuel-pump.h"
#include    "disel.h"
#include    "time-relay.h"
#include    "electric-oil-pump.h"
#include    "starter-generator.h"
#include    "voltage-regulator.h"
#include    "tep70-motor-compressor.h"
#include    "tep70-switcher.h"
#include    "field-generator.h"
#include    "trac-generator.h"
#include    "field-regulator.h"
#include    "trac-motor.h"
#include    "sl2m.h"
#include    "km-2202.h"
#include    "reversor.h"
#include    "brake-switcher.h"
#include    "hysteresis-relay.h"

#include    "msut.h"

#include    "registrator.h"

/*!
 * \class
 * \brief Основной класс, описывающий весь тепловоз
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TEP70BS : public Vehicle
{
public:

    /// Конструктор
    TEP70BS();

    /// Деструктор
    ~TEP70BS();

    /// Инициализация тормозных приборов
    void initBrakeDevices(double p0, double pBP, double pFL);

private:

    /// Имя модуля поездного крана
    QString brake_crane_module_name;
    /// Имя конфига поездного крана
    QString brake_crane_config_name;
    /// Имя модуля локомотивного крана
    QString loco_crane_module_name;
    /// Имя конфига локомотивного крана
    QString loco_crane_config_name;
    /// Имя модуля воздухораспределителя
    QString airdist_module_name;
    /// Имя конфига воздухорапределителя
    QString airdist_config_name;
    /// Имя модуля электровоздухораспределителя
    QString electro_airdist_module_name;
    /// Имя конфига электровоздухорапределителя
    QString electro_airdist_config_name;

    /// Контроллер машиниста
    ControllerKM2202    *km;

    /// Аккумуляторная батарея
    Battery             *battery;

    /// Контактор топливного насоса (КТН)
    Relay               *kontaktor_fuel_pump;

    /// Топливный бак
    FuelTank            *fuel_tank;

    /// Электрический топливный насос (ЭНТ)
    ElectricFuelPump    *electro_fuel_pump;

    /// Дизель
    Disel               *disel;

    /// Реле РУ8
    Relay               *ru8;

    /// Контактор маслопрокачивающего насоса (КМН)
    Relay               *kontaktor_oil_pump;

    /// Реле времени прокачки масла
    TimeRelay           *oilpump_time_relay;

    /// Реле времени прокрутки стартера
    TimeRelay           *starter_time_relay;

    /// Электрический маслопрокачивающий насос (ЭМН)
    ElectricOilPump     *electro_oil_pump;

    /// Стратер-генератор
    StarterGenerator    *starter_generator;

    /// Контактор стартер-генератора (КД)
    Relay               *kontaktor_starter;

    /// Реле РУ10
    Relay               *ru10;

    /// Реле РУ6
    Relay               *ru6;

    /// Реле РУ42
    Relay               *ru42;

    /// Реле РУ7
    Relay               *ru7;

    /// Реле РУ15
    Relay               *ru15;

    /// Блок-магнит МВ6
    Relay               *mv6;

    /// Вентиль топливных насосов (ВТН)
    Relay               *vtn;

    /// Реле РУ4
    Relay               *ru4;

    /// Реле времени РВ4
    TimeRelay           *rv4;

    /// Реле времени РВ9
    TimeRelay           *rv9;

    /// Контактор регулятора напряжения
    Relay               *krn;

    /// Регулятор напряжения заряда АКБ
    VoltageRegulator    *voltage_regulator;

    /// Мотор-компрессор
    TEP70MotorCompressor *motor_compressor;

    /// Регулятор давления в ГР
    PressureRegulator   *press_reg;

    /// Главный резервуар
    Reservoir           *main_reservoir;

    /// Концевой кран питательной магистрали спереди
    PneumoAngleCock     *anglecock_fl_fwd;

    /// Концевой кран питательной магистрали сзади
    PneumoAngleCock     *anglecock_fl_bwd;

    /// Рукав питательной  магистрали спереди
    PneumoHose          *hose_fl_fwd;

    /// Рукав питательной  магистрали сзади
    PneumoHose          *hose_fl_bwd;

    /// Реле РУ18
    Relay               *ru18;

    /// Контактор мотор-компрессора КТК1
    Relay               *ktk1;

    /// Контактор мотор-компрессора КТК2
    Relay               *ktk2;

    /// Реле времени РВ6
    TimeRelay           *rv6;

    /// Блокировочное устройство УБТ усл.№367м
    BrakeLock           *brake_lock;

    /// Поездной кран машиниста усл.№395
    BrakeCrane          *brake_crane;

    /// Кран впомогательного тормоза усл.№254
    LocoCrane           *loco_crane;

    /// ЭПК автостопа
    AutoTrainStop       *epk;

    /// Тормозная магистраль
    Reservoir           *brakepipe;

    /// Воздухораспределитель
    AirDistributor      *air_dist;

    /// Электровоздухораспределитель
    ElectroAirDistributor  *electro_air_dist;

    /// Запасный резервуар
    Reservoir           *supply_reservoir;

    /// Концевой кран тормозной магистрали спереди
    PneumoAngleCock     *anglecock_bp_fwd;

    /// Концевой кран тормозной магистрали сзади
    PneumoAngleCock     *anglecock_bp_bwd;

    /// Рукав тормозной магистрали спереди
    PneumoHoseEPB       *hose_bp_fwd;

    /// Рукав тормозной магистрали сзади
    PneumoHoseEPB       *hose_bp_bwd;

    /// Переключательный клапан магистрали тормозных цилиндров ЗПК
    SwitchingValve      *bc_switch_valve;

    /// Тройник для распределения воздуха от переключательного клапана
    /// к тележкам
    PneumoSplitter      *bc_splitter;

    enum
    {
        NUM_TROLLEYS = 2,
        NUM_AXIS_PER_TROLLEY = 3,
        TROLLEY_FWD = 0,
        TROLLEY_BWD = 1
    };

    /// Повторительное реле давления усл.№304
    std::array<PneumoRelay *, NUM_TROLLEYS> bc_pressure_relay;

    /// Тормозные механизмы тележек
    std::array<BrakeMech *, NUM_TROLLEYS> brake_mech;

    /// Концевой кран магистрали тормозных цилиндров спереди
    PneumoAngleCock     *anglecock_bc_fwd;

    /// Концевой кран магистрали тормозных цилиндров сзади
    PneumoAngleCock     *anglecock_bc_bwd;

    /// Рукав магистрали тормозных цилиндров спереди
    PneumoHose          *hose_bc_fwd;

    /// Рукав магистрали тормозных цилиндров сзади
    PneumoHose          *hose_bc_bwd;

    /// Источник питания ЭПТ
    EPBConverter        *epb_converter;

    /// Блок управления двухпроводного ЭПТ
    EPBControl          *epb_control;

    /// Возбудитель главного генератора
    FieldGenerator          *field_gen;

    /// Контактор возбуждения возбудителя (КВВ)
    Relay                   *kvv;

    /// Контактор возбуждения генератора (КВГ)
    Relay                   *kvg;

    /// Главный (тяговый) генератор
    TracGenerator           *trac_gen;

    /// Регулятор возбуждения тягового генератора
    FieldRegulator          *field_reg;

    /// Ток, потребляемый от главного генератора
    double                  I_gen;

    /// Регистратор, для постоения графиков
    Registrator             *reg;

    /// Кнопка "Отпуск тормозов"
    bool    button_brake_release;

    /// Кнопка "Свисток"
    bool    button_svistok;

    /// Кнопка "Тифон"
    bool    button_tifon;

    /// Рукоятка бдительности (РБ1)
    bool    button_RB1;

    /// Напряжение цепей управления
    double  Ucc;

    /// Ток цепей управления
    double  Icc;

    /// Pарядное давление ТМ
    double  charge_press;

    /// Передаточное число тягового редуктора
    double  ip;

    /// Контактор шунта 1 (КШ1)
    Relay   *ksh1;

    /// Контактор шунта 2 (КШ2)
    Relay   *ksh2;

    /// Реле управления РУ1
    Relay   *ru1;

    /// Свисток и тифон
    TrainHorn   *horn;

    double tracForce;

    bool    is_svistok;

    bool    is_tifon;

    /// Реверсор
    Reversor    *reversor;

    /// Тормозной переключатель
    BrakeSwitcher *brake_switcher;

    /// Реле перехода РП1
    HysteresisRelay     *rp1;

    /// Реле перехода PП2
    HysteresisRelay     *rp2;

    /// Реле времени для выдержки включения КШ2 (нет в схеме!)
    TimeRelay           *ksh2_delay;

    /// Реле выдержки времени для предотвращение отключения КШ1
    /// при выключении КШ2 (нет в схеме!)
    TimeRelay           *ksh1_delay;

    /// Микропроцессорная система управления тепловозом
    MSUT                *msut;

    enum
    {
        NUM_MOTORS = 6
    };

    /// Тяговые двигатели
    std::array<TractionMotor *, NUM_MOTORS> motor;

    /// Поездные контакторы
    std::array<Relay *, NUM_MOTORS + 1> kp;

    // Состояние последовательной цепи размыкающих контактов КП1 - КП7
    bool is_KP1_KP7_off;

    // Состояние последовательной цепи замыкающих контактор КП1 - КП6
    bool is_KP1_KP6_on;

    /// АЗВ "Управление общее"
    Trigger azv_common_control;

    /// АЗВ "Управление тепловозом"
    Trigger azv_upr_tepl;

    /// АЗВ "Топливный насос"
    Trigger azv_fuel_pump;

    /// АЗВ "ЭДТ"
    Trigger azv_edt_on;

    /// АЗВ "Тормоз питание"
    Trigger azv_edt_power;

    /// АЗВ "ЭПТ"
    Trigger azv_ept_on;

    /// АЗВ "Компрессор"
    Trigger azv_motor_compressor;

    /// Тумблер "Напряжение ЦУ. Напряжение ЭПТ"
    Trigger tumbler_voltage;

    /// Тумблер "Аварийная остановка дизеля"
    Trigger tumbler_disel_stop;

    /// Кнопка пуска дизеля
    Trigger button_start_disel;

    /// Тумблер "Ослабление поля I ступени руч./авт."
    TEP70Switcher tumbler_field_weak1;

    /// Тумблер "Ослабление поля II ступени руч./авт."
    TEP70Switcher tumbler_field_weak2;

    /// Тумблер "Управление жалюзи воды руч./авт."
    TEP70Switcher tumbler_water_zaluzi;

    /// Тумблер "Управление жалюзи масла руч./авт."
    TEP70Switcher tumbler_oil_zaluzi;

    TEP70Switcher tumbler_revers;

    msut_input_t msut_input;

    msut_output_t msut_output;

    std::vector<Trigger *> triggers;

    size_t start_count;

    Timer autoStartTimer;

    /// Инициализация всех систем тепловоза
    void initialization();

    /// Инициализация органов управления в кабине
    void initCabineControls();

    /// Инициализация цепей управления
    void initControlCircuit();

    /// Инициализация топливной системы
    void initFuelSystem();

    /// Инициализация дизеля
    void initDisel();

    /// Инициализация маслянной системы
    void initOilSystem();

    /// Инициализация питательной магистрали
    void initPneumoSupply(QString modules_dir);

    /// Инициализация приборов управления тормозами
    void initBrakesControl(QString modules_dir);

    /// Инициализация тормозного оборудования
    void initBrakesEquipment(QString modules_dir);

    /// Инициализация ЭПТ
    void initEPB(QString modules_dir);

    /// Инициализация электрической передачи
    void initElectroTransmission();

    /// Инициализация прочего оборудования
    void initOther();

    /// Инициализация МСУТ
    void initMSUT();

    /// Инициализация звуков
    void initSounds();

    /// Инициализация процедуры автозапуска
    void initAutostart();

    /// Шаг моделирования всех систем локомотива в целом
    void step(double t, double dt);

    /// Шаг моделирования органов управления в кабине
    void stepCabineControls(double t, double dt);

    /// Шаг моделирования цепей управления
    void stepControlCircuit(double t, double dt);

    /// Шаг моделирования топливной системы
    void stepFuelSystem(double t, double dt);

    /// Шаг моделирования дизеля
    void stepDisel(double t, double dt);

    /// Шаг моделирования масляной системы
    void stepOilSystem(double t, double dt);

    /// Шаг моделирования питательной магистрали
    void stepPneumoSupply(double t, double dt);

    /// Шаг моделирования приборов управления тормозами
    void stepBrakesControl(double t, double dt);

    /// Шаг моделирования тормозного оборудования
    void stepBrakesEquipment(double t, double dt);

    /// Шаг моделирования ЭПТ
    void stepEPB(double t, double dt);

    /// Шаг моделирования электрической передачи
    void stepElectroTransmission(double t, double dt);

    /// Шаг моделирования прочего оборудования
    void stepOther(double t, double dt);

    /// Шаг работы МСУТ
    void stepMSUT(double t, double dt);

    /// Вывод сигналов для анимаций
    void stepSignalsOutput(double t, double dt);

    /// Вывод сигналов на дисплей МСУ-ТЭ
    void stepMSUTsignals(double t, double dt);

    void stepAutostart(double t, double dt);

    /// Вывод отладочной строки
    void debugOutput(double t, double dt);

    /// Формирование состояния сигнальных ламп
    float getLampState(double signal);

    /// Загрузка данных из конфигурационных файлов
    void loadConfig(QString cfg_path);

    /// Обработка клавиш
    void keyProcess();

private slots:

    void slotAutostart();
};

#endif // TEP70_H
