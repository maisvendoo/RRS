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

#include    <simulator-command-line.h>
#include    <filesystem.h>
#include    <train.h>
#include    <elapsed-timer.h>

#include    <global-const.h>
#include    <simulator-info-struct.h>
#include    <simulator-update-struct.h>
#include    <controlled-struct.h>

//#include    <keys-control.h>

#include    <virtual-interface-device.h>

#include    <traffic-machine.h>

#include    <topology.h>

#include    <tcp-server.h>

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
    explicit Model(QObject *parent = Q_NULLPTR);
    /// Destructor
    virtual ~Model();

    /// Model initialization
    bool init(const simulator_command_line_t &command_line);

    /// Start simulation thread
    void start();

    /// Check is simulation started
    bool isStarted() const;

signals:

    void step(double t, double dt);

    void sendDataToServer(QByteArray data);

    //void getRecvData(sim_dispatcher_data_t &disp_data);

public slots:

    /// Messages output
    void outMessage(QString msg);

    ///
    void controlProcess();

    void deleteFinishedThread();

private:

    /// Current simulation time
    double      t = 0.0;
    /// Simulation start time
    double      start_time = 0.0;
    /// Simulation stop time
    double      stop_time = 1000.0;
    /// Flag of integration step is correct
    bool        is_step_correct = true;
    /// Flag is simulation thread started
    bool        is_simulation_started = false;
    /// Delay for realtime simulation
    int         realtime_delay = 0;
    /// Minimal intergation interval
    int         integration_time_interval = 100;
    /// Flag, which allow debug print
    bool        is_debug_print = false;

    /// Feedback with vehicles positions
    simulator_update_pos_t      update_pos_data = simulator_update_pos_t();
    /// Feedback with vehicles state
    simulator_update_t          update_data = simulator_update_t();
    /// Feedback with player's current and controlled vehicles
    simulator_update_players_t  update_players = simulator_update_players_t();
    /// Vehicle control and feedback with debug message
    struct controlled_client_t
    {
        int prev_vehicle_controlled = -1;
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

    /// Система трафика
    TrafficMachine  *traffic_machine = nullptr;

    /// Топология
    Topology *topology = new Topology();

    /// Simulation thread
    QThread     model_thread;

//    KeysControl keys_control;
/*
    QSharedMemory   memory_sim_info;
    QSharedMemory   memory_sim_update;
    QSharedMemory   memory_controlled;
    QSharedMemory   keys_data;
    QByteArray      data;
*/
    QTimer          controlTimer;
    QTimer          networkTimer;

    ElapsedTimer    simTimer;

    /// TCP-server
    TcpServer   *tcp_server = new TcpServer;

    /// Вектор данных о нескольких поездах
    std::vector<init_data_t> init_datas;

    /// Find trains which are near to each other and couple them
    void findNearestVehicles();

    /// Find trains which have distances between its vehicles and uncouple them
    void findFarthestVehicles();

    /// Debug print to stdout
    void debugPrint();

    /// Initial data loading
    void loadInitData(init_data_t &init_data);

    /// Override of initial data by command line
    void overrideByCommandLine(init_data_t &init_data, const simulator_command_line_t &command_line);

    /// Solver configuration loading
    void configSolver(solver_config_t &solver_config);

    void initControlPanel(QString cfg_path);

    /// Инициализация поезда
    Train *addTrain(const init_data_t &init_data);

    /// Инициализация трафика
    void initTraffic(const init_data_t &init_data);

    /// Инициализация топологии
    void initTopology(const init_data_t &init_data);

    /// Инициализация TCP-сервера
    void initTcpServer();

    /// Подготовка данных перед передачей серверу для рассылки клиентам
    void prepareFeedBack();

    /// TCP feedback
    void tcpFeedBack();
/*
    /// Shered memory feedback
    void sharedMemoryFeedback();
*/
    void controlStep();

private slots:

    void process();

    void slotGetTopologyData(QByteArray &topology_data);

    void slotGetSignalsData(QByteArray &signals_data);

    void slotGetVehicleControlByKeyboard(QByteArray &control_data, int client_id);

    void slotResetVehicleControlByKeyboard(int client_id);
};

#endif // MODEL_H
