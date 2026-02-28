#ifndef     AUTOPILOT_H
#define     AUTOPILOT_H

#include    <device.h>
#include    <autopilot-types.h>
#include    <autopilot-brakes-control.h>
#include    <autopilot-accelerometer.h>
#include    <autopilot-timetable.h>

/*!
 * \class
 * \bref Абстрактный интерфейс для реализации функций систем автоведения
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT Autopilot : public Device
{
    Q_OBJECT

public:

    Autopilot(QObject *parent = nullptr) : Device(parent)
    {
        connect(rb_timer, &Timer::process, this, &Autopilot::slotVigilanceControl);
        connect(sand_timer, &Timer::process, this, &Autopilot::slotSandTimer);
    }

    ~Autopilot()
    {

    }

    /// Выдать сигналы управления для органов управления ПС
    virtual auto_control_t *getControl() = 0;

    /// Получить обратную связь от систем ПС
    virtual void setFeedback(auto_feedback_t *feedback)
    {
        this->feedback = feedback;
    }

    /// Активация автоведения
    void on()
    {
        if (!is_active)
        {
            is_active = true;
            emit sigInitTrainParams();
        }
    }

    /// Деактивация автоведения
    void off()
    {
        if (is_active)
        {
            is_active = false;
        }
    }

    /// Проверка активности автоведения
    bool isActive() const
    {
        return is_active;
    }

    void setTrainLength(double len)
    {
        train_length = len;
    }

    void setTrainMass(double mass)
    {
        train_mass = mass;
    }

    double getRefVelocity() const
    {
        return v_ref;
    }

    void step(double t, double dt) override;

    QString getDbgMsg();

    virtual void initAutoBrakeControl(const QString& modules_dir,
                                      const QString& custom_cfg_dir)
    {

    }

    void setVehicleIndex(int vehicle_idx)
    {
        this->vehicle_idx = vehicle_idx;
    }

    void setTimetable(const autopilot_timetable_t &timetable);

signals:

    void sigInitTrainParams();

    void sigGetVehicleTrajPosition(QString *traj_name, double *coord);

    void sigIsRouteExists(QString start_traj_name, QString end_traj_name, int dir, bool *exists);

    void sigGetRouteLength(QString cur_traj_name,
                           double cur_coord,
                           QString target_traj_name,
                           double target_coord,
                           int dir,
                           double *lenght);

protected:

    /// Признак активации
    bool is_active = false;    

    /// Выдержка РБ в нажатом положении
    const double RB_PRESS_DELAY = 1.5;

    /// Тамер выдержки РБ
    Timer *rb_timer = new Timer(RB_PRESS_DELAY, false);

    /// Заданная скорость
    double v_ref = 0.0;

    /// Конструкционная скорость
    double v_constr = 0.0;

    /// Длина поезда
    double train_length = 0;

    /// Предыдущее ограничение скорости
    double prev_v_lim = 0;

    /// Длина "хвоста" на более строгом ограничении
    double tail_len = 0;

    /// Масса поезда
    double train_mass = 0;

    double ref_length = 750.0;

    double ref_mass = 4700.0;

    /// Ускорение на кривой снижения скорости
    double a_brake_ref = 0.0;

    double a_brake = 0.0;

    /// Общая для всего автоведения структура обратной связи
    auto_feedback_t *feedback = nullptr;

    /// Запрет отпуска
    bool is_disable_release = false;

    /// Разрешено движение
    bool is_motion_allowed = false;

    /// Дистанция упреждения до КЖ
    double lead_dist_RY = 0.0;

    /// Дистанция упреждения до Ж
    double lead_dist_Y = 0.0;

    /// Ограничение скорости под КЖ
    double v_lim_RY = 60.0;

    /// Скорость для заперта отпуска
    double v_disable_release = 5.0;

    /// Целевая скорость
    double v_target = 0.0;

    /// Целевая дистанция
    double dist_target = 0.0;

    /// Предсказанный тормозной путь
    double dist_predict = 0.0;

    /// Превышение скорости при котором происходит отключение тяги
    double dV_traction_off = 0.25;

    /// Вычислитель текущего ускорения
    Accelerometer *accel_meter = new Accelerometer;

    MedianFilter<7> v_predict_filter;

    enum
    {
        NUM_VALUES = 10
    };

    std::array<double, NUM_VALUES> v_filter = {0.0};

    /// Интервал времени подачи песка
    const double SAND_TIME_INTERVAL = 10.0;

    Timer *sand_timer = new Timer(SAND_TIME_INTERVAL, false);

    autopilot_timetable_t timetable;

    /// Признак готовности к движению ко графику
    bool is_timetable_ready = false;

    /// Индекс станции-цели
    int target_station_idx = 0;

    /// Куда едем по графику (1 - туда, -1 - обратно)
    int target_dir = 1;

    /// Расстояние до станции
    double target_station_dist = 0;

    /// Текущая траектория
    QString curr_traj_name = "";

    /// Предыдущая траектория
    QString prev_traj_name = "";

    /// Текущая координата на траектории
    double curr_traj_coord = 0;

    /// Предыдущая координата на траектории
    double prev_traj_coord = 0;

    /// Индекс ПЕ, на которой работает данный модуль
    int vehicle_idx = 0;

    /// Переопределяем эту реализацию пустой, так как её может и не быть
    /// (что вряд ли, конечно...)
    void ode_system(const state_vector_t &Y,
                    state_vector_t &dYdt,
                    double t) override
    {
        (void) Y; (void) dYdt; (void) t;
    }

    /// Контроль бдительности
    void vigilance_control(double t, double dt);

    /// Контроль скорости
    void velocity_control(double t, double dt);

    /// Отпустить РБ
    virtual void release_RB()
    {

    }

    /// Нажать РБ
    virtual void press_RB()
    {

    }

    /// Включить песок
    virtual void sand_ON()
    {

    }

    /// Выключить песок
    virtual void sand_OFF()
    {

    }

    void load_config(CfgReader &cfg) override;

    double calcCurrentSpeedLimit(double t, double dt);

    double calcBrakeCurveSpeed(double v_target, double dist);

    double calcAlsnSpeed(ALSN alsn_code, double signal_dist, double &v_target);

    double calcPredictVelocity(double v_cur, double dist, double accel);

    void initTimeTable();

    /// Счисление пути - определяем текущую дистанцию до цели на станции
    void calcTargetDistance();

public slots:

    void slotSetBrakeAccel(double a_brake);    

private slots:

    void slotVigilanceControl();

    void slotSandTimer();
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef Autopilot* (*GetAutopilot)();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_AUTOPILOT(ClassName) \
    extern "C" Q_DECL_EXPORT Autopilot *getAutopilot() \
    {\
        return new (ClassName) ();\
    }

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" DEVICE_EXPORT Autopilot *loadAutopilot(QString lib_path);

#endif
