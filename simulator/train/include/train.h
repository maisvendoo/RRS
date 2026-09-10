//------------------------------------------------------------------------------
//
//      Common train's model dynamics
//      (c) maisvendoo, 04/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Common train's model dynamics
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 04/09/2018
 */

#ifndef     TRAIN_H
#define     TRAIN_H

#include    "global-const.h"
#include    "datetime.h"
#include    "init_data.h"
#include    "ode-system.h"
#include    "vehicle.h"
#include    "device-list.h"
#include    "device-joint.h"
#include    "solver.h"
#include    "solver-config.h"

#include    <conductor-system.h>

#include    <topology.h>

#include    <QByteArray>

#include    <deque>

#if defined(TRAIN_LIB)
    #define TRAIN_EXPORT    Q_DECL_EXPORT
#else
    #define TRAIN_EXPORT    Q_DECL_IMPORT
#endif

/*!
 * \class
 * \brief Common train model
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TRAIN_EXPORT Train : public OdeSystem
{
    Q_OBJECT

public:

    /// Constructor
    explicit Train(QObject* parent = nullptr);
    /// Destructor
    virtual ~Train();

    /// Train initialization
    bool init(const init_data_t& init_data, int model_vehicles_count = -1);

    /// Train initialization
    bool init(const solver_config_t& solver_config, std::vector<Vehicle*>& vehicles, state_vector_t& state_vector, std::vector<std::vector<Joint*>>& joints_list);

    /// Train coupling
    void couple(double current_distance, bool is_coupling_to_head, bool is_other_coupled_by_head, Train* other_train = nullptr);

    /// Train uncoupling
    Train* uncouple(double uncoupling_distance);

    /// Set distance to stop the train before end of trajectory
    void setDistanceToEndOfTrajectory(bool is_train_head, double distance);

    /// Set train index
    void setTrainIndex(size_t idx);

    /// Get train index
    size_t getTrainIndex() const;

    /// Calculation of right part motion ODE's
    void calcDerivative(state_vector_t &Y, state_vector_t &dYdt, double t, double dt);

    /// Get first vehicle
    Vehicle* getFirstVehicle() const;

    /// Get last vehicle
    Vehicle* getLastVehicle() const;

    state_vector_t getStateVector();

    std::vector<std::vector<Joint*>> getJoints();

    double getVelocity(size_t i = 0) const;

    /// Get train mass
    double getMass() const;
    /// Get train length
    double getLength() const;

    size_t getVehiclesNumber() const;

    /// Сводка продольной динамики состава (диагностика ТЗ, п.33)
    struct LongitudinalStats
    {
        double train_mass = 0.0;         ///< Масса состава, кг
        double train_length = 0.0;       ///< Длина состава, м
        double max_tension = 0.0;        ///< Максимум растяжения сцепок, Н
        double max_compression = 0.0;    ///< Максимум сжатия сцепок, Н
        double max_abs_force = 0.0;      ///< Максимум |усилия|, Н
        int overloaded_joints = 0;       ///< Сцепок с повреждением > 0
        int broken_joints = 0;           ///< Разрушенных сцепок
    };

    /// Текущая сводка продольной динамики
    LongitudinalStats getLongitudinalStats() const;

    /// Снимок диагностики для экранов машиниста (ТЗ "Статистика
    /// вагонов"): данные из реальных физических систем
    struct VehicleDiagnostics
    {
        size_t vehicle_idx = 0;
        double mass_kg = 0.0;
        double speed_kmh = 0.0;
        double longitudinal_force_n = 0.0;  ///< Продольное усилие (сцепки)
        double vertical_accel = 0.0;        ///< Вертикальное ускорение кузова
        double lateral_accel = 0.0;         ///< Поперечное ускорение кузова
        double body_damage = 0.0;           ///< Повреждения кузова 0..1
        double bogie_damage = 0.0;
        double brake_efficiency = 1.0;      ///< Fade колодок
        double shoe_temperature = 20.0;
        double rail_coord_m = 0.0;          ///< Пикетаж
        double inclination = 0.0;           ///< Уклон, промилле
        bool derailed = false;
        bool coupled_fwd = true;
        bool coupled_bwd = true;
    };

    /// Построить снимок диагностики всех ПЕ (реальные данные систем)
    std::vector<VehicleDiagnostics> buildDiagnosticsSnapshot() const;

    /// Отладочная строка продольных сил по вагонам (ТЗ, п.34):
    /// номера ПЕ пары, усилие, состояние каждой сцепки
    QString getLongitudinalDebugMsg() const;

    QString getClientName();

    QString getTrainID();

    std::vector<Vehicle*>* getVehicles();

    void setTopology(Topology* topology);

    void setName(const std::string &name)
    {
        this->name = name;
    }

    std::string getName() const
    {
        return this->name;
    }

    /// Табельный номер игрока, закреплённого за поездом (ТЗ "RP-сервер",
    /// п.9): связан с ID пользователя на сайте, по нему выполняется
    /// автоназначение при подключении. -1 - поезд не закреплён
    void setTabNumber(int tab_number)
    {
        this->tab_number = tab_number;
    }

    int getTabNumber() const
    {
        return this->tab_number;
    }

    /// Система проводников пассажирских вагонов (ТЗ "Система
    /// проводников"): создаётся по вагонам с настроенной секцией
    /// [PassengerCar]; контекст станции задаёт модель
    conductor::ConductorSystem& getConductors();
    const conductor::ConductorSystem& getConductors() const;

    /// Разрешено ли отправление: готовность проводников поезда
    /// (система выключена или проводников нет - true)
    bool isDepartureAllowed() const;

public slots:

    /// Integration step
    void slotStep(const simulator_time_t& current_time, const double& integration_time);

signals:

    /// Integration step done
    void stepDone(int idx);

private:

    /// Имя поезда, служащее ему уникальным идентификатором
    std::string name = "";

    /// Табельный номер закреплённого игрока (ТЗ "RP-сервер", п.9)
    int tab_number = -1;

    /// Train index
    size_t      train_idx = 0;

    /// Train mass
    double      trainMass = 0.0;
    /// Train length
    double      trainLength = 0.0;

    /// Distance to stop the head of train before end of trajectory
    double      distance_to_stop_head = DISTANCE_TO_COUPLE_TRAINS;
    /// Distance to stop the tail of train before end of trajectory
    double      distance_to_stop_tail = DISTANCE_TO_COUPLE_TRAINS;

    /// Order of system ODE motion
    size_t      ode_order = 0;

    /// Coefficient to friction between wheel and rail
    double      coeff_to_wheel_rail_friction = 1.0;

    /// Charging pressure
    double      charging_pressure = 0.0;

    /// No air flag (for empty air system on start)
    bool        no_air = false;

    /// Initial main reservoir pressure
    double      init_main_res_pressure = 0.0;

    /// Motion ODE's solver
    Solver*     train_motion_solver = nullptr;

    /// Имя сетевого клиента для ВЖД
    QString     client_name;

    /// Идентификатор поезда для ВЖД
    QString     train_id;

    /// All train's vehicles
    std::vector<Vehicle*> vehicles;

    /// Проводники пассажирских вагонов (ТЗ "Система проводников")
    conductor::ConductorSystem conductors;

    /// All joints between neighbor vehicles
    std::vector<std::vector<Joint*>> joints_list;

    /// Solver's configuration
    solver_config_t solver_config;

    Topology* topology = nullptr;

    /// Train's loading
    bool loadTrain(QString cfg_path, const init_data_t &init_data, int model_vehicles_count = -1);
    /// Создание проводников по текущим вагонам состава
    /// (пассажирские = с настроенной секцией [PassengerCar]).
    /// Вызывается при загрузке поезда и при сцепке/расцепке
    void attachConductors();
    /// Joints loading
    bool loadTrainJoints();
    /// Joints loading
    void loadJoints(device_list_t* cons_fwd, device_list_t* cons_bwd, std::vector<Joint*>& joints);
    /// Joint module loading
    void loadJointModule(Device* con_fwd, Device* con_bwd, std::vector<Joint*>& joints);

    /// Set initial conditions
    void setInitConditions(const init_data_t &init_data);

    /// Initialization of vehicles brakes
    void initVehiclesBrakes();

    /// Set initial conditions
    double calcStopForce(double distance, double veh_velocity, double veh_mass, double dt);

    /// Диагностика продольной динамики: события перегрузки/разрушения
    /// сцепок в журнал (ТЗ, п.25, 35). Вызывается раз в шаг модели
    void stepLongitudinalDiagnostics();

    /// Шаг СМЕ: передача управляющих сигналов от головного локомотива
    /// ведомым с реалистичной задержкой; контроль целостности связи
    /// (обрыв сцепки = потеря управления) - ТЗ "Система многих единиц"
    void stepMultipleUnit(double dt);

    /// Изменились ли команды поезда (контрольная сумма активных
    /// аналоговых сигналов всех ПЕ) - условие выхода из L3-заморозки
    /// (ТЗ "Оптимизация", п.5)
    bool lodCommandsChanged();

    /// Сводка, вычисленная последним шагом диагностики
    LongitudinalStats longitudinal_stats;

    /// Сцепки, о перегрузке которых уже сообщено (индексы соединений)
    std::vector<bool> overload_reported;

    /// Сцепки, о разрушении которых уже сообщено
    std::vector<bool> break_reported;

    /// Задержка передачи команд СМЕ, с (конфиг поезда, секция [SME],
    /// ключ CommandDelay; по умолчанию 0.15)
    double sme_command_delay = 0.15;

    /// Очередь команд СМЕ: (время постановки, сигналы головного)
    std::deque<std::pair<double, control_signals_t>> sme_queue;

    /// Связь СМЕ потеряна (обрыв сцепки)
    bool sme_link_lost = false;

    /// Локальное время очереди СМЕ, с
    double t_sme = 0.0;

    ///=== Агрегированная модель L2/L3 (ТЗ "Оптимизация", п.3-5) ===
    /// Накопитель времени между шагами пониженной частоты:
    /// L2 - шаг раз в 100 мс, L3 - раз в 500 мс (заморозка:
    /// без движения и смены команд шаг не выполняется). При возврате
    /// на L0 скачков нет: состояния непрерывны, шагается накопленный
    /// интервал с внутренними подшагами решателя
    double lod_step_accum = 0.0;

    /// Контрольная сумма команд поезда прошлого кадра (L3-заморозка)
    float lod_command_checksum = 0.0f;
    bool lod_checksum_valid = false;
};

#endif // TRAIN_H
