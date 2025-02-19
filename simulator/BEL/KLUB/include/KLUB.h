#ifndef     KLUB_H
#define     KLUB_H

#include    "safety-device.h"
#include    "ALSN-struct.h"
#include    "reversor-direction-enum.h"
#include    "train-direction-enum.h"
#include    "vigilanse-check-state-enum.h"
#include    "code-info-msg-enum.h"

class AbstractDeviceTSKBM;
class SpeedMap;
struct speed_limit_t;
struct station_t;
struct sound_state_t;

class KLUB final : public SafetyDevice
{

public:

    explicit KLUB(QObject *parent = nullptr);
    ~KLUB() = default;

    bool init() override;

    void step(double t, double dt) override;

    /// Прием кода АЛСН
    void setAlsnCode(int code_alsn) override;

    /// Прием состояния РБ
    void setRBstate(bool state) override;

    /// Прием состояния РБС
    void setRBSstate(bool state) override;

    /// Прием состояния РБП
    void setRBPstate(bool state) override;

    /// Прием состояния ключа ЭПК
    void setKeyEPK(bool key_epk) override;

    /// Прием состояния срыва ЭПК
    void setFailureEPKState(bool failure_EPK_state) override;

    /// Прием состояния нулевой позиция контроллера тяги
    void setTractionIsZero(bool traction_is_zero) override;

    /// Вернуть флаг работы устройства ТСКБМ
    bool isWorkingTSKBMDevice() const override;

    /// Вернуть состояние цепи удерживающей катушки ЭПК
    bool getEPKPowerState() const override;

    /// Получить состояние лампы локомотивного световора
    float getLampState(size_t lamp_idx) const override;

    /// Получить активную лампу локомотивного светофора
    float getLampNum() override;

    double getCurrentSpeedLimit() const override;

    double getNextSpeedLimit() const override;

    /// Вернуть флаг работы устройства в режиме ЭК
    bool isModeEK() const override;

    bool isDisplayON() const override;

    void setVoltage(double U_pow) override;

    /// Задать конструкционную скорость
    void setMaxVelocity(double v_max) override;

    /// Вернуть скорость поезда в км/ч
    double getVelocityKmh() const override;

    /// Вернуть состояние индикации сигнала "проверка бдительности"
    int getStateVigilanseCheck() const override;

    /// Вернуть ускорение поезда
    double getAcceleration() const override;

    /// Задать координату центра локомотива в пространстве
    void setCoord(dvec3 coord) override;

    /// Задать координату
    void setRailCoord(double rail_coord) override;

    /// Модуль работы с ограничениями скорости на путевой топологии
    void setSpeedMapModule(SpeedMap *device) override;

    /// Задать длину поезда
    void setTrainLength(double train_length) override;

    /// Задать направление движения поезда(чётное/нечётное)
    void setTrainDirection(int train_dir) override;

    /// Задать состояние реверсора
    void setReversorDirection(int reversor_direction) override;

    double getLimitDistance() const override;

    double getRailCoord() const override;

    QString getCurStation() const override;

    int getStationIndex() const override;

    /// Задать имя цели
    void setNameTarget(QString &name_target) override;

    /// Вернуть время по графику
    QString getScheduleTime() const override;

    /// Задать диаметр колеса
    void setWheelDiameter(double wheel_diameter) override;

    /// Задать угловую скороcть колесной пары поезда
    void setOmega(double omega) override;

    /// Задать дистанцию до цели
    void setDistanceTarget(int distance_target) override;

    /// Вернуть код информационного сообщения
    int getCodeInfoMsg() const override;

    /// Загрузка станций из ЭК
    bool loadStationsMap(const QString &path) override;

    void load_config(CfgReader &cfg) override;

    float getSoundSignal(size_t idx = ON_SOUND) const override;

private:

