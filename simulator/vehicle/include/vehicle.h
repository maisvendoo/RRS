//------------------------------------------------------------------------------
//
//      Vehicle base class
//      (c) maisvendoo, 03/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief  Vehicle base class
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 03/09/2018
 */

#ifndef     VEHICLE_H
#define     VEHICLE_H

#include    <QObject>
#include    <QtGlobal>
#include    <mutex>
#include    <functional>

#include    "datetime.h"
#include    "control-signals.h"
#include    "feedback-signals.h"

#include    "profile-point.h"
#include    "vehicle-collision.h"
#include    "vehicle-dynamics.h"
#include    "vehicle-lateral-dynamics.h"
#include    "vehicle-derailment.h"
#include    "vehicle-damage.h"
#include    "vehicle-hazard.h"
#include    "vehicle-flat.h"
#include    "vehicle-adhesion.h"
#include    "vehicle-sand.h"
#include    "vehicle-brake-shoes.h"
#include    "vehicle-energy.h"
#include    "vehicle-pantograph.h"
#include    "vehicle-depot-power.h"
#include    "vehicle-coupling-interaction.h"
#include    "vehicle-sound-events.h"
#include    "vehicle-windshield.h"
#include    "vehicle-camera-motion.h"
#include    "vehicle-cab-interaction.h"
#include    "vehicle-cargo.h"
#include    "vehicle-passengers.h"
#include    "vehicle-service.h"
#include    "vehicle-condensate.h"
#include    "vehicle-wheel-wear.h"
#include    "vehicle-tunnel.h"
#include    "vehicle-wsp.h"

#include    <catenary-system.h>

#include    <simulation-lod.h>

#include    "device-list.h"

#include    "physics.h"
#include    "solver-types.h"

#include    <autopilot.h>

namespace collision
{
    class CollisionWorld;
    struct CollisionEvent;
}

#if defined(VEHICLE_LIB)
    #define VEHICLE_EXPORT  Q_DECL_EXPORT
#else
    #define VEHICLE_EXPORT  Q_DECL_IMPORT
