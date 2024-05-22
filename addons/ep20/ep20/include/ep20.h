//------------------------------------------------------------------------------
//
//      Двухсистемный пассажирский электровоз с асинхронным тяговым приводом ЭП20.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Балашов Евгений (MrJecson)
//
//      Дата: 1/08/2019
//
//------------------------------------------------------------------------------
#ifndef     EP20_H
#define     EP20_H

#include    "vehicle-api.h"
#include    "pantograph.h"
#include    "mpcs.h"
#include    "pant-description.h"
#include    "current-kind-switch.h"
#include    "protective-device.h"
#include    "traction-transformer.h"
#include    "traction-converter.h"
#include    "auxiliary-converter.h"
#include    "ep20-signals.h"
#include    "kmb2.h"

/*!
 * \class
 * \brief Основной класс, описывающий весь тепловоз
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class EP20 : public Vehicle
{
public:

    /// Конструктор
    EP20();

    /// Деструктор
    ~EP20();

    void initBrakeDevices(double p0, double pBP, double pFL);

private:

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
    /// Имя модуля электровоздухораспределителя
    QString electro_airdist_module_name;
    /// Имя конфига электровоздухорапределителя
    QString electro_airdist_config_name;

//    /// Выбор кабины
//    int selectedCab;

    /// Сцепка спереди
    Coupling *coupling_fwd;
    /// Сцепка сзади
    Coupling *coupling_bwd;

    /// Расцепной рычаг спереди
    OperatingRod *oper_rod_fwd;
    /// Расцепной рычаг сзади
    OperatingRod *oper_rod_bwd;

    /// Микропроцессорная система управления электровозом
    MPCS    *mpcs;

    /// Перекрлючатель рода тока
    CurrentKindSwitch   *kindSwitch;

    /// Аппарат защиты (ГВ)
    ProtectiveDevice    *mainSwitch;

    /// Аппарат защиты (БВ)
    ProtectiveDevice    *fastSwitch;

    /// Тяговый трансформатор
    TractionTransformer *tractionTrans;

    /// Нумератор тяговых преобразователей
    enum
    {
        NUM_TRAC_CONV = 3
    };

    /// Массив тяговых преобразователей
    std::array<TractionConverter *, NUM_TRAC_CONV> trac_conv;

    enum
    {
        NUM_AUX_CONV = 4
    };

    /// Преобразователь собственных нужд
    std::array<AuxiliaryConverter *, NUM_AUX_CONV> auxConv;

    /// Входные значения
    mpcs_input_t mpcsInput;

    /// Выходные значения
    mpcs_output_t mpcsOutput;

    /// Массив токоприемников
    std::array<Pantograph *, NUM_PANTOGRAPHS> pantograph;

    /// Мотор-компрессор
    std::array<ACMotorCompressor *, 2> motor_compressor;

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

    /// Зарядное давление
    double charge_press;

    /// Поездной кран машиниста усл.№130
    BrakeCrane          *brake_crane;

    /// Кран впомогательного тормоза усл.№224
    LocoCrane           *loco_crane;

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

    /// Тройники для распределения воздуха от переключательного клапана
    /// к тележкам
    std::array<PneumoSplitter *, 2> bc_splitter;

    enum
    {
        NUM_TROLLEYS = 3,
        NUM_AXIS_PER_TROLLEY = 2,
        TROLLEY_FWD = 0,
        TROLLEY_MID = 1,
        TROLLEY_BWD = 2
    };

    /// Повторительное реле давления усл.№304
    std::array<PneumoRelay *, NUM_TROLLEYS> bc_pressure_relay;

    /// Тормозные механизмы тележек
    std::array<BrakeMech *, NUM_TROLLEYS> brake_mech;

    /// Напряжение аккумуляторной батареи
    double U_bat;

    /// Источник питания ЭПТ
    EPBConverter        *epb_converter;

    /// Блок управления двухпроводного ЭПТ
    EPBControl          *epb_control;

    /// Бесконтактный контроллер машиниста
    KMB2    *kmb2;

    /// Инициализация
    void initialization();

    /// Инициализация сцепных устройств
    void initCouplings(QString modules_dir);

    /// Инициализация высоковольтной схемы
    void initHighVoltageScheme();

    /// Инициализация питательной магистрали
    void initPneumoSupply(QString modules_dir);

    /// Инициализация приборов управления тормозами
    void initBrakesControl(QString modules_dir);

    /// Инициализация тормозного оборудования
    void initBrakesEquipment(QString modules_dir);

    /// Инициализация ЭПТ
    void initEPB(QString modules_dir);

    /// Инициализация МПСУ
    void initMPCS();

    /// Инициализация КМБ2
    void initKMB2();

    /// Шаг моделирования всех систем локомотива в целом
    void step(double t, double dt);

    /// Моделирование сцепных устройств
    void stepCouplings(double t, double dt);

    /// Шаг моделирования МПСУ
    void stepMPCS(double t, double dt);

    /// Шаг моделирования высоковольтной схемы
    void stepHighVoltageScheme(double t, double dt);

    /// Шаг моделирования питательной магистрали
    void stepPneumoSupply(double t, double dt);

    /// Шаг моделирования приборов управления тормозами
    void stepBrakesControl(double t, double dt);

    /// Шаг моделирования тормозного оборудования
    void stepBrakesEquipment(double t, double dt);

    /// Шаг моделирования ЭПТ
    void stepEPB(double t, double dt);

    /// Шаг моделирования бесконтактного контроллера машиниста
    void stepKMB2(double t, double dt);

    /// Вывод отладочной строки
    void debugOutput(double t, double dt);

    /// Загрузка данных из конфигурационных файлов
    void loadConfig(QString cfg_path);

    /// Обработчик клавиш
    void keyProcess();

    void load_brakes_config(QString path);

    void stepSignals();
};

#endif // EP20_H