    struct KLUB_cfg
    {
        /// Минимальный интервал периодической проверки бдительности при К, КЖ, Ж - в режиме выключенного ТСКБМ
        int     min_vig_check_interval = 0;
        /// Максимальный интервал периодической проверки бдительности при К, КЖ, Ж - в режиме выключенного ТСКБМ
        int     max_vig_check_interval = 0;
        /// Минимальный интервал периодической проверки бдительности при З, Б - в режиме выключенного ТСКБМ
        int     min_green_vig_check_interval = 0;
        /// Максимальный интервал периодической проверки бдительности при З, Б - в режиме выключенного ТСКБМ
        int     max_green_vig_check_interval = 0;
        /// Интервал периодической проверки бдительности при прочих ситуациях - в режиме выключенного ТСКБМ
        int     other_vig_check_interval = 0;
        /// Интервал до срыва ЭПК при тяговом положении контроллера и отсутствии движения
        int     motion_control_interval = 0;
        /// Интервал до срыва ЭПК при периодической проверке бдительности
        double  failure_EPK_interval = 0.0;
        /// Интервал предупреждающего звукового сигнала при превышении скорости
        double  beep_interval = 0.0;
        /// Интервал удержания рукояток РБ, РБС, РБП
        double  RB_hold_interval = 0.0;
        /// Допустимое время удержания рукояток РБ, РБС, РБП
        double  RB_accept_holding_time = 0.0;
        /// Допустимая скорость движения при жёлтом сигнале
        double  V_yellow = 0.0;
        /// Ускорение для рассчёта кривой снижения скорости
        double  acceleration_slow_down = 0.0;
        /// Погрешность ограничения скорости
        double  speed_limit_error = 0.0;
        /// Плагин устройства ТСКБМ
        QString TSKBM_device_plugin = "";
        /// Флаг работы устройства ТСКБМ
        bool    is_working_TSKBM_device = false;

        KLUB_cfg() = default;
    };

    enum
    {
        STATION_MAX_SYMBOLS = 8,
        INFO_MAX_SYMBOLS = 24,
    };

    enum
    {
        NUM_LAMPS = 8,
        WHITE_LAMP = 0,
        RED_LAMP = 1,
        RED_YELLOW_LAMP = 2,
        YELLOW_LAMP = 3,
        GREEN_LAMP1 = 4,
        GREEN_LAMP2 = 5,
        GREEN_LAMP3 = 6,
        GREEN_LAMP4 = 7
    };

    enum
    {
        DIFF_NUM = 3
    };

    KLUB_cfg klub_cfg;

    /// Устройство ТСКБМ
    AbstractDeviceTSKBM *TSKBM = nullptr;

    /// База станций
    std::vector<station_t> stations;

    /// Массив значений скоростей для численного дифференцирования
    std::array<double, DIFF_NUM> v_i;

    std::array<sound_state_t, NUM_SOUNDS> sound_states;

    std::array<float, NUM_LAMPS> lamps = {0.0f, 0.0f, 0.0f, 0.0f,
                                          0.0f, 0.0f, 0.0f, 0.0f};

    /// Модуль работы с ограничениями скорости на путевой топологии
    SpeedMap *speed_map = nullptr;

    /// Таймер периодической проверки бдительности
    Timer   *safety_timer = new Timer(40.0, false, this);

    /// Таймер срыва ЭПК при периодической проверке бдительности
    Timer   *failure_EPK_timer = new Timer(5.0, false, this);

    /// Таймер предупреждающего звукового сигнала при превышении скорости
    Timer   *beepTimer = new Timer(0.55, true, this);

    /// Таймер снижения максимально допустимой скорости движения
    /// при жёлтом сигнале в режиме отсутствия ЭК
    Timer   *V_yellow_reduce_timer = new Timer(1.0, true, this);

    /// Таймер контроля движения при тяговом положении контроллера
    Timer   *motion_control_timer = new Timer(74.0, false, this);

    Trigger epk_power_state;

    Trigger is_red;

    QString name_target = "";

    QString cur_station = "";

    /// Время по графику
    QString schedule_time = "";

    /// Текущая ордината локомотива
    double  rail_coord = 0.0;

    /// Положение центра локомотива в пространстве
    dvec3   coord = {0.0, 0.0, 0.0};

    double  train_length = 22.532;

    double  acceleration = 0.0;

    /// Дистанция до ограничения
    double  limit_dist = 0.0;

