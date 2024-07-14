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
#ifndef     VL60K_H
#define     VL60K_H

#include    "vl60k-headers.h"

/*!
 * \class
 * \brief Основной класс, описывающий весь электровоз
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class VL60k : public Vehicle
{
public:

    /// Конструктор
    VL60k();

    /// Деструктор
    ~VL60k();

    /// Инициализация тормозных приборов
    void initBrakeDevices(double p0, double pBP, double pFL);

private:

    enum
    {
        /// Объем главного резервуара (ГР), литров
        MAIN_RESERVOIR_VOLUME = 1200
    };

    float   pant1_pos;
    float   pant2_pos;
    float   gv_pos;
    bool    gv_return;

    /// Зарядное давление
    double  charge_press;

    /// Передаточное число редуктора
    double  ip;

    /// Подключение рукавов магистрали тормозных цилиндров к импульсной магистрали
    bool bc_hose_to_impulse_line;

    /// Имя модуля сцепного устройства
    QString coupling_module_name;
    /// Имя конфига сцепного устройства
    QString coupling_config_name;
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

    /// Регистратор, для записи параметров
    Registrator *reg;

    /// Сцепка спереди
    Coupling *coupling_fwd;
    /// Сцепка сзади
    Coupling *coupling_bwd;

    /// Расцепной рычаг спереди
    OperatingRod *oper_rod_fwd;
    /// Расцепной рычаг сзади
    OperatingRod *oper_rod_bwd;

    // Дальний ряд тумблеров приборной панели машиниста
//    /// Тригер тумблера "Прожектор яркий"
//    Trigger proj2_tumbler;
//    /// Тригер тумблера "Прожектор тусклый"
//    Trigger proj1_tumbler;
//    /// Тригер тумблера "Радиостанция"
//    Trigger radio_tumbler;
    /// Триггер тумблера "Цепи управления"
    Trigger cu_tumbler;
    /// Триггер тумблера "Токоприемник задний"
    Trigger pant2_tumbler;
    /// Триггер тумблера "Токоприемник передний"
    Trigger pant1_tumbler;
    /// Тригер тумблера "Токоприемники"
    Trigger pants_tumbler;
    /// Тригер тумблена "ГВ вкл. Возврат защиты"
    Trigger gv_return_tumbler;
    /// Триггер тумблера "ГВ вкл/выкл"
    Trigger gv_tumbler;

    // Ближний ряд тумблеров приборной панели машиниста
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

//    /// Тригер тумблера "Автоматическая подача песка"
//    Trigger autosand_tumbler;
    /// Триггеры тумблеров "Вентилятор 1-6"
    std::array<Trigger, NUM_MOTOR_FANS> mv_tumblers;
    /// Тригер тумблера "Компрессор"
    Trigger mk_tumbler;
    /// Тригер тумблера "Фазорасщепитель"
    Trigger fr_tumbler;

    enum
    {
        NUM_RB = 3,
        RB_1 = 0,
        RBP = 1,
        RBS = 2
    };

    /// Список звуков перестука
    QList<QString>   tap_sounds;

    /// Триггеры рукояток бдительности
    std::array<Trigger, NUM_RB>  rb;

    enum
    {
        NUM_PANTOGRAPHS = 2,
        WIRE_VOLTAGE = 25000
    };

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
    std::array<ACMotorFan *, NUM_MOTOR_FANS> motor_fans;

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

    /// Запасный резервуар
    Reservoir   *supply_reservoir;

    /// Концевой кран тормозной магистрали спереди
    PneumoAngleCock *anglecock_bp_fwd;

    /// Концевой кран тормозной магистрали сзади
    PneumoAngleCock *anglecock_bp_bwd;

    /// Рукав тормозной магистрали спереди
    PneumoHose  *hose_bp_fwd;

    /// Рукав тормозной магистрали сзади
    PneumoHose  *hose_bp_bwd;

    /// Импульсная магистраль с ложным тормозным цилиндром
    Reservoir  *impulse_line;

    /// Тройник подключения тележек к магистрали тормозных цилиндров
    PneumoSplitter  *bc_splitter;

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

    /// Система подачи песка
    SandingSystem   *sand_system;

    std::vector<Trigger *> triggers;
    Timer   *autoStartTimer;
    size_t  start_count;


    /// Общая инициализация локомотива
    void initialization();

    /// Инициализация сцепных устройств
    void initCouplings(const QString &modules_dir, const QString &custom_cfg_dir);

    /// Инициализация токоприемников
    void initPantographs(const QString &modules_dir, const QString &custom_cfg_dir);

    /// Иницаализация высоковольтной части схемы (ГВ, тяговый трансформатор)
    void initHighVoltageScheme(const QString &modules_dir, const QString &custom_cfg_dir);

    /// Инициализация вспомогательных машин (ФР, МК, МВ1 - МВ6)
    void initSupplyMachines(const QString &modules_dir, const QString &custom_cfg_dir);

    /// Инициализация питательной магистрали
    void initPneumoSupply(const QString &modules_dir, const QString &custom_cfg_dir);

    /// Инициализация приборов управления тормозами
    void initBrakesControl(const QString &modules_dir, const QString &custom_cfg_dir);

    /// Инициализация тормозного оборудования
    void initBrakesEquipment(const QString &modules_dir, const QString &custom_cfg_dir);

    /// Инициализация схемы управления тягой
    void initTractionControl(const QString &modules_dir, const QString &custom_cfg_dir);

    void initOtherEquipment(const QString &modules_dir, const QString &custom_cfg_dir);

    void initTriggers();

    void initTapSounds();

    /// Предварительные расчёты перед симуляцией
    void preStep(double t);

    /// Предварительный расчёт координат сцепных устройств
    void preStepCouplings(double t);

    /// Шаг симуляции всех систем электровоза
    void step(double t, double dt);

    /// Моделирование сцепных устройств
    void stepCouplings(double t, double dt);

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

    void stepTractionControl(double t, double dt);

    void stepLineContactors(double t, double dt);

    void stepOtherEquipment(double t, double dt);

    void stepSoundSignalsOutput(double t, double dt);

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

#endif // VL60K_H