#endif

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class VEHICLE_EXPORT Vehicle : public QObject
{
    Q_OBJECT

public:

    /// Constructor
    explicit Vehicle(QObject* parent = nullptr);
    /// Destructor
    virtual ~Vehicle();

    /// Vehicle initialization
    void init(QString cfg_path);

    /// Set vehicle module directory
    void setModuleDir(QString module_dir);
    /// Set Vehicle module name
    void setModuleName(QString module_name);
    /// Set vehicle configuration file directory
    void setConfigDir(QString config_dir);
    /// Set Vehicle configuration file name
    void setConfigName(QString config_name);
    /// Set current route directory
    void setRouteDir(QString route_dir);

    /// Set vehicle index
    void setModelIndex(size_t idx);
    /// Set train index
    void setTrainIndex(size_t idx);
    /// Set vehicle state index
    void setStateIndex(size_t idx);

    /// Set inclination
    void setProfilePoint(profile_point_t point_data);

    /// Set friction coefficient between wheel and rail
    void setFrictionCoeff(double value);

    /// Set direction relative to train
    void setDirection(std::int8_t dir);

    /// Set forward coupling force
    void addForwardForce(double value);

    /// Set backward coupling force
    void addBackwardForce(double value);

    /// Set active common force
    void setActiveCommonForce(size_t idx, double value);

    /// Set reactive common force
    void setReactiveCommonForce(size_t idx, double value);

    /// Set payload level
    void setPayloadCoeff(double payload_coeff);

    void setTrainCoord(double value);

    /// Создать коллайдеры ПЕ в мире коллизий (вызывается моделью
    /// после расстановки всех ПЕ на топологии)
    void createCollisionBodies(collision::CollisionWorld* world);

    /// Синхронизировать коллайдеры с текущим положением на траектории
    void syncCollisionPose();

    /// Реакция на контакт коллайдера ПЕ с препятствием (вызывается моделью)
    void onCollisionContact(const collision::CollisionEvent& event);

    /// Снять аварийное состояние после прекращения контакта
    void resetCollisionState();

    /// ПЕ в аварийном состоянии после столкновения
    bool isCollided() const;

    /// Повреждение кузова (0.0 - цел, 1.0 - разрушен)
    float getBodyDamage() const;

    /// Повреждение ходовой части (0.0 - цела, 1.0 - разрушена)
    float getBogieDamage() const;

    /// Сброс повреждений (ремонт)
    void resetDamage();

    /// Привязать источник высоты рельса (контроллер ПЕ на топологии).
    /// Вызывается один раз после расстановки ПЕ
    void setRailHeightSource(VehicleVerticalDynamics::RailHeightFn fn);

    /// Источник высоты рельса уже привязан
    bool hasRailHeightSource() const;

    /// Привязать источник боковых неровностей пути (для поперечной динамики)
    void setLateralOffsetSource(VehicleLateralDynamics::LateralOffsetFn fn);

    /// Источник боковых неровностей уже привязан
    bool hasLateralOffsetSource() const;

    /// Привязать источник возвышения наружного рельса (cant, мм; Б16).
    /// Возвращает возвышение по абсолютной координате пути (топология
    /// через контроллер ПЕ). Паттерн - как у rail height
    void setLateralCantSource(std::function<double(double)> fn);

    /// Источник возвышения уже привязан
    bool hasCantSource() const;

    /// Вертикальная динамика ПС (колебания от неровностей пути)
    VehicleVerticalDynamics& getVerticalDynamics();

    /// Поперечная динамика ПС (виляние, крип, критерий схода)
    VehicleLateralDynamics& getLateralDynamics();

    /// Система схода с рельсов (постепенная, по физическим датчикам)
    VehicleDerailment& getDerailment();

    /// ПЕ сошла с рельсов (хотя бы одна ось)
    bool isDerailed() const;

    /// Рывок сцепки от сошедшего соседа: боковая составляющая дёргающего
    /// удара разгружает колёса (цепной сход, ТЗ "Физика после схода", п.21)
    void onCouplerJerk(double energy);

    /// Сброс состояния схода (восстановление после аварии)
    void resetDerailment();

    /// Компонентная система повреждений ПЕ
    VehicleDamageSystem& getDamageSystem();

    /// Система опасного груза (утечка/пожар/взрыв)
    VehicleHazard& getHazard();

    /// Система ползунов колёсных пар (юз -> ползун -> удары при обороте)
    WheelFlatSystem& getFlatSpots();

    /// Система сцепления колёс с рельсами (погода/загрязнение/песок)
    WheelRailAdhesion& getAdhesion();

    /// Система пескоподачи (бункер, форсунки, автоматика)
    SandSystem& getSand();

    /// Система тормозных колодок (нагрев/износ/fade)
    BrakeShoeSystem& getBrakeShoes();

    /// Учёт электроэнергии и статистика рейса
    EnergyMeterSystem& getEnergy();

    /// Токоприёмник (контакт с КС, дуги)
    PantographSystem& getPantograph();

    /// Деповское питание 380 В и аккумуляторная батарея
    DepotPowerSystem& getDepotPower();

    /// Интерактивная сцепка (рукава, краны, рычаг СА-3)
    CouplingInteraction& getCouplingInteraction();

    /// Ветер для токоприёмника (от погоды, ТЗ "Видимость и погода")
    void applyWindToPantograph(double wind_speed);

    /// Собрать физические звуковые события ПЕ с последнего опроса
    /// (мост физика -> аудиосистема, ТЗ "Аудиосистема")
    void collectSoundEvents(std::vector<SoundEvent>& out);

    /// Лобовое стекло кабины (грязь/дворники/омыватель/лёд)
    WindshieldSystem& getWindshield();

    /// Физическая реакция машиниста (камера на физике кузова)
    CameraMotionFromPhysics& getCameraMotion();

    /// Реестр интерактивных элементов кабины (подсказки Alt)
    CabInteractionRegistry& getCabInteraction();

    /// Грузовая система вагона (погрузка/разгрузка с изменением массы)
    CargoSystem& getCargo();

    /// Пассажирская система вагона (посадка/высадка)
    PassengerSystem& getPassengers();

    /// Система снабжения (заправка топливом/маслом/ОЖ/песком на
    /// стоянке через колонки депо/ПТО, ТЗ "Снабжение локомотива")
    ServiceSystem& getService();

    /// Конденсат и лёд в пневматической системе (точка росы, влага,
    /// замерзание, слив; тормозная волна деградирует при льде)
    CondensateSystem& getCondensate();

    /// Износ колёсных пар (пробег/тоннаж/боксование/торможения ->
    /// профиль -> коничность -> виляние, ТЗ "43-47", п.1)
    WheelWearSystem& getWheelWear();

    /// Аэродинамика тоннеля ("воздушный поршень"; зоны тоннелей
    /// маршрута загружает модель через setZones)
    TunnelAerodynamics& getTunnel();

    /// Противоюзная система WSP (модуляция тормозного момента осей)
    WSPSystem& getWSP();

    /// Обточка колёсных пар в депо: сброс ползунов + уменьшение
    /// диаметра от износа (ТЗ "43-47", п.1)
    void reprofileWheels();

    /// Интенсивность осадков (от погоды, для стекла)
    void setRainIntensity(double intensity);

    /// Уровень детализации симуляции ПЕ (ТЗ "Оптимизация", п.2-5):
    /// L0 полный; L1 пропускает дорогие визуальные/тепловые подсистемы;
    /// L2 только продольная модель; L3 заморозка
    void setSimulationLOD(perf::SimLOD lod);
    perf::SimLOD getSimulationLOD() const;

    /// Группа СМЕ (0 - не в СМЕ)
    int getSMEGroup() const;

    /// Текущие управляющие сигналы (для передачи по СМЕ)
    const control_signals_t& getControlSignalsRef() const;

    /// Головной локомотив СМЕ
    bool isSMELead() const;

    /// Источник питания КС: (пикетаж, ток) -> состояние питания
    void setCatenaryFeed(std::function<catenary::FeedState(double, double)> fn);

    /// Сколько сеть готова принять рекуперации от этой ПЕ, Вт
    /// (считается моделью: подстанция + потребители секции)
    void setRegenAcceptance(double accept_w, bool accepted);

    /// Источник питания КС привязан
    bool getCatenaryFeedActive() const;

    /// Ремонт: сброс повреждений и последствий аварии
    void repair();

    /// Высота центра масс над уровнем осей колёсных пар, м
    double getMassCenterHeight() const;

    /// Продольное смещение центра масс от середины ПЕ, м
    /// (положительное - к переду поезда)
    double getMassCenterLongitudinal() const;

    /// Поперечное смещение центра масс, м (положительное - вправо)
    double getMassCenterLateral() const;

    void setVelocity(double value);

    void setWheelAngle(size_t i, double value);
    void setWheelOmega(size_t i, double value);

    void setPrevVehicle(Vehicle* vehicle);
    void setNextVehicle(Vehicle* vehicle);

    void setNeedDebugMsg(bool is_needed);

    /// Get vehicle module directory
    QString getModuleDir() const;
    /// Get Vehicle module name
    QString getModuleName() const;
    /// Get vehicle configuration file directory
    QString getConfigDir() const;
    /// Get Vehicle configuration file name
    QString getConfigName() const;

    /// Get vehicle index
    size_t getModelIndex() const;
    /// Get train index
    size_t getTrainIndex() const;
    /// Get vehicle state index
    size_t getStateIndex() const;

    profile_point_t* getProfilePoint();

    /// Direction relative to train: 1 - co-directional, -1 - reversed
    std::int8_t getDirection() const;

    /// Get vehicle mass
    double getMass() const;

    /// Get vehicle length
    double getLength() const;

    /// Get degrees of freedom
    size_t getDegressOfFreedom() const;

    /// Get number of axis
    size_t getNumAxis() const;

    /// Get wheel diameter
    double getWheelDiameter(size_t i) const;

    double getTrainCoord() const;

    double getVelocity() const;

    double getWheelAngle(size_t i);

    double getWheelOmega(size_t i);

    Vehicle* getPrevVehicle();
    Vehicle* getNextVehicle();

    float getAnalogSignal(size_t i);
    std::vector<float>* getAnalogSignals();

    device_list_t* getFwdConnectors();
    device_list_t* getBwdConnectors();
    device_coord_list_t* getRailwayConnectors();

    /// Init vehicle brake devices
    virtual void initBrakeDevices(double p0, double pTM, double pFL);

    /// Common acceleration calculation
    virtual void getAcceleration(state_vector_t& Y, state_vector_t& dYdt, const double& t, const double& dt);

    void integrationProcess(const simulator_time_t& t, const double& dt);

    void integrationPreStep(state_vector_t& Y, const double& t);

    void integrationStep(state_vector_t& Y, const double& t, const double& dt);

    void integrationPostStep(state_vector_t& Y, const double& t);

    QString getDebugMsg() const;

    void setUks(double value);

    void setCurrentKind(int value);

    void setKeyboardControl(const uint8_t& cab_num, const std::vector<uint16_t>& pressed_keys);

    void resetKeyboardControl(const uint8_t& cab_num);

    void setControlSignals(const control_signals_t& control_signals);

    feedback_signals_t& getFeedBackSignals();

    void setBrakeShoesState(bool state);

    const std::vector<Autopilot *>& getAutopilot() const
    {
        return autopilot;
    }

    virtual void OnAutopilot()
    {
        auto_start_autopilot = true;
    }

    virtual void OffAutopilot()
    {
        auto_start_autopilot = false;
    }

    std::vector<QMap<int, float>> control_inputs;

signals:

    void sigGetTrainParams(int train_idx, double &train_len, double &train_mass);

protected:

    /// Vehicle configuration file directory
    QString module_dir = "";
    /// Vehicle configuration file name
    QString module_name = "";
    /// Vehicle configuration file directory
    QString config_dir = "";
    /// Vehicle configuration file name
    QString config_name = "";
    /// Current route directory
    QString route_dir = "";

    /// Vehicle index
    size_t  model_idx = 0;
    /// Train index
    size_t  train_idx = 0;
    /// Vehicle ODE system index
    size_t  state_idx = 0;

    /// Empty vehicle mass (without payload)
    double  empty_mass = 25000.0;
    /// Full payload mass
    double  payload_mass = 65000.0;
    /// Payload coefficient (0.0 - empty, 1.0 - full payload)
    double  payload_coeff = 0.0;
    /// Full vehicle mass
    double  full_mass = 25000.0;
    /// Length between coupling's axis
    double  length = 13.92;

    // Main resistant's coefficients
    double  b0 = 0.7;
    double  b1 = 8.0;
    double  b2 = 0.08;
    double  b3 = 0.002;
    double  q0 = full_mass / 4.0;
    double  W_coef = (b0 + b1 / q0) * Physics::g / 1000.0;
    double  W_coef_v = (b2 / q0) * Physics::g * Physics::kmh / 1000.0;
    double  W_coef_v2 = (b3 / q0) * Physics::g * Physics::kmh * Physics::kmh / 1000.0;
    double  W_coef_curv = 700.0 * Physics::g / 1000.0;

    // Wheels model's coefficients
    double  psi_a = 0.0;
    double  psi_b = 30.0;
    double  psi_c = 100.0;
    double  psi_d = 1.0;
    double  psi_e = 0.0;

    /// Numder of axis
    size_t              num_axis = 4;
    /// Wheels rotation angles
    std::vector<double> wheel_rotation_angle = {0.0, 0.0, 0.0, 0.0};
    /// Wheels angular velocities
    std::vector<double> wheel_omega = {0.0, 0.0, 0.0, 0.0};
    /// Wheel diameter
    std::vector<double> wheel_diameter = {0.95, 0.95, 0.95, 0.95};
    /// Wheel radius
    std::vector<double> rk = {0.475, 0.475, 0.475, 0.475};
    /// Axis moment of inertia
    std::vector<double> J_axis = {100.0, 100.0, 100.0, 100.0};
    /// Vertical axis load
    std::vector<double> axis_load = {full_mass/4.0, full_mass/4.0, full_mass/4.0, full_mass/4.0};
    /// Friction coefficient between wheel and rail
    std::vector<double> psi = {psi_a+psi_b/psi_c, psi_a+psi_b/psi_c, psi_a+psi_b/psi_c, psi_a+psi_b/psi_c};
    /// Friction coefficient changing
    double              psi_coeff = 1.0;

    /// Forward coupling force
    double  F_fwd = 0.0;
    /// Backward coupling force
    double  F_bwd = 0.0;
    /// Gravity force from profile inclination
    double  F_g = 0.0;

    /// Number of degrees of freedom
    size_t  s = num_axis + 1;

    /// Train coordinate
    double train_coord = 0.0;
    /// Body velocity
    double velocity = 0.0;

    /// Position at world and on railway
    profile_point_t profile_point_data = profile_point_t();

    /// Коллайдеры ПЕ (кузов, тележки, колёсные пары)
    VehicleCollision colliders;

    /// Вертикальная динамика ПС (неровности пути -> подвеска -> кузов)
    VehicleVerticalDynamics vertical_dynamics;

    /// Поперечная динамика ПС (виляние, коничность, крип, Y/Q)
    VehicleLateralDynamics lateral_dynamics;

    /// Система схода с рельсов
    VehicleDerailment derailment;

    /// Компонентные повреждения (сцепки/ходовая/колёса/тормоза/кузов/
    /// электро/силовая/ёмкости), 0..1 каждый
    VehicleDamageSystem damage_system;

    /// Опасный груз: утечки, пожар, взрыв
    VehicleHazard hazard;

    /// Ползуны колёсных пар
    WheelFlatSystem flat_spots;

    /// Сцепление колёс с рельсами (погода, загрязнение, самоочистка)
    WheelRailAdhesion adhesion;

    /// Пескоподача
    SandSystem sand;

    /// Тормозные колодки
    BrakeShoeSystem brake_shoes;

    /// Учёт электроэнергии (электровозы)
    EnergyMeterSystem energy;

    /// Токоприёмник
    PantographSystem pantograph;

    /// Деповское питание и АБ
    DepotPowerSystem depot_power;

    /// Система многих единиц: группа и роль (секция [SME])
    int sme_group = 0;
    bool sme_lead = false;

    /// Интерактивная сцепка/рукава/краны
    CouplingInteraction coupling_interaction;

    /// Лобовое стекло кабины
    WindshieldSystem windshield;

    /// Реакция тела машиниста
    CameraMotionFromPhysics camera_motion;

    /// Реестр интерактивной кабины
    CabInteractionRegistry cab_interaction;

    /// Груз
    CargoSystem cargo;

    /// Пассажиры
    PassengerSystem passengers;

    /// Система снабжения (колонки депо/ПТО)
    ServiceSystem service_system;

    /// Конденсат/лёд пневмосистемы
    CondensateSystem condensate_system;

    /// Износ колёсных пар
    WheelWearSystem wheel_wear;

    /// Аэродинамика тоннеля
    TunnelAerodynamics tunnel_aero;

    /// Противоюзная система (ТЗ "Сцепление", п.10-11): модуляция
    /// тормозного момента осей при юзе, множитель применяется при
    /// чтении Q_r в ОДУ без мутации самого Q_r
    WSPSystem wsp;

    /// Источник возвышения наружного рельса, мм (топология через
    /// контроллер ПЕ); пусто - возвышения нет
    std::function<double(double)> cant_source;

    /// Возвышение наружного рельса под центром ПЕ последнего шага, мм
    double rail_cant_mm = 0.0;

    /// Локальные часы ПЕ (время симуляции последнего шага), с - для
    /// кулдаунов звуковых событий
    double sim_clock = 0.0;

    /// Кулдауны звуковых событий по типам (индекс - SoundEventType):
    /// подавление спама, подготовка к аудио-потребителю. last - время
    /// последнего отправленного события типа
    double sound_event_last_time[10] = {};

    /// Интенсивность осадков от погоды
    double rain_intensity = 0.0;

    /// Продольное ускорение (для реакции машиниста), м/с^2
    double longitudinal_accel = 0.0;
    double prev_velocity = 0.0;

    /// База ЦМ из конфига (груз добавляет сдвиг сверху)
    double mass_center_longitudinal_base = 0.0;
    double mass_center_lateral_base = 0.0;

    /// Уровень детализации симуляции (ТЗ "Оптимизация")
    perf::SimLOD sim_lod = perf::SimLOD::L0_Full;

    /// Аккумуляторы адаптивных частот (п.4): термо 10 Гц, износ 5 Гц
    double thermal_accum = 0.0;
    double wear_accum = 0.0;

    /// Скорость ветра от погоды (токоприёмник)
    double wind_speed_for_pantograph = 0.0;

    /// Параметры ветровой нагрузки на кузов (секция [WindLoad], ТЗ
    /// "43-47", п.2): боковая площадь, Cd, плотность воздуха,
    /// детерминированные порывы и высота приложения силы
    bool wind_load_enabled = true;
    double wind_lateral_area = 0.0;   ///< 0 - автоматически length * 3.7
    double wind_drag_coeff = 1.1;
    double wind_air_density = 1.225;
    double wind_gust_period = 37.0;   ///< период порыва, с
    double wind_gust_min = 0.6;       ///< минимум модуля порыва (доля)
    double wind_app_height = 1.8;     ///< высота приложения, м

    /// Счётчики для детекта новых событий между опросами
    unsigned long last_flat_impacts = 0;
    unsigned long last_panto_arcs = 0;
    unsigned long last_joint_impacts = 0;  ///< стуки осей на стыках
    double prev_coupler_fwd = 0.0;         ///< сила сцепки на прошлом опросе, Н
    double prev_coupler_bwd = 0.0;

    /// Источник питания КС (привязывается моделью)
    std::function<catenary::FeedState(double, double)> catenary_feed;

    /// Проезд нейтральной вставки под током уже зафиксирован (на ПЕ)
    bool neutral_fault_reported = false;

    /// Приём рекуперации сетью (считается моделью по секции), Вт
    double regen_accept_w = 0.0;
    bool regen_accepted = false;

    /// Сила тяги/торможения на ободе последнего шага ОДУ (для учёта
    /// энергии: P = F*v)
    double last_wheel_traction = 0.0;

    /// Применять fade колодок к реактивным (тормозным) моментам ПЕ.
    /// Локомотивам с электротормозом - выключить и использовать
    /// getAxleEfficiency в пневматической части
    bool brake_shoes_apply_reactive = true;

    /// Множитель тормозного момента оси от fade колодок
    /// (температура/износ), применяется при чтении Q_r в ОДУ
    std::vector<double> brake_fade_eff = {1.0};

    /// Центр масс (ТЗ "Продольная динамика", п.5): высота над осями, м
    double mass_center_height = 1.8;
    /// Продольное смещение от середины ПЕ, м (+ к переду)
    double mass_center_longitudinal = 0.0;
    /// Поперечное смещение, м (+ вправо)
    double mass_center_lateral = 0.0;

    /// Статическое распределение нагрузки по осям от продольного
    /// смещения центра масс (сумма множителей = num_axis)
    std::vector<double> axle_load_share;

    /// ПЕ в аварийном состоянии после столкновения с препятствием
    bool is_collided = false;

    /// Повреждение кузова (0.0 - цел, 1.0 - разрушен)
    float body_damage = 0.0f;
    /// Повреждение ходовой части (0.0 - цела, 1.0 - разрушена)
    float bogie_damage = 0.0f;
    /// Энергия удара, разрушающая кузов, Дж
    double body_damage_threshold = 10.0e6;
    /// Энергия удара, разрушающая ходовую, Дж
    double bogie_damage_threshold = 3.0e6;
    /// Коэффициент доп. сопротивления от повреждений (доля от веса)
    double damage_resist_coeff = 0.05;

    /// Накопление повреждений от контакта коллайдера
    void applyCollisionDamage(const collision::CollisionEvent& event);

    /// Пересчёт статического распределения нагрузки по осям
    /// по продольному смещению центра масс
    void calcAxleLoadShare();

    /// Direction relative to train: 1 - co-directional, -1 - reversed
    std::int8_t dir = 1;

    /// Brake shoes state
    bool is_brake_shoes = false;

    /// Start autopilot after autostart
    bool auto_start_autopilot = false;

    bool needDebugMsg = false;
    QString DebugMsg = "";

    Vehicle* prev_vehicle = nullptr;
    Vehicle* next_vehicle = nullptr;

    /// Напряжение в КС
    double      Uks = 25000.0;

    /// Род тока в КС
    int         current_kind = 1;

    /// Active common forces
    state_vector_t  Q_a = {0.0, 0.0, 0.0, 0.0, 0.0};
    /// Reactive common forces
    state_vector_t  Q_r = {0.0, 0.0, 0.0, 0.0, 0.0};

    /// Keyboard state
    std::set<uint16_t> pressed_keys = {KEY_Undefined};
    std::vector<std::set<uint16_t>> pressed_keys_by_cabine = {{KEY_Undefined}};
    std::mutex keyboard_mutex;

    /// Analog signals for output
    std::vector<float>  analogSignal;

    /// List of devices - forward connectors
    device_list_t forward_connectors;
    /// List of devices - backward connectors
    device_list_t backward_connectors;
    /// List of devices - railway connectors
    device_coord_list_t railway_connectors;

    control_signals_t   control_signals;

    feedback_signals_t  feedback_signals;

    /// Automation control module
    std::vector<Autopilot *> autopilot;

    /// User defined initialization
    virtual void initialization();

    /// User defined configuration load
    virtual void loadConfig(QString cfg_path);

    /// User defined simulation process
    virtual void process(const simulator_time_t& t, const double& dt);

    /// User defined step prepare
    virtual void preStep(const double& t);

    /// User defined ODE integration step
    virtual void step(const double& t, const double& dt);

    /// User define step result processing
    virtual void postStep(const double& t);

    /// Recalculate coefficients for default main resistant formula
    virtual void mainResistCoeffs();

    /// Calculate main resistant to motion
    virtual double mainResist(const double& velocity);

    /// Calculate wheel-rail friction coefficient
    virtual double wheelrailFriction(const double& velocity);

    /// Calculate reduced wheel-rail friction coefficient when wheel slips
    virtual double wheelrailFrictionReducedBySlip(const double& psi, const double& slip_velocity);

    /// Add device to forward connectors
    void addFwdConnector(Device* device);
    /// Add device to backward connectors
    void addBwdConnector(Device* device);
    /// Add device to railway connectors
    void addRailwayConnector(Device* device, double distance_from_center = 0.0);

    /* Modkeys extended functions */

    bool isShift(int cab_num = -1) const;

    bool isControl(int cab_num = -1) const;

    bool isAlt(int cab_num = -1) const;

    bool getKeyState(uint16_t key, int cab_num = -1) const;

private:

    /// Default configuration load
    void loadConfiguration(QString cfg_path);

    /// Пересчёт боковой ветровой нагрузки на кузов и подача её в
    /// поперечную динамику (детерминированные порывы по времени)
    void updateWindLoad(double time_s);

    /// Load main resistence coefficients
    void loadMainResist(QString cfg_path, QString main_resist_cfg);
    /// Load wheel-rail friction coefficients
    void loadWheelRailFriction(QString cfg_path, QString wheel_rail_friction_cfg);
};

#endif // VEHICLE_H
