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
#ifndef     VL60PK_H
#define     VL60PK_H

#include "trigger-control.h"
#include "vehicle.h"

#include <QString>

#include <array>

class ACMotorCompressor;
class ACMotorFan;
class AirDistributor;
class AutoTrainStop;
class BrakeCrane;
class BrakeLock;
class BrakeMech;
class CoilALSN;
class ControllerKME_60_044;
class Coupling;
class DCMotor;
class DecoderALSN;
class EKG_8G;
class ElectroAirDistributor;
class EPBControl;
class EPBConverter;
class LocoCrane;
class OperatingRod;
class Oscillator;
class OverloadRelay;
class Pantograph;
class PhaseSplitter;
class PneumoAngleCock;
class PneumoHose;
class PneumoHoseEPB;
class PneumoRelay;
class PressureRegulator;
class ProtectiveDevice;
class Rectifier;
class Registrator;
class Relay;
class Reservoir;
class SafetyDevice;
class SandingSystem;
class SL2M;
class SpeedMap;
class SpotLight;
class SwitchingValve;
class Timer;
class TracTransformer;
class TrainHorn;

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
        /// Объем главного резервуара (ГР), литров
        MAIN_RESERVOIR_VOLUME = 1200
    };

    /// Напряжение аккумуляторной батареи
    double  U_bat = 55.0;

    float   pant1_pos = 0.0;
    float   pant2_pos = 0.0;
    float   gv_pos = 0.0;
    bool    gv_return = false;

    /// Зарядное давление
    double  charge_press = 0.0;

    /// Передаточное число редуктора
    double  ip = 2.73;

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
    /// Имя модуля поездного крана
    QString brake_crane_module_name = "krm395";
    /// Имя конфига поездного крана
    QString brake_crane_config_name = "krm395";
    /// Имя модуля локомотивного крана
    QString loco_crane_module_name = "kvt254";
    /// Имя конфига локомотивного крана
    QString loco_crane_config_name = "kvt254";
    /// Имя модуля воздухораспределителя
    QString airdist_module_name = "vr292";
    /// Имя конфига воздухорапределителя
    QString airdist_config_name = "vr292";
    /// Имя модуля электровоздухораспределителя
    QString electro_airdist_module_name = "evr305";
    /// Имя конфига электровоздухорапределителя
    QString electro_airdist_config_name = "evr305";

    /// Регистратор, для записи параметров
    Registrator *reg = nullptr;

    /// Сцепка спереди
    Coupling *coupling_fwd = nullptr;
    /// Сцепка сзади
    Coupling *coupling_bwd = nullptr;

    /// Расцепной рычаг спереди
    OperatingRod *oper_rod_fwd = nullptr;
    /// Расцепной рычаг сзади
    OperatingRod *oper_rod_bwd = nullptr;

    // Дальний ряд тумблеров приборной панели машиниста
    /// Триггер тумблера "Прожектор яркий"
    TriggerControl spotlight_high_tumbler[CABS_NUM];
    /// Триггер тумблера "Прожектор тусклый"
    TriggerControl spotlight_low_tumbler[CABS_NUM];
    /// Триггер тумблера "Радиостанция"
    TriggerControl radio_tumbler[CABS_NUM];
    /// Триггер тумблера "Цепи управления"
    TriggerControl cu_tumbler[CABS_NUM];
    /// Триггер тумблера "Токоприемник передний"
    TriggerControl pant1_tumbler[CABS_NUM];
    /// Триггер тумблера "Токоприемник задний"
    TriggerControl pant2_tumbler[CABS_NUM];
    /// Триггер тумблера "Токоприемники"
    TriggerControl pants_tumbler[CABS_NUM];
    /// Триггер тумблера "ГВ вкл. Возврат защиты"
    TriggerControl gv_return_tumbler[CABS_NUM];
    /// Триггер тумблера "ГВ вкл/выкл"
    TriggerControl gv_tumbler[CABS_NUM];

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

    /// Тригер тумблера "Автоматическая подача песка"
    TriggerControl autosand_tumbler[CABS_NUM];
    /// Триггеры тумблеров "Вентилятор 1-6"
    TriggerControl mv_tumblers[CABS_NUM][NUM_MOTOR_FANS];
    /// Тригер тумблера "Компрессор"
    TriggerControl mk_tumbler[CABS_NUM];
    /// Тригер тумблера "Фазорасщепитель"
    TriggerControl fr_tumbler[CABS_NUM];

    // Ряд тумблеров на приборной панели помощника машиниста
    /// Триггер тумблера "Тифон"
    TriggerControl P_tifon_tumbler[CABS_NUM];
    /// Триггер тумблера "Свисток"
    TriggerControl P_whistle_tumbler[CABS_NUM];
    /// Триггер тумблера "Обогрев кабины"
    TriggerControl P_cab_heat_tumbler[CABS_NUM];
    /// Триггер тумблера "Тусклое освещение кабины"
    TriggerControl P_cab_light_low_tumbler[CABS_NUM];
    /// Триггер тумблера "Яркое освещение кабины"
    TriggerControl P_cab_light_high_tumbler[CABS_NUM];
    /// Триггер тумблера в резерве
    TriggerControl P_reserv1_tumbler[CABS_NUM];
    /// Триггер тумблера "Освещение ходовой"
    TriggerControl P_light_chassis_tumbler[CABS_NUM];
    /// Триггер тумблера "Освещение приборов"
    TriggerControl P_light_devices_tumbler[CABS_NUM];
    /// Триггер тумблера "Фонарь левый буферный"
    TriggerControl P_bufferlight_L_tumbler[CABS_NUM];
    /// Триггер тумблера "Фонарь правый буферный"
    TriggerControl P_bufferlight_R_tumbler[CABS_NUM];
    /// Триггер тумблера в резерве
    TriggerControl P_reserv2_tumbler[CABS_NUM];
    /// Триггер тумблера "Проверка АЛСН"
    TriggerControl P_ALSN_check_tumbler[CABS_NUM];

    /// Триггер переключателя "Левый буферный белый/красный"
    TriggerControl P_buffercolor_L_toogle[CABS_NUM];
    /// Триггер переключателя "Правый буферный белый/красный"
    TriggerControl P_buffercolor_R_toogle[CABS_NUM];

    enum
    {
        NUM_RB = 3,
        RBS = 0,
        RB_1 = 1,
        RBP = 2
    };

    /// Триггеры рукояток бдительности
    TriggerControl rb[CABS_NUM][NUM_RB];

    /// Ключ ЭПК
    TriggerControl key_epk[CABS_NUM];

    /// Тумблер включения ЭПТ
    TriggerControl epb_switch[CABS_NUM];

    enum
    {
        NUM_PANTOGRAPHS = 2,
        WIRE_VOLTAGE = 25000
    };

    /// Токоприемники
    std::array<Pantograph *, NUM_PANTOGRAPHS>   pantographs;

    /// Главный выключатель (ГВ)
    ProtectiveDevice    *main_switch = nullptr;

    /// Механизм киловольтметра КС
    Oscillator      *gauge_KV_ks = nullptr;

    /// Тяговый трансформатор
    TracTransformer *trac_trans = nullptr;

    /// Асинхронный расщепитель фаз
    PhaseSplitter   *phase_spliter = nullptr;

    /// Мотор-вентиляторы
    std::array<ACMotorFan *, NUM_MOTOR_FANS> motor_fans;

    /// Мотор-компрессор
    ACMotorCompressor *motor_compressor = nullptr;

    /// Регулятор давления в ГР
    PressureRegulator *press_reg = nullptr;

    /// Главный резервуар
    Reservoir   *main_reservoir = nullptr;

    /// Концевой кран питательной магистрали спереди
    PneumoAngleCock *anglecock_fl_fwd = nullptr;

    /// Концевой кран питательной магистрали сзади
    PneumoAngleCock *anglecock_fl_bwd = nullptr;

    /// Рукав питательной  магистрали спереди
    PneumoHose      *hose_fl_fwd = nullptr;

    /// Рукав питательной  магистрали сзади
    PneumoHose      *hose_fl_bwd = nullptr;

    /// Блокировочное устройство УБТ усл.№367м
    BrakeLock   *brake_lock[CABS_NUM] = {nullptr, nullptr};

    /// Поездной кран машиниста усл.№395
    BrakeCrane  *brake_crane[CABS_NUM] = {nullptr, nullptr};

    /// Кран впомогательного тормоза усл.№254
    LocoCrane   *loco_crane[CABS_NUM] = {nullptr, nullptr};

    /// Тормозная магистраль
    Reservoir   *brakepipe = nullptr;

    /// Воздухораспределитель
    AirDistributor  *air_dist = nullptr;

    /// Электровоздухораспределитель
    ElectroAirDistributor  *electro_air_dist = nullptr;

    /// Запасный резервуар
    Reservoir   *supply_reservoir = nullptr;

    /// Концевой кран тормозной магистрали спереди
    PneumoAngleCock *anglecock_bp_fwd = nullptr;

    /// Концевой кран тормозной магистрали сзади
    PneumoAngleCock *anglecock_bp_bwd = nullptr;

    /// Рукав тормозной магистрали спереди
    PneumoHoseEPB   *hose_bp_fwd = nullptr;

    /// Рукав тормозной магистрали сзади
    PneumoHoseEPB   *hose_bp_bwd = nullptr;

    /// Переключательный клапан магистрали тормозных цилиндров ЗПК
    SwitchingValve  *bc_switch_valve = nullptr;

    /// Повторительное реле давления усл.№304
    PneumoRelay     *bc_pressure_relay = nullptr;

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
    PneumoAngleCock  *anglecock_bc_fwd = nullptr;

    /// Концевой кран магистрали тормозных цилиндров сзади
    PneumoAngleCock  *anglecock_bc_bwd = nullptr;

    /// Рукав магистрали тормозных цилиндров спереди
    PneumoHose  *hose_bc_fwd = nullptr;

    /// Рукав магистрали тормозных цилиндров сзади
    PneumoHose  *hose_bc_bwd = nullptr;

    /// Источник питания ЭПТ
    EPBConverter    *epb_converter = nullptr;

    /// Блок управления двухпроводного ЭПТ
    EPBControl  *epb_control = nullptr;

    /// Контроллер машиниста
    ControllerKME_60_044    *controller[CABS_NUM] = {nullptr, nullptr};

    /// Главный контроллер (переключение обмоток тягового трансформатора)
    EKG_8G                  *main_controller = nullptr;

    enum
    {
        NUM_VU = 2,
        VU1 = 0,
        VU2 = 1
    };

    /// Выпрямительные установки
    std::array<Rectifier *, NUM_VU> vu;

    /// Механизм киловольтметра ТЭД
    Oscillator  *gauge_KV_motors = nullptr;

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

    /// Ограничения скорости на путевой инфраструктуре для кабины А
    SpeedMap    *speedmap_fwd = nullptr;
    /// Ограничения скорости на путевой инфраструктуре для кабины Б
    SpeedMap    *speedmap_bwd = nullptr;

    /// Приёмная катушка АЛСН для кабины А
    CoilALSN    *coil_ALSN_fwd = nullptr;
    /// Приёмная катушка АЛСН для кабины Б
    CoilALSN    *coil_ALSN_bwd = nullptr;

    /// Локомотивный скоростемер
    SL2M    *speed_meter[CABS_NUM] = {nullptr, nullptr};

    /// Свисток и тифон
    TrainHorn   *horn[CABS_NUM] = {nullptr, nullptr};

    /// Система подачи песка
    SandingSystem   *sand_system = nullptr;

    std::vector<Trigger *> triggers;
    Timer   *autoStartTimer = nullptr;
    size_t  start_count = 0;
    size_t  autostart_cab = 0;

    /// Устройство безопасности УКБМ
    SafetyDevice *safety_device[CABS_NUM] = {nullptr, nullptr};

    /// Электропневматический клапан автостопа
    AutoTrainStop *epk[CABS_NUM] = {nullptr, nullptr};

    enum
    {
        NUM_LC_CONTACTS = 3,
        LC_SELF = 0,
        LC_TED = 1,
        LC_TED_LAMP = 2
    };

    /// Линейные контакторы тяговых двигателей
    std::array<Relay *, NUM_MOTORS> linear_contactor;

    DecoderALSN *alsn_decoder[CABS_NUM] = {nullptr, nullptr};

    SpotLight *spotlight[CABS_NUM] = {Q_NULLPTR, Q_NULLPTR};

    /// Общая инициализация локомотива
    void initialization();

    /// Инициализация управления тумблерами
    void initTumblers(const QString &modules_dir, const QString &custom_cfg_dir);

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

    /// Инициализация ЭПТ
    void initEPB(const QString &modules_dir, const QString &custom_cfg_dir);

    /// Инициализация схемы управления тягой
    void initTractionControl(const QString &modules_dir, const QString &custom_cfg_dir);

    /// Инициализация приборов безопасности
    void initSafetyDevices(const QString &modules_dir, const QString &custom_cfg_dir);

    void initOtherEquipment(const QString &modules_dir, const QString &custom_cfg_dir);

    bool initAutostartProgram(int cab_autostart_request);

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

    /// Моделирование ЭПТ
    void stepEPB(double t, double dt);

    void stepTractionControl(double t, double dt);

    void stepLineContactors(double t, double dt);

    void stepOtherEquipment(double t, double dt);

    void stepSoundSignalsOutput(double t, double dt);

    void lineContactorsControl(bool state);

    float isLineContactorsOff();

    void stepSignalsOutput(double t, double dt);

    /// Моделирование приборов безопасности
    void stepSafetyDevices(double t, double dt);

    double getTractionForce();

    bool getHoldingCoilState() const;

    /// Обработка нажатий клавиш
    void keyProcess();

    void debugPrint();

    void load_brakes_config(QString path);

    void loadConfig(QString cfg_path);

private slots:

    void slotAutoStart();
};

#endif // VL60PK_H

