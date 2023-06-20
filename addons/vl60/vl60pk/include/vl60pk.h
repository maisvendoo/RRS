//------------------------------------------------------------------------------
//
//      Магистральный электровоз переменного тока ВЛ60.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Роман Бирюков (РомычРЖДУЗ)
//
//      Дата: 28/03/2019
//
//------------------------------------------------------------------------------
#ifndef     VL60_H
#define     VL60_H

#include    "vl60pk-headers.h"

/*!
 * \class
 * \brief Основной класс, описывающий весь электровоз
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class VL60pk : public Vehicle
{
public:

    /// Конструктор
    VL60pk();

    /// Деструктор
    ~VL60pk();

    /// Инициализация тормозных приборов
    void initBrakeDevices(double p0, double pBP, double pFL);

private:

    enum
    {
        NUM_PANTOGRAPHS = 2,
        WIRE_VOLTAGE = 25000
    };

    enum
    {
        NUM_MOTOR_FANS = 6,
        MV1 = 0,
        MV2 = 1,
        MV3 = 2,
        MV4 = 3,
        MV5 = 4,
        MV6 = 5
    };

    enum
    {
        /// Объем главного резервуара (ГР), литров
        MAIN_RESERVOIR_VOLUME = 1200
    };

    /// Список звуков перестука
    QList<QString> tap_sounds;

    /// Напряжение аккумуляторной батареи
    double  U_bat;

    float   pant1_pos;
    float   pant2_pos;
    float   gv_pos;
    bool    gv_return;

    /// Зарядное давление
    double  charge_press;

    /// Передаточное число редуктора
    double  ip;

    /// Тригер тумблера "Токоприемники"
    Trigger pants_tumbler;
    /// Триггер тумблера "Токоприемник передний"
    Trigger pant1_tumbler;
    /// Триггер тумблера "Токоприемник задний"
    Trigger pant2_tumbler;

    /// Триггер тумблера "ГВ вкл/выкл"
    Trigger gv_tumbler;

    /// Тригер тумблена "ГВ вкл. Возврат защиты"
    Trigger gv_return_tumbler;

    /// Тригер тумблера "Фазорасщепитель"
    Trigger fr_tumbler;

    /// Триггеры тумблеров управления мотор-вентиляторами
    std::array<Trigger, NUM_MOTOR_FANS> mv_tumblers;

    /// Тригер тумблера управления мотор-компрессором
    Trigger mk_tumbler;

    /// Триггер тумблера "Цепи управления"
    Trigger cu_tumbler;

    enum
    {
        NUM_RB = 3,
        RB_1 = 0,
        RBP = 1,
        RBS = 2
    };

    /// Триггеры рукояток бдительности
    std::array<Trigger, NUM_RB>  rb;

    /// Токоприемники
    std::array<Pantograph *, NUM_PANTOGRAPHS>   pantographs;

    /// Главный выключатель (ГВ)
    ProtectiveDevice    *main_switch;

    /// Механизм киловольтметра КС
    Oscillator      *gauge_KV_ks;

    /// Тяговый трансформатор
    TracTransformer *trac_trans;

    /// Асинхронный расщепитель фаз
    PhaseSplitter   *phase_spliter;

    /// Мотор-вентиляторы
    std::array<MotorFan *, NUM_MOTOR_FANS> motor_fans;

    /// Мотор-компрессор
    ACMotorCompressor *motor_compressor;

    /// Регулятор давления в ГР
    PressureRegulator *press_reg;

    /// Главный резервуар
    Reservoir   *main_reservoir;

    /// Концевой кран питательной магистрали спереди
    PneumoAngleCock *anglecock_fl_fwd;

    /// Концевой кран питательной магистрали сзади
    PneumoAngleCock *anglecock_fl_bwd;

    /// Рукав питательной  магистрали спереди
    PneumoHose      *hose_fl_fwd;

    /// Рукав питательной  магистрали сзади
    PneumoHose      *hose_fl_bwd;

    /// Блокировочное устройство УБТ усл.№367м
    BrakeLock   *brake_lock;

    /// Поездной кран машиниста усл.№395
    BrakeCrane  *brake_crane;

    /// Кран впомогательного тормоза усл.№254
    LocoCrane   *loco_crane;

    /// Тормозная магистраль
    Reservoir   *brakepipe;

    /// Воздухораспределитель
    AirDistributor  *air_dist;

    /// Электровоздухораспределитель
    ElectroAirDistributor  *electro_air_dist;

    /// Запасный резервуар
    Reservoir   *supply_reservoir;

    /// Концевой кран тормозной магистрали спереди
    PneumoAngleCock *anglecock_bp_fwd;

    /// Концевой кран тормозной магистрали сзади
    PneumoAngleCock *anglecock_bp_bwd;

    /// Рукав тормозной магистрали спереди
    PneumoHoseEPB   *hose_bp_fwd;

    /// Рукав тормозной магистрали сзади
    PneumoHoseEPB   *hose_bp_bwd;

    /// Переключательный клапан магистрали тормозных цилиндров ЗПК
    SwitchingValve  *bc_switch_valve;

    /// Повторительное реле давления усл.№304
    PneumoRelay     *bc_pressure_relay;

    enum
    {
        NUM_TROLLEYS = 2,
        NUM_AXIS_PER_TROLLEY = 3,
        TROLLEY_FWD = 0,
        TROLLEY_BWD = 1
    };

    /// Тормозные механизмы тележек
    std::array<BrakeMech *, NUM_TROLLEYS> brake_mech;

    /// Концевой кран магистрали тормозных цилиндров спереди
    PneumoAngleCock  *anglecock_bc_fwd;

    /// Концевой кран магистрали тормозных цилиндров сзади
    PneumoAngleCock  *anglecock_bc_bwd;

    /// Рукав магистрали тормозных цилиндров спереди
    PneumoHose  *hose_bc_fwd;

    /// Рукав магистрали тормозных цилиндров сзади
    PneumoHose  *hose_bc_bwd;

    /// Тумблер включения ЭПТ
    Trigger         epb_switch;

    /// Источник питания ЭПТ
    EPBConverter    *epb_converter;

    /// Блок управления двухпроводного ЭПТ
    EPBControl  *epb_control;

    /// Контроллер машиниста
    ControllerKME_60_044    *controller;

    /// Главный контроллер (переключение обмоток тягового трансформатора)
    EKG_8G                  *main_controller;

    enum
    {
        NUM_VU = 2,
        VU1 = 0,
        VU2 = 1
    };

    /// Выпрямительные установки
    std::array<Rectifier *, NUM_VU> vu;

    /// Механизм киловольтметра ТЭД
    Oscillator  *gauge_KV_motors;

    enum
    {
        NUM_MOTORS = 6,
        TED1 = 0,
        TED2 = 1,
        TED3 = 2,
        TED4 = 3,
        TED5 = 4,
        TED6 = 5
    };

    /// Тяговые электродвигатели
    std::array<DCMotor *, NUM_MOTORS>  motor;

    /// Реле перегрузки ТЭД
    std::array<OverloadRelay *, NUM_MOTORS> overload_relay;

    /// Линейные контакторы ТЭД
    std::array<Trigger, NUM_MOTORS> line_contactor;

    /// Локомотивный скоростемер
    SL2M    *speed_meter;

    /// Свисток и тифон
    TrainHorn   *horn;

    /// Регистратор, для записи параметров
    Registrator *reg;

    std::vector<Trigger *> triggers;
    Timer   *autoStartTimer;
    size_t  start_count;

    // ЭПТ -------------------------

    /// Общая инициализация локомотива
    void initialization();

    /// Инициализация токоприемников
    void initPantographs();

    /// Иницаализация высоковольтной части схемы (ГВ, тяговый трансформатор)
    void initHighVoltageScheme();

    /// Инициализация вспомогательных машин (ФР, МК, МВ1 - МВ6)
    void initSupplyMachines();

    /// Инициализация питательной магистрали
    void initPneumoSupply(QString modules_dir);

    /// Инициализация приборов управления тормозами
    void initBrakesControl(QString modules_dir);

    /// Инициализация тормозного оборудования
    void initBrakesEquipment(QString modules_dir);

    /// Инициализация ЭПТ
    void initEPB(QString modules_dir);

    /// Инициализация схемы управления тягой
    void initTractionControl();

    void initOtherEquipment();

    void initTriggers();

    void initTapSounds();

    /// Шаг симуляции всех систем электровоза
    void step(double t, double dt);

    void stepPantographsControl(double t, double dt);

    void stepMainSwitchControl(double t, double dt);

    void stepTracTransformer(double t, double dt);

    void stepPhaseSplitter(double t, double dt);

    void stepMotorFans(double t, double dt);

    /// Моделирование питательной магистрали
    void stepPneumoSupply(double t, double dt);

    /// Моделирование приборов управления тормозами
    void stepBrakesControl(double t, double dt);

    /// Моделирование тормозного оборудования
    void stepBrakesEquipment(double t, double dt);

    /// Моделирование ЭПТ
    void stepEPB(double t, double dt);

    void stepTractionControl(double t, double dt);

    void stepLineContactors(double t, double dt);

    void stepOtherEquipment(double t, double dt);

    void stepTapSound();

    void lineContactorsControl(bool state);

    float isLineContactorsOff();

    void stepSignalsOutput();

    void stepDecodeAlsn();

    double getTractionForce();

    bool getHoldingCoilState() const;

    /// Обработка нажатий клавиш
    void keyProcess();

    void debugPrint(double t, double dt);

    void load_brakes_config(QString path);

    void loadConfig(QString cfg_path);

private slots:

    void slotAutoStart();
};

#endif // VL60_H

