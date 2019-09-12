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
#include    "engine.h"
#include    "registrator.h"
#include    "overload-relay.h"
#include    "dc-motor-compressor.h"
#include    "pressure-regulator.h"
#include    "chs2t-brake-mech.h"
#include    "dako.h"

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

    /// Тяговый электродвигатель
    Engine *motor;

    /// Токоприемники
    std::array<Pantograph *, NUM_PANTOGRAPHS>    pantographs;

    /// Быстрый выключатель
    ProtectiveDevice *bistV;

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

    Reservoir *mainReservoir;

    Reservoir *spareReservoir;

    PressureRegulator *pressReg;

    DCMotorCompressor *motor_compressor;

    Trigger     mk_tumbler;

    BrakeCrane *brakeCrane;

    std::array<CHS2tBrakeMech *, 2>    brakesMech;

    Dako *dako;

    AirDistributor *airDistr;

    LocoCrane *locoCrane;

    SwitchingValve *zpk;



    double charging_press;



    void initBrakeDevices(double p0, double pTM, double pFL);

    /// Загрузка данных из конфигурационных файлов
    void loadConfig(QString cfg_path);

    void keyProcess();

    void registrate(double t, double dt);

    bool getHoldingCoilState() const;



    void initPantographs();

    void initBrakesMech();

    void initFastSwitch();

    void initBrakesControl(QString module_path);

    void initProtection();

    void initAirSupplySubsystem();

    /// Инициализация схемы управления тягой
    void initTractionControl();

    void initBrakesEquipment(QString module_path);

    void initRegistrator();


    /// Общая инициализация локомотива
    void initialization();



    void stepPantographs(double t, double dt);

    void stepBrakesMech(double t, double dt);

    void stepFastSwitch(double t, double dt);

    void stepProtection(double t, double dt);

    void stepTractionControl(double t, double dt);

    void stepAirSupplySubsystem(double t, double dt);

    void stepBrakesControl(double t, double dt);

    void stepBrakesEquipment(double t, double dt);

    void stepDebugMsg(double t, double dt);

    /// Шаг моделирования всех систем локомотива в целом
    void step(double t, double dt);

};

#endif // CHS2T
