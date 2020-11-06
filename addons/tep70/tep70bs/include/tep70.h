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
#ifndef     TEP70_H
#define     TEP70_H

#include    "vehicle-api.h"
#include    "tep70-headers.h"
#include    "tep70-signals.h"

#include    "battery.h"
#include    "relay.h"
#include    "fuel-tank.h"
#include    "electric-fuel-pump.h"
#include    "disel.h"
#include    "time-relay.h"
#include    "electric-oil-pump.h"
#include    "starter-generator.h"
#include    "voltage-regulator.h"
#include    "dc-motor-compressor.h"
#include    "pressure-regulator.h"
#include    "ubt367m.h"
#include    "ept-converter.h"
#include    "ept-pass-control.h"
#include    "tep70-switcher.h"
#include    "field-generator.h"
#include    "trac-generator.h"
#include    "field-regulator.h"
#include    "trac-motor.h"
#include    "sl2m.h"
#include    "tep70-horn.h"
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
class TEP70 : public Vehicle
{
public:

    /// Конструктор
    TEP70();

    /// Деструктор
    ~TEP70();

    /// Инициализация тормозных приборов
    void initBrakeDevices(double p0, double pTM, double pFL);

private:

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

    /// Главный воздушный резервуар (ГР)
    Reservoir           *main_reservoir;

    /// Мотор-компрессор
    DCMotorCompressor   *motor_compressor;

    /// Регулятор давления в ГР
    PressureRegulator   *press_reg;

    /// Реле РУ18
    Relay               *ru18;

    /// Контактор мотор-компрессора КТК1
    Relay               *ktk1;

    /// Контактор мотор-компрессора КТК2
    Relay               *ktk2;

    /// Реле времени РВ6
    TimeRelay           *rv6;

    /// Устройство блокировки тормозов усл. №367М
    BrakeLock           *ubt367m;

    /// Кран машиниста усл. №395
    BrakeCrane          *krm;

    /// Кран вспомогательного тормоза усл. №254
    LocoCrane           *kvt;

    /// Воздухораспределитель усл. №242
    AirDistributor      *vr;

    /// Электровоздухораспределитель усл. №305
    ElectroAirDistributor   *evr;

    /// Переключательный клапан
    SwitchingValve          *zpk;

    /// Реле давления усл. №304
    PneumoReley             *rd304;

    /// Тройник для распределения воздуха от переключательного клапана
    /// к тележкам
    PneumoSplitter          *pneumo_splitter;

    /// Передняя тележка
    BrakeMech               *fwd_trolley;

    /// Задняя тележка
    BrakeMech               *bwd_trolley;

    /// Запасный резервуар
    Reservoir               *zr;

    /// ЭПК автостопа
    AutoTrainStop           *epk;

    /// Преобразователь питания ЭПТ
    EPTConverter            *ept_converter;

    /// Блок управления ЭПТ
    EPTPassControl          *ept_pass_control;

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

    /// Скоростемер
    //SL2M                    *speed_meter;

    /// Кнопка "Пуск дизеля"
    bool    button_disel_start;

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
    TEP70Horn   *horn;

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

    /// Тумблер "Ослабление поля I ступени руч./авт."
    TEP70Switcher tumbler_field_weak1;

    /// Тумблер "Ослабление поля II ступени руч./авт."
    TEP70Switcher tumbler_field_weak2;

    /// Тумблер "Управление жалюзи воды руч./авт."
    TEP70Switcher tumbler_water_zaluzi;

    /// Тумблер "Управление жалюзи масла руч./авт."
    TEP70Switcher tumbler_oil_zaluzi;

    /// Ключ ЭПК
    Trigger  epk_key;

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

    /// Инициализация тормозной системы
    void initPneumoBrakeSystem();

    /// Инициализация ЭПТ
    void initEPT();

    /// Инициализация электрической передачи
    void initElectroTransmission();

    /// Инициализация прочего оборудования
    void initOther();

    /// Инициализация МСУТ
    void initMSUT();

    /// Инициализация звуков
    void initSounds();

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

    /// Шаг моделирвания масляной системы
    void stepOilSystem(double t, double dt);

    /// Шаг моделирования пневматической тормозной системы
    void stepPneumoBrakeSystem(double t, double dt);

    /// Шаг моделирования ЭПТ
    void stepEPT(double t, double dt);

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

    /// Вывод отладочной строки
    void debugOutput(double t, double dt);

    /// Формирование состояния сигнальных ламп
    float getLampState(double signal);

    /// Загрузка данных из конфигурационных файлов
    void loadConfig(QString cfg_path);

    /// Обработка клавиш
    void keyProcess();
};

#endif // TEP70_H