    /// Диаметр колеса
    double  wheel_diameter = 0.92;

    /// Угловая скороcть колесной пары поезда
    double  omega = 0.0;

    /// Допустимая скорость движения при зелёном сигнале
    double  V_green = 160.0;

    /// Максимально допустимая скорость движения на участке пути
    /// (текущее ограничение скорости)
    double  V_permissible = 250.0;

    /// Целевая скорость - скорость проезда места начала ограничения скорости
    /// (следующее ограничение скорости)
    double  V_target = 250.0;

    double  U_pow = 0.0;

    double  U_nom = 50.0;

    /// Фактическая скорость движения в км/ч
    double  V_fact_kmh = 0.0;

    /// Фактическая скорость движения в м/с
    double  V_fact_ms = 0.0;

    /// Время удержания рукоятки РБ
    double  t_holding_RB = 0.0;

    /// Время удержания рукоятки РБС
    double  t_holding_RBS = 0.0;

    /// Шаг дифференцирования скорости
    double  delta_t = 0.1;

    size_t  v_count = 0;

    double  t_diff = 0.0;

    double  dt = 0.0;

    int     code_alsn = ALSN::NO_CODE;

    int     old_code_alsn = ALSN::NO_CODE;

    /// состояние индикации сигнала "проверка бдительности"
    int     vigilanse_check_state = SignalVigilanseCheckState::NoneIndication;

    /// Направление движения поезда
    int     train_dir = Direction::Even;

    /// Состояние реверсора
    int     reversor_direction = ReversorDirection::Neutral;

    /// Код информационного сообщения
    int     code_info_msg = CodeInfoMsg::None;

    /// Дистанция до цели
    int     distance_target = 0;

    /// Индекс станции из ЭК
    int     station_idx = -1;

    bool    state_RB = false;

    bool    state_RB_old = false;

    bool    state_RBS = false;

    bool    state_RBS_old = false;

    bool    state_RBP = false;

    bool    state_RBP_old = false;

    bool    key_epk = false;

    bool    key_epk_old = false;

    /// флаг работы устройства ТСКБМ
    bool    is_working_TSKBM_device = true;

    /// Флаг работы устройства в режиме ЭК
    bool    is_mode_EK = true;

    /// Состояние срыва ЭПК
    bool    failure_EPK_state = false;

    /// Флаг срыва ЭПК от ТСКБМ
    bool    is_TSKBM_failure_EPK = false;

    /// Флаг необходимости проверки контроля скатывания
    bool    is_roll_control = false;

    /// Флаг необходимости однократной проверки бдительности
    /// при трогании в поездном режиме по сигналам КЖ, К, Б
    bool    is_start_move_prohibiting = false;

    /// Флаг проверки бдительности при смене сигнала АЛСН на КЖ и Б
    bool    is_changing_ALSN = false;

    /// Флаг снижения скорости по кривой
    bool    deceleration_curve = false;

    /// Нулевая позиция контроллера тяги
    bool    traction_is_zero = false;

    bool    is_dislplay_ON = false;

    void preStep(state_vector_t &Y, double t) override;

    /// Периодические проверки бдительности без ТСКБМ
    void periodicVigilanceChecks();

    /// Однократная проверка бдительности при трогании в поездном режиме по сигналам КЖ, К, Б
    bool checkVigStartMoveProhibiting();

    /// Изменение сигнала АЛСН
    bool isALSNChange();

    /// Однократная проверка бдительности при смене сигнала АЛСН на запрещающий
    bool checkVigChangingALSN();

    /// Проверка РБ
    void checkRB();

    /// Проверка РБС
    void checkRBS();

    /// Обработка удержания рукояток РБ, РБС, РБП
    void onHoldRB();

    void alsn_process(int code_alsn);

    /// Озвучка
    void soundsProcess();

    /// Проверка смены сигнала АЛСН
    void checkChangesALSN();

    /// Звук при начале снижения скорости по кривой
    void soundDecelerationCurve(double V_permissible_lim);

    /// Вычисление скорости
    void calc_speed();

    /// Вычисление ускорения
    void calc_acceleration(double t, double dt);

