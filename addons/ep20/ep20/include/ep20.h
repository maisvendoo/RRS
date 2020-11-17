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
#include    "ac-motor-compressor.h"
#include    "ep20-signals.h"
#include    "ep20-brake-mech.h"
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

    void initBrakeDevices(double p0, double pTM, double pFL);

private:

//    /// Выбор кабины
//    int selectedCab;

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

    /// Резервуар
    Reservoir   *main_reservoir;

    /// Запасный резервуар (ЗР)
    Reservoir   *spareReservoir;

    /// Мотор компрессор   
    std::array<ACMotorCompressor *, 2> motorCompAC;

    /// Входные значения
    mpcs_input_t mpcsInput;

    /// Выходные значения
    mpcs_output_t mpcsOutput;

    /// Массив токоприемников
    std::array<Pantograph *, NUM_PANTOGRAPHS> pantograph;

    /// Зарядное давление
    double charge_press;

    /// Поездной кран машиниста (КрМ)
    BrakeCrane *krm;

    /// Кран вспомогательного тормоза (КВТ)
    LocoCrane *kvt;

    /// Переключательный клапан (ЗПК)
    SwitchingValve *zpk;

    /// Воздухораспределитель (ВР)
    AirDistributor  *airDistr;

    /// Электро-воздухораспределитель (ЭВР)
    ElectroAirDistributor   *electroAirDistr;

    enum
    {
        NUM_TROLLEYS = 3,
        FWD_TROLLEY = 0,
        MDL_TROLLEY = 1,
        BWD_TROLLEY = 2
    };

    std::array<EP20BrakeMech *, NUM_TROLLEYS> brake_mech;
    std::array<PneumoReley *, NUM_TROLLEYS> rd304;
    std::array<PneumoSplitter *, 2> pSplit;

    /// Бесконтактный контроллер машиниста
    KMB2    *kmb2;

    /// Инициализация
    void initialization();

    /// Инициализация высоковольтной схемы
    void initHighVoltageScheme();

    /// Инициализация тормозного крана
    void initBrakeControls(QString modules_dir);

    /// Инициализация МПСУ
    void initMPCS();

    /// Инициализация КМБ2
    void initKMB2();

    /// Шаг моделирования всех систем локомотива в целом
    void step(double t, double dt);

    /// Шаг моделирования МПСУ
    void stepMPCS(double t, double dt);

    /// Шаг моделирования высоковольтной схемы
    void stepHighVoltageScheme(double t, double dt);

    /// Шаг моделирования тормозного крана
    void stepBrakeControls(double t, double dt);

    /// Шаг моделирования бесконтактного контроллера машиниста
    void stepKMB2(double t, double dt);

    /// Загрузка данных из конфигурационных файлов
    void loadConfig(QString cfg_path);

    /// Обработчик клавиш
    void keyProcess();

    void load_brakes_config(QString path);

    void stepSignals();
};

#endif // EP20_H

