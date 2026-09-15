#ifndef     EP1M_H
#define     EP1M_H

#include    <QMap>
#include    <array>
#include    <ep1m-headers.h>
#include    <ep1m-autopilot-types.h>

//---------------------------------------------------------------------
//
//---------------------------------------------------------------------
class EP1m : public Vehicle
{
public:

    /// Конструктор класса
    EP1m(QObject* parent = nullptr);

    /// Деструктор класса
    ~EP1m() override;

    /// Инициализация тормозных приборов
    void initBrakeDevices(double p0, double pBP, double PFL) override;

private:

    enum
    {
        CABS_NUM = 2,
        CAB1 = 0,
        CAB2 = 1
    };

    /// Имя модуля сцепного устройства
    QString coupling_module_name = "sa3";
    /// Имя конфига сцепного устройства
    QString coupling_config_name = "sa3";

    QString autopilot_module_name = "ep1m-autopilot";
    /// Имя конфига модуля автоведения
    QString autopilot_config_name = "ep1m-autopilot";
    /// Каталог поиска кастомных модулей
    QString custom_modules_dir = "ep1m";

    /// Сцепка спереди
    Coupling* coupling_fwd = nullptr;
    /// Сцепка сзади
    Coupling* coupling_bwd = nullptr;

    /// Расцепной рычаг спереди
    OperatingRod* oper_rod_fwd = nullptr;
    /// Расцепной рычаг сзади
    OperatingRod* oper_rod_bwd = nullptr;

    /// Панель тумблеров
    EP1MTumblersPanel*   tumblers_panel[CABS_NUM] = {nullptr, nullptr};

    /// МСУД
    MSUD*    msud = nullptr;

    /// Напряжение питания цепей управления
    double  Ucc = 0.0;

    /// Аккумуляторная батарея 21KL-125P
    Battery* battery = nullptr;

    /// Шкаф питания ШП-21
    PowerSupply* power_supply = nullptr;

    /// Ток, потребляемый цепями управления
    double Icc = 0.0;

    /// Тяговый трансформатор
    TractionTransformer* trac_trans = nullptr;

    /// Главный выключатель
    ProtectiveDevice* main_switch = nullptr;

    /// Напряжение на крышевой шине
    double Ukr = 0.0;

    /// Контактор КМ5 включения ШП-21
    Relay*  km5 = nullptr;

    /// Вентиль защиты ВЗ-6
    ElectroValve*   safety_valve = nullptr;

    /// Промежуточное реле KV44
    Relay*  kv44 = nullptr;

    /// Промежуточное реле KV39
    Relay*  kv39 = nullptr;

    Relay*  kv21 = nullptr;

    Relay*  kv22 = nullptr;

    Relay*  kv23 = nullptr;

    Relay*  kv41 = nullptr;

    Relay*  km7 = nullptr;

    Relay*  km8 = nullptr;

    Relay*  km9 = nullptr;

    Relay*  km11 = nullptr;

    Relay*  km12 = nullptr;

    Relay*  km13 = nullptr;

    /// Контроллер машиниста
    TracController* km[CABS_NUM] = {nullptr, nullptr};

    /// Блок сигнализации БС-002
    SignalizationModule* signals_module = nullptr;

    /// Яркость подсветки пульта (не реализовано)
    float panel_light_intensity[CABS_NUM] = {0.75f, 0.75f};
    /// Яркость подсветки приборов
    float device_light_intensity[CABS_NUM] = {0.75f, 0.75f};

    bool return_GV = false;

    /// Преобразователь частоты и числа фаз (ПЧФ)
    FreqPhaseConverter* freq_phase_conv  = nullptr;

    /// Передаточное число тягового редуктора
    double ip = 1.0;

    /// Сигнал на проводе Н36
    bool is_H36 = false;

    /// Сигнал на проводе Н211
    bool is_N211_on = false;

    /// Сигнал на проводе Н212
    bool is_N212_on = false;

    /// Регистрировать параметры движения
    bool is_Registrator_on = false;

    /// Сигнал на проводе Н45
    bool is_N45_on = false;

    /// Сигнал на проводе Н53
    bool is_N53_on = false;

    /// Реверсор
    Reversor* reversor = nullptr;

    /// Мотор-компрессор
    ACMotorCompressor*  motor_compressor  = nullptr;

    /// Регулятор давления в ГР
    PressureRegulator*  press_reg = nullptr;

    /// Главный резервуар
    Reservoir*          main_reservoir = nullptr;

