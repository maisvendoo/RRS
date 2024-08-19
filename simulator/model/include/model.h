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

#include    <simulator-info-struct.h>
#include    <simulator-update-struct.h>
#include    <controlled-struct.h>

#include    <profile.h>

#include    <keys-control.h>

#include    <virtual-interface-device.h>

#include    <signaling.h>

#include    <traffic-machine.h>

#include    <topology.h>

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

    void sendDataToServer(QByteArray data);

//    void sendDataToTrain(QByteArray data);

    //void getRecvData(sim_dispatcher_data_t &disp_data);

public slots:

    /// Messages output
    void outMessage(QString msg);

    ///
    void controlProcess();

    /// Обмен данными с ВЖД
    void virtualRailwayFeedback();

private:

    /// Current simulation time
    double      t = 0.0;
    /// Current simulation time step
    double      dt = 1e-3;
    /// Current simulation time in current integration interval
    double      tau = 0.0;
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

    double      control_time = 0.0;
    double      control_delay = 0.05;

    /// Vehicle which selected by user for view
    int             current_vehicle = -1;
//    int             prev_current_vehicle;

    /// Vehicle which selected by user for control
    int             controlled_vehicle = -1;
    int             prev_controlled_vehicle = -1;

    /// Train model
    Train       *train = nullptr;

    /// Profile
    Profile     *profile = nullptr;

    /// TCP-server
    //Server      *server = nullptr;

    /// Виртуальное устройство для сопряжения с внешним пультом
    VirtualInterfaceDevice  *control_panel = nullptr;

    /// Клиент для связи с ВЖД
    //SimTcpClient *sim_client = nullptr;

    /// Система СЦБ
    Signaling *signaling = nullptr;

    /// Система трафика
    TrafficMachine  *traffic_machine = nullptr;

    /// Топология
    Topology *topology = new Topology();

    /// Simulation thread
    QThread     model_thread;

    KeysControl keys_control;
/*
    /// Server data to clinet transmission
    server_data_t   viewer_data;*/
/*
    /// Server info to clinet transmission
    simulator_info_t   info_data;*/
    /// Server update data to clinet transmission
    simulator_update_t   update_data;

    QSharedMemory   memory_sim_info;
    QSharedMemory   memory_sim_update;
    QSharedMemory   memory_controlled;
//    QSharedMemory   shared_memory;
    QSharedMemory   keys_data;
    QByteArray      data;

    QTimer          controlTimer;
    QTimer          networkTimer;

    ElapsedTimer    simTimer;       

    /// Actions, which prerare integration step and also update shared data
    void preStep(double t);
    /// Simulation step
    bool step(double t, double &dt);
    /// Actions after integration step
    void postStep(double t);

    /// Debug print to stdout
    void debugPrint();

    /// Initial data loading
    void loadInitData(init_data_t &init_data);

    /// Override of initial data by command line
    void overrideByCommandLine(init_data_t &init_data, const simulator_command_line_t &command_line);

    /// Solver configuration loading
    void configSolver(solver_config_t &solver_config);

    void initControlPanel(QString cfg_path);

    void initSimClient(QString cfg_path);

    /// Инициализация СЦБ
    void initSignaling(const init_data_t &init_data);

    /// Инициализация трафика
    void initTraffic(const init_data_t &init_data);

    /// Инициализация топологии
    void initTopology(const init_data_t &init_data);

    /// TCP feedback
    void tcpFeedBack();


    /// Shered memory feedback
    void sharedMemoryFeedback();

    void controlStep(double &control_time, const double control_delay);    

private slots:

    void process();
};

#endif // MODEL_H
