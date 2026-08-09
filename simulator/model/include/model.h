//------------------------------------------------------------------------------
//
//      Train motion model simulation control
//      (c) maisvendoo, 02/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Train motion model simulation control
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 02/09/2018
 */

#ifndef     MODEL_H
#define     MODEL_H

#include    <QtGlobal>
#include    <QObject>
#include    <QThread>
#include    <QSharedMemory>
#include    <QTimer>
#include    <chrono>

#include    <simulator-command-line.h>
#include    <filesystem.h>
#include    <train.h>
#include    <elapsed-timer.h>

#include    <global-const.h>
#include    <datetime.h>
#include    <simulator-info-struct.h>
#include    <simulator-update-struct.h>
#include    <controlled-struct.h>

#include    <virtual-interface-device.h>

#include    <traffic-machine.h>

#include    <topology.h>

#include    <tcp-server.h>

#include    <scenario-manager.h>

#if defined(MODEL_LIB)
    #define MODEL_EXPORT Q_DECL_EXPORT
#else
    #define MODEL_EXPORT Q_DECL_IMPORT
#endif

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class MODEL_EXPORT Model : public QObject
{
    Q_OBJECT

public:

    /// Constructor
    explicit Model(QObject *parent = nullptr);
    /// Destructor
    virtual ~Model();

    /// Model initialization
    bool init(const simulator_command_line_t &command_line);

    /// Start simulation thread
    void start();

    /// Check is simulation started
    bool isStarted() const;

    /// Индексы ПЕ, управляемых какими-либо клиентами в данном поезде
    std::vector<size_t> getControlledVehiclesInTrain(std::size_t train_idx);

public slots:

    /// Messages output
    void outMessage(QString msg);

    void deleteFinishedThread();

    ///
    void controlProcess();

    void receiveSignalsFromControlPanel(const control_signals_t& control_signals);

    void slotSetSimSpeed(int speed_factor);

signals:

    void sendSignalsToControlPanel(feedback_signals_t feedback_signals);

    void sendDataToServer(QByteArray data);

    void step(const simulator_time_t& current_time, const double& integration_time);

    void sigInitTimetable();

private:

    /// Realtime start timepoint
    std::chrono::steady_clock::time_point start_timepoint = std::chrono::steady_clock::now();
    /// Realtime timepoint of current process() begin
    std::chrono::steady_clock::time_point process_timepoint = std::chrono::steady_clock::now();
    /// Current simulation time
    simulator_time_t sim_time = simulator_time_t::timeNow();
    /// Simulation start time
    double      start_time = 0.0;
    /// Delay for realtime simulation
    double      realtime_delay = 0;
    /// Flag of integration step is correct
    bool        is_step_correct = true;
    /// Flag is simulation thread started
    bool        is_simulation_started = false;
    /// Flag is trains changed since previous tcpFeedBack
    bool        is_trains_changed = true;
    /// Minimal intergation interval
    int         integration_time_interval = 100;

    int count_trains_done_its_step = -1;

    /// Feedback with vehicles state
    simulator_trains_update_t   update_trains = simulator_trains_update_t();
    /// Feedback with vehicles positions
    simulator_update_pos_t      update_pos_data = simulator_update_pos_t();
    /// Feedback with vehicles state
    simulator_vehicles_update_t update_vehicles = simulator_vehicles_update_t();
    /// Feedback with player's current and controlled vehicles
    simulator_update_players_t  update_players = simulator_update_players_t();
    /// Vehicle control and feedback with debug message
    struct controlled_client_t
    {
        int prev_vehicle_current = -1;
        int prev_vehicle_controlled = -1;
        int prev_cab_controlled = -1;
        controlled_t vehicle_control_by_keyboard = controlled_t();
        simulator_vehicle_controlled_update_t vehicle_controlled = simulator_vehicle_controlled_update_t();
    };
    QMap<int, controlled_client_t> controlled_clients;

    /// All vehicles
    std::vector<Vehicle *> vehicles;

    /// Train model
    std::vector<Train *> trains;

    /// Train threads
    std::vector<QThread *> train_threads;

    /// Виртуальное устройство для сопряжения с внешним пультом
    VirtualInterfaceDevice  *control_panel = nullptr;

    Vehicle* vehicle_controlled_by_panel = nullptr;

    /// Система трафика
    TrafficMachine  *traffic_machine = nullptr;

    /// Топология
    Topology *topology = new Topology();

    /// Simulation thread
    QThread     model_thread;

    QTimer          controlTimer;

    ElapsedTimer    simTimer;

    /// TCP-server
    TcpServer   *tcp_server = new TcpServer;

    /// Менеджер сценариев
    ScenarioManager *scnmgr = new ScenarioManager;

    /// Вектор данных о нескольких поездах
    std::vector<init_data_t> init_datas;

    /// Очередь на автозапуск в поездку по данным из сценария
    std::queue<Vehicle *> vehicles_for_autostart;

    /// Построение очереди автозапуска
    void buildAutostartQueue(Train *train);    

    /// Обработка очереди автозапуска
    void processAutostartQueue();

    /// Find trains which are near to each other and couple them
    void findNearestVehicles();

    /// Find trains which have distances between its vehicles and uncouple them
    void findFarthestVehicles();

    /// Initial data loading
    void loadInitData(init_data_t &init_data);

    /// Override of initial data by command line
    void overrideByCommandLine(init_data_t &init_data, const simulator_command_line_t &command_line);

    /// Solver configuration loading
    void configSolver(solver_config_t &solver_config);

    void initControlPanel(QString cfg_path);

    /// Инициализация поезда
    Train *addTrain(const init_data_t &init_data);    

    /// Инициализация топологии
    void initTopology(const init_data_t &init_data);

    /// Инициализация движка сценариев
    bool initScenarioManager(const init_data_t &init_data,
                             const simulator_command_line_t &command_line);

    /// Инициализация TCP-сервера
    void initTcpServer();

    /// Подготовка данных перед передачей серверу для рассылки клиентам
    void prepareFeedBack(bool need_trains_feedback);

    /// TCP feedback
    void tcpFeedBack(bool need_trains_feedback);

    void controlStep();

    int speed_factor = 1;

private slots:

    void process();

    void slotTrainStepDone(int idx);

    void slotGetTopologyData(QByteArray &topology_data);

    void slotGetSignalsData(QByteArray &signals_data);

    void slotGetVehicleControlByKeyboard(QByteArray &control_data, int client_id);

    void slotResetVehicleControlByKeyboard(int client_id);

    void slotRenameTrainInModel(int train_idx, QString new_name);

    void slotGetTrainParams(int train_idx, double &train_len, double &train_mass);

    /// Связывание сигналов и слотов для загрузки сценария в модули автоведения
    void slotUpdateTrainTimetable(int train_idx);    
};

#endif // MODEL_H