    /// Концевой кран питательной магистрали спереди
    PneumoAngleCock*    anglecock_fl_fwd = nullptr;

    /// Концевой кран питательной магистрали сзади
    PneumoAngleCock*    anglecock_fl_bwd = nullptr;

    /// Рукав питательной  магистрали спереди
    PneumoHose*         hose_fl_fwd = nullptr;

    /// Рукав питательной  магистрали сзади
    PneumoHose*         hose_fl_bwd = nullptr;

    double  charge_press = 0.5;

    /// Сигнализатор давления в ТМ
    HysteresisRelay*    sp4 = nullptr;

    /// Блокировочное устройство УБТ усл.№367м
    PneumoBrakeLock*    brake_lock[CABS_NUM] = {nullptr, nullptr};

    /// Поездной кран машиниста усл.№395
    BrakeCrane*         brake_crane[CABS_NUM] = {nullptr, nullptr};

    /// Кран впомогательного тормоза усл.№215
    LocoCrane*          loco_crane[CABS_NUM] = {nullptr, nullptr};

    /// ЭПК автостопа
    AutoTrainStop*      epk[CABS_NUM] = {nullptr, nullptr};

    /// Тормозная магистраль
    Reservoir*          brakepipe = nullptr;

    /// Воздухораспределитель
    AirDistributor*     air_dist = nullptr;

    /// Электровоздухораспределитель
    ElectroAirDistributor*  electro_air_dist = nullptr;

    /// Запасный резервуар
    Reservoir*          supply_reservoir = nullptr;

    /// Концевой кран тормозной магистрали спереди
    PneumoAngleCock*    anglecock_bp_fwd = nullptr;

    /// Концевой кран тормозной магистрали сзади
    PneumoAngleCock*    anglecock_bp_bwd = nullptr;

    /// Рукав тормозной магистрали спереди
    PneumoHoseEPB*      hose_bp_fwd = nullptr;

    /// Рукав тормозной магистрали сзади
    PneumoHoseEPB*      hose_bp_bwd = nullptr;

    /// Повторительное пневмореле для давления от воздухораспределителя РД4
    PneumoRelay*        rd4 = nullptr;

    /// Переключательный клапан КП1
    SwitchingValve*     kp1 = nullptr;

    /// Переключательный клапан КП2
    SwitchingValve*     kp2 = nullptr;

    /// Переключательный клапан КП5
    SwitchingValve*     kp5 = nullptr;

    /// Тройники для распределения воздуха от переключательного клапана
    /// к тележкам
    std::array<PneumoSplitter*, 2> bc_splitter;

    enum
    {
        NUM_TROLLEYS = 3,
        NUM_AXIS_PER_TROLLEY = 2,
        TROLLEY_FWD = 0,
        TROLLEY_MID = 1,
        TROLLEY_BWD = 2
    };

    /// Повторительное реле давления тележек
    std::array<PneumoRelay*, NUM_TROLLEYS> bc_pressure_relay;

    /// Тормозные механизмы тележек
    std::array<BrakeMech*, NUM_TROLLEYS> brake_mech;

    /// Источник питания ЭПТ
    EPBConverter*       epb_converter = nullptr;

    /// Блок управления двухпроводного ЭПТ
    EPBControl*         epb_control = nullptr;

    /// Блок электронный локомотивный (БЭЛ)
    KLUB*   klub_BEL = nullptr;

    /// Реле подачи питания на МСУД
    Relay*  km43 = nullptr;

    /// Промежуточное реле KV11
    Relay*  kv11 = nullptr;

    /// Промежуточное реле KV12
    Relay*  kv12 = nullptr;

    /// Промежуточное реле KV13
    Relay*  kv13 = nullptr;

    /// Промежуточное реле KV14
    Relay*  kv14 = nullptr;

    /// Промежуточное реле KV15
    Relay*  kv15 = nullptr;

    /// Реле КЛУБ
    Relay*  kv84 = nullptr;

    /// Реле выдержки времени КТ10
    TimeRelay*  kt10 = nullptr;

    /// Реле выдержки времени КТ1
    TimeRelay*  kt1 = nullptr;

    /// Тормозной переключатель
    BrakeSwitcher* qt1 = nullptr;

    /// Контактор KM41
    Relay*  km41 = nullptr;

    /// Контактор KM42
    Relay*  km42 = nullptr;

    /// Реле выдержки времени КТ4
    TimeRelay*  kt4 = nullptr;

    /// Реле выдержки времени КТ5
    TimeRelay*  kt5 = nullptr;

    /// Контактор мотор-вентилятора ББР
    Relay*  km14 = nullptr;

