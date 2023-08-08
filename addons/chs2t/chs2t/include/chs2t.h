//------------------------------------------------------------------------------
//
//      Магистральный пассажирский электровоз постоянного тока ЧС2т.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Николай Авилкин (avilkin.nick)
//
//      Дата: 21/08/2019
//
//------------------------------------------------------------------------------
#ifndef     CHS2T_H
#define     CHS2T_H

#include    "vehicle-api.h"

#include    "km-21kr2.h"
#include    "stepswitch.h"
#include    "pusk-rez.h"
#include    "motor.h"
#include    "registrator.h"
#include    "overload-relay.h"
#include    "electropneumovalve-emergency.h"
#include    "electropneumovalve-release.h"
#include    "dako.h"
#include    "generator.h"
#include    "pulse-converter.h"
#include    "brake-regulator.h"
#include    "handle-edt.h"
#include    "dc-motor-fan.h"
#include    "blinds.h"
#include    "hardware-signals.h"
#include    "convert-physics-to-modbus.h"
#include    "sl2m.h"
#include    "energy-counter.h"
#include    "chs2t-switcher.h"
#include    "alsn-ukbm.h"

/*!
 * \class
 * \brief Основной класс, описывающий весь электровоз
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class CHS2T : public Vehicle
{
public:

    /// Конструктор
    CHS2T();

    /// Деструктор
    ~CHS2T();

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

    enum
    {
        NUM_PANTOGRAPHS = 2,
        WIRE_VOLTAGE = 3000
    };

    /// Напряжение аккумуляторной батареи
    double U_bat;

    /// Схема тяги
    Motor *motor;

    /// Токоприемники
    std::array<Pantograph *, NUM_PANTOGRAPHS>    pantographs;

    /// Быстрый выключатель
    ProtectiveDevice *bv;

    /// Пусковой резистор
    PuskRez *puskRez;

    /// Регистратор
    Registrator *reg;

    /// Контроллер машиниста
    Km21KR2 *km21KR2;

    /// Переключатель ступеней
    StepSwitch *stepSwitch;

    double tracForce_kN;

    /// Возврат защиты
    bool bv_return;

    /// Список звуков перестука
    QList<QString> tap_sounds;

    /// Реле перегрузки ТЭД
    OverloadRelay *overload_relay;

    PhysToModbus *TM_manometer;
    PhysToModbus *UR_manometer;
    PhysToModbus *ZT_manometer;
    PhysToModbus *GR_manometer;
    PhysToModbus *TC_manometer;

    /// Тумблер включенияМК
    Trigger     mk_tumbler;

    /// Галетники управления МК
    std::array<CHS2TSwitcher *, 2> mk_switcher;

    /// Мотор-компрессоры (МК)
    std::array<DCMotorCompressor *, 2> motor_compressor;

    /// Регулятор давления ГР
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

    /// Поездной кран машиниста усл.№395
    BrakeCrane  *brake_crane;

    /// Кран впомогательного тормоза усл.№254
    LocoCrane   *loco_crane;

    /// Рукоятка задатчика тормозного усилия
    HandleEDT   *handleEDT;

    /// Электропневматический клапан автостопа усл.№150
    AutoTrainStop   *epk;

    /// Электропневматический вентиль экстренного торможения (при ТМ < 0.3 МПа)
    ElectroPneumoValveEmergency *emergency_valve;

    /// Электропневматический вентиль отпуска пневматических тормозов
    ElectroPneumoValveRelease   *release_valve;

    /// Управляющая камера воздухораспределителя (ложный ТЦ)
    Reservoir   *brake_ref_res;

    /// Тормозная магистраль
    Reservoir   *brakepipe;

    /// Воздухораспределитель
    AirDistributor  *air_dist;

    /// Электровоздухораспределитель
    ElectroAirDistributor  *electro_air_dist;

    /// Запасный резервуар
    Reservoir   *supply_reservoir;

    /// Разветвитель потока воздуха от локомотивного крана к тележкам
    PneumoSplitter  *loco_crane_splitter;

    /// Скоростной клапан ДАКО
    Dako *dako;

    /// Повторительное реле давления усл.№304
    PneumoRelay     *bc_pressure_relay;

    /// Концевой кран тормозной магистрали спереди
    PneumoAngleCock *anglecock_bp_fwd;

    /// Концевой кран тормозной магистрали сзади
    PneumoAngleCock *anglecock_bp_bwd;

    /// Рукав тормозной магистрали спереди
    PneumoHoseEPB   *hose_bp_fwd;

    /// Рукав тормозной магистрали сзади
    PneumoHoseEPB   *hose_bp_bwd;

    enum
    {
        NUM_TROLLEYS = 2,
        NUM_AXIS_PER_TROLLEY = 3,
        TROLLEY_FWD = 0,
        TROLLEY_BWD = 1
    };

    /// Переключательные клапаны ЗПК потока в тормозные цилиндры
    std::array<SwitchingValve *, NUM_TROLLEYS> bc_switch_valve;

    /// Тормозные механизмы тележек
    std::array<BrakeMech *, NUM_TROLLEYS> brake_mech;

    /// Выключатель ЭПТ
    Trigger     epb_switch;

    /// Преобразователь питания ЭПТ
    EPBConverter *epb_converter;

    /// Блок управления ЭПТ
    EPBControl *epb_control;

    DCMotorFan *motor_fan_ptr;

    /// Свисток и тифон
    TrainHorn   *horn;

    /// Схема реостатного тормоза
    Generator   *generator;

    /// Импульсный преобразователь возбуждения
    PulseConverter  *pulseConv;

    /// Регулятор тормозного усилия (САРТ)
    BrakeRegulator  *BrakeReg;

    /// Галетники управления токоприемниками
    std::array<CHS2TSwitcher *, NUM_PANTOGRAPHS> pantoSwitcher;

    /// Галетник управления БВ
    CHS2TSwitcher    *fastSwitchSw;

    std::array<DCMotorFan*, 2> motor_fan;

    CHS2TSwitcher *motor_fan_switcher;

    CHS2TSwitcher *blindsSwitcher;

    /// Зарядное давление
    double charging_press;

    /// Напряжение на крышевой шине токоприемников
    double U_kr;

    /// Разъединители токоприемников
    std::array<Trigger, NUM_PANTOGRAPHS> pant_switch;

    /// Тригеры поднятия/опускания ТП
    std::array<Trigger, NUM_PANTOGRAPHS> pantup_trigger;

    /// Тригер включения БВ
    Trigger     fast_switch_trigger;

    /// Выключатель ЭДТ
    Trigger     EDTSwitch;

    /// Разрешение тяги
    Trigger     allowTrac;

    bool dropPosition;

    Timer EDT_timer;

    /// Передаточное число тягового редуктора
    double      ip;

    /// Флаг сбора схемы ЭДТ
    bool        EDT;

    /// Флаг разрешения работы ЭДТ
    bool        allowEDT;

    /// Жалюзи пуско-тормозных резисторов
    Blinds      *blinds;

    /// Скоростемер 3СЛ2М
    SL2M        *speed_meter;

    /// Счетчик энергии
    EnergyCounter   *energy_counter;

    /// Устройство безопасности
    SafetyDevice    *safety_device;

    /// Состояния РБ и РБС
    bool state_RB;

    bool state_RBS;

    /// Загрузка данных из конфигурационных файлов
    void loadConfig(QString cfg_path);

    /// Обработка клавиш
    void keyProcess();

    /// Вывод данных на внешние СОИ
    void hardwareOutput();

    /// Сброс данных в регистратор
    void registrate(double t, double dt);

    /// Состояние удерживающей катушки БВ
    bool getHoldingCoilState() const;

    /// Общая инициализация локомотива
    void initialization();

    /// Инициализация токоприемников
    void initPantographs();

    /// Инициадизация рычажки
    void initBrakesMech();

    /// Инициализация БВ
    void initFastSwitch();

    /// Инициализация защит
    void initProtection();

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

    /// Инициализация ЭДТ
    void initEDT();

    /// Инициализация вспомогательного оборудования
    void initSupportEquipment();

    /// Инициализация прочего оборудования
    void initOtherEquipment();

    ///
    void initModbus();

    /// Инициализация списка звуков перестука
    void initTapSounds();

    /// Инициализация регистратора
    void initRegistrator();

    /// Подпрограмма изменения положения пакетника
    void setSwitcherState(Switcher *sw, signal_t signal);

    /// Шаг моделирования всех систем локомотива в целом
    void step(double t, double dt);

    /// Моделирование работы токоприемников
    void stepPantographs(double t, double dt);

    void stepFastSwitch(double t, double dt);

    void stepProtection(double t, double dt);

    /// Моделирование питательной магистрали
    void stepPneumoSupply(double t, double dt);

    /// Моделирование приборов управления тормозами
    void stepBrakesControl(double t, double dt);

    /// Моделирование тормозного оборудования
    void stepBrakesEquipment(double t, double dt);

    /// Моделирование ЭПТ
    void stepEPB(double t, double dt);

    void stepTractionControl(double t, double dt);

    void stepEDT(double t, double dt);

    void stepEDT2(double t, double dt);

    void stepSupportEquipment(double t, double dt);

    void stepDebugMsg(double t, double dt);

    void stepSignals();

    void stepSwitcherPanel();

    void stepTapSound();

    void stepDecodeAlsn();

    void disableEDT() { EDT = allowEDT = false; }

private slots:

    void enableEDT()
    {
        EDT = allowEDT = true;
        EDT_timer.stop();
    }
};

#endif // CHS2T_H
