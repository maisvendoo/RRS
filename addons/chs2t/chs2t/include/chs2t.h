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
#include    "pantograph.h"
#include    "protective-device.h"
#include    "km-21kr2.h"
#include    "stepswitch.h"
#include    "pusk-rez.h"
#include    "motor.h"
#include    "registrator.h"
#include    "overload-relay.h"
#include    "dc-motor-compressor.h"
#include    "pressure-regulator.h"
#include    "chs2t-brake-mech.h"
#include    "dako.h"
#include    "generator.h"
#include    "pulse-converter.h"
#include    "brake-regulator.h"
#include    "handle-edt.h"
#include    "dc-motor-fan.h"
#include    "release-valve.h"
#include    "blinds.h"
#include    "hardware-signals.h"
#include    "ept-converter.h"
#include    "ept-pass-control.h"
#include    "convert-physics-to-modbus.h"

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

private:

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

    /// Реле перегрузки ТЭД
    OverloadRelay *overload_relay;

    /// Главный резервуар (ГР)
    Reservoir *mainReservoir;

    /// Запасный резервуар (ЗР)
    Reservoir *spareReservoir;

    /// Задатчик тормозного усилия ЭДТ (ЗТ)
    Reservoir *brakeRefRes;

    PhysToModbus *TM_manometer;
    PhysToModbus *UR_manometer;
    PhysToModbus *ZT_manometer;
    PhysToModbus *GR_manometer;
    PhysToModbus *TC_manometer;

    /// Регулятор давления ГР
    PressureRegulator *pressReg;

    /// Мотор-компрессоры (МК)
    std::array<DCMotorCompressor *, 2> motor_compressor;

    /// Тумблер включенияМК
    Trigger     mk_tumbler;

    /// Галетники управления МК
    std::array<Switcher *, 2> mk_switcher;

    /// Поездной кран машиниста (КрМ)
    BrakeCrane *brakeCrane;

    ElectroAirDistributor *electroAirDistr;

    /// Тормозная рычажная передача тележек
    std::array<CHS2tBrakeMech *, 2>    brakesMech;

    /// Скоростной клапан ДАКО
    Dako *dako;

    /// Воздухораспределитель (ВР)
    AirDistributor *airDistr;

    /// Кран вспомогательного тормоза (КВТ)
    LocoCrane *locoCrane;

    /// Переключательный клапан (ЗПК)
    SwitchingValve *zpk;

    /// Реле давления РД304 (РД)
    PneumoReley *rd304;

    /// Разветвитель потока воздуха от ДАКО к тележке 1 и РД304
    PneumoSplitter *pnSplit;

    /// Разветвитель от ВР к ДАКО и ЗТ
    PneumoSplitter *airSplit;

    ReleaseValve *relValve;

    DCMotorFan *motor_fan_ptr;

    /// Свисток и тифон
    TrainHorn   *horn;

    /// Схема реостатного тормоза
    Generator   *generator;

    /// Импульсный преобразователь возбуждения
    PulseConverter  *pulseConv;

    /// Регулятор тормозного усилия (САРТ)
    BrakeRegulator  *BrakeReg;

    AutoTrainStop *autoTrainStop;

    /// Рукоятка задатчика тормозного усилия
    HandleEDT       *handleEDT;

    /// Галетники управления токоприемниками
    std::array<Switcher *, NUM_PANTOGRAPHS> pantoSwitcher;

    /// Галетник управления БВ
    Switcher    *fastSwitchSw;

    std::array<DCMotorFan*, 2> motor_fan;
    Switcher *motor_fan_switcher;

    Switcher *blindsSwitcher;

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

    /// Выключатель ЭПТ
    Trigger     eptSwitch;

    /// Преобразователь питания ЭПТ
    EPTConverter *ept_converter;

    /// Блок управления ЭПТ
    EPTPassControl *ept_pass_control;

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

    /// Инициадизация тормозных приборов
    void initBrakeDevices(double p0, double pTM, double pFL);

    /// Загрузка данных из конфигурационных файлов
    void loadConfig(QString cfg_path);

    /// Обработка клавиш
    void keyProcess();

    /// Вывод данных на внешние СОИ
    void hardwareOutput();

    /// Сброс данных в регистратор
    void registrate(double t, double dt);

    /// Состояние удерживающей катошки БВ
    bool getHoldingCoilState() const;

    /// Инициализация токоприемников
    void initPantographs();

    /// Инициадизация рычажки
    void initBrakesMech();

    /// Инициализация БВ
    void initFastSwitch();

    /// Инициализация приборов управления тормозами
    void initBrakesControl(QString module_path);

    /// Инициализация защит
    void initProtection();

    /// Инициализация подсистемы снабжения электровоза сжатым воздухом
    void initAirSupplySubsystem();

    /// Инициализация схемы управления тягой
    void initTractionControl();

    /// Инициализация тормозного оборудования
    void initBrakesEquipment(QString module_path);

    /// Инициализация ЭДТ
    void initEDT();

    /// Инициализация вспомогательного оборудования
    void initSupportEquipment();

    /// Инициализация прочего оборудования
    void initOtherEquipment();

    /// Инициализация приборов ЭПТ
    void initEPT();

    ///
    void initModbus();

    /// Инициализация регистратора
    void initRegistrator();

    /// Общая инициализация локомотива
    void initialization();

    /// Моделирование работы токоприемников
    void stepPantographs(double t, double dt);

    /// Моделирование работы рычажки
    void stepBrakesMech(double t, double dt);

    void stepFastSwitch(double t, double dt);

    void stepProtection(double t, double dt);

    void stepTractionControl(double t, double dt);

    void stepAirSupplySubsystem(double t, double dt);

    void stepBrakesControl(double t, double dt);

    void stepBrakesEquipment(double t, double dt);

    void stepEDT(double t, double dt);

    void stepEDT2(double t, double dt);

    void stepSupportEquipment(double t, double dt);

    void stepEPT(double t, double dt);

    void stepDebugMsg(double t, double dt);

    void stepSignals();

    /// Шаг моделирования всех систем локомотива в целом
    void step(double t, double dt);

    void disableEDT() { EDT = allowEDT = false; }

private slots:

    void enableEDT()
    {
        EDT = allowEDT = true;
        EDT_timer.stop();
    }
};

#endif // CHS2TOO