    /// Загрузка плагина устройства ТСКБМ
    bool loadTSKBMDevicePlugin(const QString &plugins_dir);

    /// Вернуть путь к каталогу с плагинами
    QString getPluginsDirectory() const;

    /// Контроль превышения допустимой скорости
    bool speed_control();

    /// Контроль скатывания
    bool rollControl();

    /// Расчет ограничений скорости по ЭК
    void calcSpeedLimits();

    /// Расчет ограничений скорости при отсутствии ЭК
    void calcSpeedLimitsNoEK();

    /// Ограничения по сигналам АЛСН в режиме ЭК
    void limitsALSN(speed_limit_t &V_permissible_lim, speed_limit_t &V_target_lim);

    /// Определение текущей станции
    void stations_process();

    /// Вернуть интервал таймера периодической проверки бдительности
    int getSafetyTimerInterval(int min, int max) const;

    /// Периодическая проверка бдительности при движении поезда в определённых ситуациях
    void trainMovSafetyCheck(int min, int max);

    /// Реверсор в позиции назад
    bool isReversorBackward() const;

    /// Проверка на движение поезда
    bool isMove();

    /// Срыв ЭПК
    void failureEPK();

    /// Управление срывом цепи удерживающей катушки ЭПК от ТСКБМ
    void controlResetEPKPowerState();

    /// Нажатие РБ/РБС
    void setEPKPowerState();

    /// Запуск таймера срыва ЭПК при периодической проверке бдительности
    void startFailureEPKTimer() const;
    /// Остановка таймера срыва ЭПК при периодической проверке бдительности
    void stopFailureEPKTimer() const;

    /// Запуск таймера периодической проверки бдительности
    /// с указанием минимального и максимального интервала
    void startSafetyTimer(int min, int max) const;

    /// Перегрузка: Запуск таймера периодической проверки бдительности с фиксированным интервалом
    void startSafetyTimer(int interval) const;

    /// Остановка таймера периодической проверки бдительности
    void stopSafetyTimer() const;

    /// Запуск таймера предупреждающего звукового сигнала при превышении скорости
    void startBeepTimer() const;
    /// Остановка таймера предупреждающего звукового сигнала при превышении скорости
    void stopBeepTimer() const;

    /// Запуск таймера снижения максимально допустимой скорости движения
    /// при жёлтом сигнале в режиме отсутствия ЭК
    void startVYellowReduceTimer() const;
    /// Остановка таймера снижения максимально допустимой скорости движения
    /// при жёлтом сигнале в режиме отсутствия ЭК
    void stopVYellowReduceTimer() const;

    /// Запуск таймера контроля движения при тяговом положении контроллера
    void startMotionControlTimer() const;
    /// Остановка таймера контроля движения при тяговом положении контроллера
    void stopMotionControlTimer() const;

    /// Действия при выключенном ЭПК
    void EPKTurnedOff();

    /// Включение дисплея
    bool turnOnDisplay();

    /// Предупреждающий звуковой сигнал
    void warningBeep();

    /// Информационное сообщение о режиме без ЭК
    void infoMsgNoEKMode();

    /// Информационное сообщение о срыве ЭПК
    void infoMsgFailureEPK();

private slots:
    /// Обработчик таймера периодической проверки бдительности
    void onSafetyTimer();

    /// Обработчик таймера срыва ЭПК при периодической проверке бдительности
    void onFailureEPKTimer();

    /// Обработчик таймера предупреждающего звукового сигнала при превышении скорости
    void onBeepTimer();

    /// Обработчик сигнала таймера периодической проверки бдительности ТСКБМ
    void onTSKBMVigilanceCheck();

    /// Обработчик сигнала таймера срыва ЭПК при периодической проверке бдительности ТСКБМ
    void onTSKBMfailureEPK();

    /// Обработчик сигнала таймера снижения максимально допустимой скорости движения
    /// при жёлтом сигнале в режиме отсутствия ЭК
    void onVYellowReduceTimer();

    /// Обработчик сигнала таймера контроля движения при тяговом положении контроллера
    void onMotionControlTimer();
};

#endif // KLUB_H