    /// Контактор К1
    Relay*  k1 = nullptr;

    /// Вентиль отпуска У3
    PneumoElectroValve* Y3 = nullptr;

    /// Вентиль замещения ЭДТ У4
    PneumoElectroValve* Y4 = nullptr;

    /// Вентиль усиления торможения У5
    PneumoElectroValve* Y5 = nullptr;

    /// Панель пневматических редукторов
    PneumoReducerPanel* pneumo_red_panel = nullptr;

    /// Датчик давления магистрали передней тележки (ТЦ1 и ТЦ2)
    PressureSensor* sp3 = nullptr;

    /// Датчик аварийного давления в ТМ SP6
    PressureSensor* sp6 = nullptr;

    /// Регистратор параметров движения (для отладки и испытаний)
    Registrator* registrator = nullptr;

    /// Свисток и тифон
    TrainHorn* horn[CABS_NUM] = {nullptr, nullptr};

    /// Система подачи песка
    SandingSystem* sand_system = nullptr;

    /// Шунты ослабления возбуждения ТЭД
    ShuntsModule* shunts = nullptr;

    /// Выпрямительна установка возбуждения ВУВ-118
    FieldRect* field_rect = nullptr;

    enum
    {
        PANT_NUMBER = 2,
        PANT1 = 0,
        PANT2 = 1
    };

    /// Данные, передаваемые в МСУД-Н
    msud_input_t msud_input = msud_input_t();

    /// Токоприемники
    std::array<Pantograph*, PANT_NUMBER> pant;

    /// Тумблеры и кнопки вне блокируемой панели
    TriggerControl tumblers[TUMBLERS_COUNT][CABS_NUM];
    SwitcherControl switchers[SWITCHERS_COUNT][CABS_NUM];
    /// Выключатель шкафа питания ШП-21
    TriggerControl tumbler_power_supply;
    /// Дверца тумбы с устройством блокировки тормозов
    TriggerControl brake_lock_door[CABS_NUM];

    /// Мотор-вентиляторы М11 - М13
    std::array<MotorFan*, MOTOR_FANS_NUM> motor_fan;

    std::array<TractionMotor*, TRAC_MOTORS_NUM> trac_motor;

    /// Быстродействующие выключатели ТЭД
    std::array<FastSwitch*, TRAC_MOTORS_NUM> fast_switch;

    /// Выпрямительно инверторные преобразователи
    enum
    {
        RECT_INV_CONV_NUM = 2,
        VIP1 = 0,
        VIP2 = 1
    };

    std::array<RectInvertConverter*, RECT_INV_CONV_NUM> vip;

    /// Ограничения скорости на путевой инфраструктуре для кабины А
    SpeedMap*   speedmap_fwd = nullptr;
    /// Ограничения скорости на путевой инфраструктуре для кабины Б
    SpeedMap*   speedmap_bwd = nullptr;

    /// Приёмная катушка АЛСН для кабины А
    CoilALSN*   coil_ALSN_fwd = nullptr;
    /// Приёмная катушка АЛСН для кабины Б
    CoilALSN*   coil_ALSN_bwd = nullptr;

    /// Дешифратор сигнала АЛСН
    DecoderALSN* alsn_decoder[CABS_NUM] = {nullptr, nullptr};


    /// Чтение конфигурационного файла
    void loadConfig(QString cfg_path) override;


    /// Общая инициализация локомотива
    void initialization() override;

    /// Инициализация сцепных устройств
    void initCouplings(const QString& modules_dir, const QString& custom_cfg_dir);

    /// Инициализация подсистемы питания цепей управления
    void initControlPower(const QString& modules_dir, const QString& custom_cfg_dir);

    /// Инициализация цепей управления
    void initControlCircuit(const QString& modules_dir, const QString& custom_cfg_dir);

    /// Инициализация пульта управления в кабине
    void initPanel(const QString& modules_dir, const QString& custom_cfg_dir);

    /// Инициализация МСУД
    void initMSUD(const QString& modules_dir, const QString& custom_cfg_dir);

    /// Инициализация силовой схемы
    void initPowerCircuit(const QString& modules_dir, const QString& custom_cfg_dir);

    /// Инициализация питательной магистрали
    void initPneumoSupply(const QString& modules_dir, const QString& custom_cfg_dir);

    /// Инициализация приборов управления тормозами
    void initBrakesControl(const QString& modules_dir, const QString& custom_cfg_dir);

    /// Инициализация тормозного оборудования
    void initBrakesEquipment(const QString& modules_dir, const QString& custom_cfg_dir);

    /// Инициализация ЭПТ
    void initEPB(const QString& modules_dir, const QString& custom_cfg_dir);

    /// Инициализация вспомогательных машин
    void initAuxMachines(const QString& modules_dir, const QString& custom_cfg_dir);

    /// Инициализация устройств безопасности
    void initSafetyDevices(const QString& modules_dir, const QString& custom_cfg_dir);

    /// Инициализация прочих устройств
    void initOtherEquipment(const QString& modules_dir, const QString& custom_cfg_dir);

    /// Инициализация управления
    void initControl(const QString& modules_dir, const QString& custom_cfg_dir);

    /// Инициализация регистратора параметров движения
    void initRegistartor(const QString& modules_dir, const QString& custom_cfg_dir);


    /// Процесс симуляции
    void process(const simulator_time_t& t, const double& dt) override;

    /// Управление
    void keyProcess(const simulator_time_t& t, const double& dt);

    /// Отладочная строка
    void debugPrint(const simulator_time_t& t, const double& dt);

    /// Сигналы для анимации
    void signalsOutput(const simulator_time_t& t, const double& dt);

    /// Сигналы для озвучки
    void soundsOutput(const simulator_time_t& t, const double& dt);


    /// Предварительные расчёты перед симуляцией
    void preStep(const double& t) override;

    /// Предварительный расчёт координат сцепных устройств
    void preStepCouplings(const double& t);


    /// Шаг симуляции всех систем электровоза
    void step(const double& t, const double& dt) override;

    /// Моделирование сцепных устройств
    void stepCouplings(const double& t, const double& dt);

    void stepControlPower(const double& t, const double& dt);

    void stepControlCircuit(const double& t, const double& dt);

    bool getHoldingCoilState();

    /// Управление схемой тяги
    void stepTractionControl(const double& t, const double& dt);

    /// Управление схемой рекуперации
    void stepRecuperationControl(const double& t, const double& dt);

    void stepPanel(const double& t, const double& dt);

    void setSignalsModuleInputs();

    void stepMSUD(const double& t, const double& dt);

    void stepPowerCircuit(const double& t, const double& dt);

    /// Шаг моделирования питательной магистрали
    void stepPneumoSupply(const double& t, const double& dt);

    /// Шаг моделирования приборов управления тормозами
    void stepBrakesControl(const double& t, const double& dt);

    /// Шаг моделирования тормозного оборудования
    void stepBrakesEquipment(const double& t, const double& dt);

    /// Шаг моделирования ЭПТ
    void stepEPB(const double& t, const double& dt);

    void stepAuxMachines(const double& t, const double& dt);

    void stepSafetyDevices(const double& t, const double& dt);

    void stepOtherEquipment(const double& t, const double& dt);

    void stepRegistration(const double& t, const double& dt);


    void load_brakes_config(QString path);

    double calcTracForce();

    Timer *autoStartTimer = new Timer(0.5, false);

    std::vector<Trigger *> autostart_triggers;
    size_t start_count = 0;
    size_t autostart_cab = 0;

    /// Направление последовательности автозапуска:
    /// false - включение, true - выключение
    bool autostart_shutdown = false;

    /// Инициализация автозапуска
    bool initAutostartProgram(int cab_autostart_request);

    /// Инициализация автовыключения
    bool initShutdownProgram(int cab_shutdown_request);

    /// Шаг последовательности выключения электровоза
    void stepShutdownSequence();

    ep1m_control_t *auto_control[CABS_NUM] = {nullptr, nullptr};

    ep1m_feedback_t *auto_feedback[CABS_NUM];

    TriggerControl autopilot_switcher[CABS_NUM];

    /// Инициализация автоведения
    void initAutopilot(const QString& modules_dir, const QString& custom_cfg_dir);

    /// Подготовка рабочей кабины к включению автоведения
    void prepareCabineForAutopilot(int my_cab_idx, int other_cab_idx);

    void OnAutopilot() override;

    void OffAutopilot() override;

    /// Шаг работы автоведения
    void stepAutopilot(double t, double dt);

    void stepControls(const double &t, const double &dt);

    void applyControlCommand(int cab_idx, int id, float value);

    std::array<QMap<int, float>, CABS_NUM> prev_control_values;

    /// Условный переключатель поездного/маневрового режима
    TriggerControl shunting_mode_switcher[CABS_NUM];

private slots:

    void slotAutostart();

    void slotInitTrainForAutopilot();
};

#endif // EP1M_H
