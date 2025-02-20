#include    "KLUB.h"
#include    <QDebug>
#include    <QRandomGenerator>
#include    "abstract-device-TSKBM.h"
#include    "speed-limits.h"
#include    "filesystem.h"
#include    "speedmap.h"
#include    "klub-stations.h"

const QString KLUB_CFG = "KLUB-config";

constexpr double SPEED_START_MOVEMENT = 2.0;
/// Радиус поиска ближайшей станции
constexpr double STATION_SEARCH_RADIUS = 3000.0;
constexpr double EPS_SPEED = 0.1;
constexpr double TRAIN_STOP_EPS_SPEED = 0.001;

KLUB::KLUB(QObject *parent) : SafetyDevice(parent)
{

}

bool KLUB::init()
{
    this->read_config(KLUB_CFG);

    V_permissible = V_green + klub_cfg.speed_limit_error;

    epk_power_state.set();

    connect(safety_timer, &Timer::process, this, &KLUB::onSafetyTimer);
    connect(failure_EPK_timer, &Timer::process, this, &KLUB::onFailureEPKTimer);
    connect(beepTimer, &Timer::process, this, &KLUB::onBeepTimer);
    connect(V_yellow_reduce_timer, &Timer::process, this, &KLUB::onVYellowReduceTimer);
    connect(motion_control_timer, &Timer::process, this, &KLUB::onMotionControlTimer);

    QString plugins_dir = getPluginsDirectory();

    if(loadTSKBMDevicePlugin(plugins_dir))
    {
        connect(TSKBM, &AbstractDeviceTSKBM::TSKBMVigilanceCheck, this, &KLUB::onTSKBMVigilanceCheck);
        connect(TSKBM, &AbstractDeviceTSKBM::TSKBMfailureEPK, this, &KLUB::onTSKBMfailureEPK);
    }
    else
    {
        qCritical("Error: Failed to load TSKBM device plugin: '%s'",
                  qUtf8Printable(klub_cfg.TSKBM_device_plugin));

        return false;
    }

    return true;
}

void KLUB::step(double t, double dt)
{
    this->dt = dt;

    safety_timer->step(t, dt);
    failure_EPK_timer->step(t, dt);
    beepTimer->step(t, dt);
    V_yellow_reduce_timer->step(t, dt);
    // motion_control_timer->step(t, dt);

    TSKBM->step(t, dt);

    calc_speed();
    calc_acceleration(t, dt);

    SafetyDevice::step(t, dt);
}

void KLUB::setAlsnCode(int code_alsn)
{
    old_code_alsn = this->code_alsn;
    this->code_alsn = code_alsn;
}

void KLUB::setRBstate(bool state)
{
    state_RB = state;
}

void KLUB::setRBSstate(bool state)
{
    state_RBS = state;
}

void KLUB::setRBPstate(bool state)
{
    state_RBP = state;
}

void KLUB::setKeyEPK(bool key_epk)
{
    this->key_epk = key_epk;
}

void KLUB::setFailureEPKState(bool failure_EPK_state)
{
    this->failure_EPK_state = failure_EPK_state;
}

void KLUB::setTractionIsZero(bool traction_is_zero)
{
    this->traction_is_zero = traction_is_zero;
}

bool KLUB::isWorkingTSKBMDevice() const
{
    return is_working_TSKBM_device;
}

bool KLUB::getEPKPowerState() const
{
    return epk_power_state.getState();
}

float KLUB::getLampState(size_t lamp_idx) const
{
    if(lamp_idx < lamps.size())
        return lamps[lamp_idx];

    return 0.0f;
}

float KLUB::getLampNum()
{
    for(int i = 0; i <= GREEN_LAMP1; ++i)
    {
        if(static_cast<bool>(lamps[i]))
            return static_cast<float>(i);
    }

    return 7.0f;
}

double KLUB::getCurrentSpeedLimit() const
{
    return V_permissible;
}

double KLUB::getNextSpeedLimit() const
{
    return V_target;
}

bool KLUB::isModeEK() const
{
    return is_mode_EK;
}

bool KLUB::isDisplayON() const
{
    return is_dislplay_ON;
}

void KLUB::setVoltage(double U_pow)
{
    this->U_pow = U_pow;
}

void KLUB::setMaxVelocity(double v_max)
{
    this->V_green = v_max;
}

double KLUB::getVelocityKmh() const
{
    return V_fact_kmh;
}

int KLUB::getStateVigilanseCheck() const
{
    return vigilanse_check_state;
}

double KLUB::getAcceleration() const
{
    return acceleration;
}

void KLUB::setCoord(dvec3 coord)
{
    this->coord = coord;
}

void KLUB::setRailCoord(double rail_coord)
{
    this->rail_coord = rail_coord;
}

void KLUB::setSpeedMapModule(SpeedMap *device)
{
    speed_map = device;
    speed_map->setDirection(train_dir);
    speed_map->setCurrentSearchDistance(train_length);
}

void KLUB::setTrainLength(double train_length)
{
    if(!is_mode_EK)
        return;

    this->train_length = train_length;

    if(speed_map != nullptr)
        speed_map->setCurrentSearchDistance(train_length);
}

bool KLUB::loadStationsMap(const QString &path)
{
    QFile stations_file(path);

    if(stations_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&stations_file);

        while(!stream.atEnd())
        {
            QString line = stream.readLine();
            QStringList tokens = line.split('\t');

            station_t station;
            station.name = tokens[0];
            station.coord.x = tokens[1].toDouble();
            station.coord.y = tokens[2].toDouble();
            station.coord.z = tokens[3].toDouble();

            stations.push_back(station);
        }

        stations_file.close();

        stations.shrink_to_fit();

        if(stations.empty())
            return false;

        return true;
    }
    else
        return false;
}

bool KLUB::loadTSKBMDevicePlugin(const QString &plugins_dir)
{
#ifdef QT_DEBUG
    klub_cfg.TSKBM_device_plugin += "_d";
#endif

    TSKBM = loadPluginTSKBMDevice(plugins_dir + QDir::separator() + klub_cfg.TSKBM_device_plugin);

    return TSKBM != nullptr ? true : false;
}

QString KLUB::getPluginsDirectory() const
{
    FileSystem &fs = FileSystem::getInstance();
    return QString(fs.getModulesDir().c_str());
}

void KLUB::setTrainDirection(int train_dir)
{
    this->train_dir = train_dir;

    if(speed_map)
        speed_map->setDirection(train_dir);
}

void KLUB::setReversorDirection(int reversor_direction)
{
    this->reversor_direction = reversor_direction;
}

double KLUB::getLimitDistance() const
{
    return limit_dist;
}

double KLUB::getRailCoord() const
{
    return rail_coord / 1000.0;
}

QString KLUB::getCurStation() const
{
    QString tmp = cur_station;
    tmp.resize(STATION_MAX_SYMBOLS, QChar(' '));
    return tmp;
}

int KLUB::getStationIndex() const
{
    return station_idx;
}

void KLUB::setNameTarget(QString &name_target)
{
    this->name_target = name_target;
}

QString KLUB::getScheduleTime() const
{
    return schedule_time;
}

void KLUB::setWheelDiameter(double wheel_diameter)
{
    this->wheel_diameter = wheel_diameter;
}

void KLUB::setOmega(double omega)
{
    this->omega = omega;
}

void KLUB::setDistanceTarget(int distance_target)
{
    this->distance_target = distance_target;
}

int KLUB::getCodeInfoMsg() const
{
    if(key_epk)
        return code_info_msg;
    else
        return CodeInfoMsg::None;
}

void KLUB::preStep(state_vector_t &Y, double t)
{
    (void) Y;
    (void) t;

    if(turnOnDisplay())
    {
        is_red.reset();
        is_dislplay_ON = false;
        cur_station = "";
        return;
    }

    is_dislplay_ON = true;

    // Действия при выключенном ЭПК
    if(!key_epk)
    {
        EPKTurnedOff();
        return;
    }

    this->soundsProcess();

    stations_process();

    alsn_process(code_alsn);

    checkChangesALSN();

    // Контроль скатывания
    if(rollControl())
    {
        // Срыв ЭПК при скатывании
        failureEPK();
        return;
    }

    // Однократная проверка бдительности при трогании в поездном режиме по сигналам КЖ, К, Б
    if(checkVigStartMoveProhibiting())
    {
        failureEPK();
        is_start_move_prohibiting = false;
    }

    // Контроль движения при тяговом положении контроллера
    // if(!traction_is_zero && !isMove())
    //     startMotionControlTimer();
    // else
    //     stopMotionControlTimer();

    // Контроль превышения допустимой скорости
    if(speed_control())
    {
        // Срыв ЭПК при превышении скорости
        failureEPK();
        return;
    }

    if(isMove())
    {
        // Однократная проверка бдительности при движении и смене сигнала на запрещающий
        if(checkVigChangingALSN())
            failureEPK();

        // Контроль движения назад
        if(isReversorBackward())
        {
            // Срыв ЭПК при движении назад
            failureEPK();
            return;
        }
    }

    // Периодические проверки бдительности
    if(is_working_TSKBM_device)
    {
        // Периодическая проверка бдительности ТСКБМ при работающем устройстве
        stopSafetyTimer();
        TSKBM->startSafetyTimer();
    }
    else
    {
        // Периодические проверки бдительности без ТСКБМ
        TSKBM->stopSafetyTimer();
        periodicVigilanceChecks();
    }

    // Управление срывом цепи удерживающей катушки ЭПК от ТСКБМ
    controlResetEPKPowerState();

    // Проверка РБС
    checkRBS();

    // Проверка РБ
    checkRB();

    infoMsgNoEKMode();

    infoMsgFailureEPK();
}

void KLUB::periodicVigilanceChecks()
{
    // Проверка бдительности по сигналам АЛСН
    switch(code_alsn)
    {
    case ALSN::GREEN:
    {
        trainMovSafetyCheck(klub_cfg.min_green_vig_check_interval, klub_cfg.max_green_vig_check_interval);
        break;
    }

    case ALSN::YELLOW:
    {
        // Периодическая проверка бдительности при движении на жёлтый. Отменяется при работающем ТСКБМ
        if(V_fact_kmh >= klub_cfg.V_yellow)
            startSafetyTimer(klub_cfg.min_vig_check_interval, klub_cfg.max_vig_check_interval);
        else
            if(isMove())
                startSafetyTimer(klub_cfg.min_green_vig_check_interval, klub_cfg.max_green_vig_check_interval);
            else
                stopSafetyTimer();

        break;
    }

    case ALSN::RED_YELLOW:
    {
        // Периодическая проверка бдительности при движении на КЖ с допустимой скоростью. Отменяется при работающем ТСКБМ
        trainMovSafetyCheck(klub_cfg.min_vig_check_interval, klub_cfg.max_vig_check_interval);
        break;
    }

    case ALSN::NO_CODE:
    {
        if((old_code_alsn == ALSN::RED_YELLOW) && isMove())
        {
            trainMovSafetyCheck(klub_cfg.min_vig_check_interval, klub_cfg.max_vig_check_interval);
        }
        else
        {
            if(!is_red.getState())
                // Периодическая проверка бдительности при движении на белый с допустимой скоростью. Отменяется при работающем ТСКБМ
                trainMovSafetyCheck(klub_cfg.min_green_vig_check_interval, klub_cfg.max_green_vig_check_interval);
        }

        break;
    }

    default:
        break;
    }
}

bool KLUB::checkVigStartMoveProhibiting()
{
    if(V_fact_kmh < TRAIN_STOP_EPS_SPEED)
        is_start_move_prohibiting = true;

    return is_start_move_prohibiting && code_alsn < ALSN::YELLOW && isMove() ? true : false;
}

bool KLUB::isALSNChange()
{
    return code_alsn != old_code_alsn ? true : false;
}

bool KLUB::checkVigChangingALSN()
{
    // Проверка бдительности при смене сигнала АЛСН на КЖ и Б
    if(isALSNChange() && code_alsn < ALSN::YELLOW)
        return is_changing_ALSN = true;
    else
        return false;
}

void KLUB::checkRB()
{
    // Проверку бдительности при превышении скорости можно отбить только после снижения к допустимой
    if(state_RB && is_changing_ALSN)
    {
        t_holding_RB += dt;

        if(t_holding_RB >= klub_cfg.RB_hold_interval && t_holding_RB < klub_cfg.RB_accept_holding_time)
            onHoldRB();
    }
    else
        t_holding_RB = 0.0;
}

void KLUB::checkRBS()
{
    // Проверку бдительности при превышении скорости можно отбить только после снижения к допустимой
    if(state_RBS)
    {
        t_holding_RBS += dt;

        if(t_holding_RBS >= klub_cfg.RB_hold_interval && t_holding_RBS < klub_cfg.RB_accept_holding_time)
            onHoldRB();
    }
    else
        t_holding_RBS = 0.0;
}

void KLUB::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    if(!cfg.getInt(secName, "MinVigCheckInterval", klub_cfg.min_vig_check_interval))
    {
        constexpr int DEFAULT_MIN_VIG_CHECK_INTERVAL = 30;

        klub_cfg.min_vig_check_interval = DEFAULT_MIN_VIG_CHECK_INTERVAL;

        qWarning() << "Warning: Minimum vigilance check interval configuration not found."
                   << "Default minimum vigilance check interval:" << DEFAULT_MIN_VIG_CHECK_INTERVAL;
    }

    if(!cfg.getInt(secName, "MaxVigCheckInterval", klub_cfg.max_vig_check_interval))
    {
        constexpr int DEFAULT_MAX_VIG_CHECK_INTERVAL = 40;

        klub_cfg.max_vig_check_interval = DEFAULT_MAX_VIG_CHECK_INTERVAL;

        qWarning() << "Warning: Maximum vigilance check interval configuration not found."
                   << "Default maximum vigilance check interval:" << DEFAULT_MAX_VIG_CHECK_INTERVAL;
    }

    if(!cfg.getInt(secName, "MinGreenVigCheckInterval", klub_cfg.min_green_vig_check_interval))
    {
        constexpr int DEFAULT_MIN_GREEN_VIG_CHECK_INTERVAL = 60;

        klub_cfg.min_green_vig_check_interval = DEFAULT_MIN_GREEN_VIG_CHECK_INTERVAL;

        qWarning() << "Warning: Minimum green vigilance check interval configuration not found."
                   << "Default minimum green vigilance check interval:" << DEFAULT_MIN_GREEN_VIG_CHECK_INTERVAL;
    }

    if(!cfg.getInt(secName, "MaxGreenVigCheckInterval", klub_cfg.max_green_vig_check_interval))
    {
        constexpr int DEFAULT_MAX_GREEN_VIG_CHECK_INTERVAL = 90;

        klub_cfg.max_green_vig_check_interval = DEFAULT_MAX_GREEN_VIG_CHECK_INTERVAL;

        qWarning() << "Warning: Maximum green vigilance check interval configuration not found."
                   << "Default maximum green vigilance check interval:" << DEFAULT_MAX_GREEN_VIG_CHECK_INTERVAL;
    }

    if(!cfg.getInt(secName, "OtherVigCheckInterval", klub_cfg.other_vig_check_interval))
    {
        constexpr int DEFAULT_OTHER_VIG_CHECK_INTERVAL = 90;

        klub_cfg.other_vig_check_interval = DEFAULT_OTHER_VIG_CHECK_INTERVAL;

        qWarning() << "Warning: Vigilance check interval in other situations configuration not found."
                   << "Default vigilance check interval in other situations:" << DEFAULT_OTHER_VIG_CHECK_INTERVAL;
    }

    if(!cfg.getDouble(secName, "FailureEPKInterval", klub_cfg.failure_EPK_interval))
    {
        constexpr double DEFAULT_FAILURE_EPK_INTERVAL = 5.0;

        klub_cfg.failure_EPK_interval = DEFAULT_FAILURE_EPK_INTERVAL;

        qWarning() << "Warning: Failure EPK interval configuration not found."
                   << "Default failure EPK interval:" << DEFAULT_FAILURE_EPK_INTERVAL;
    }

    failure_EPK_timer->setTimeout(klub_cfg.failure_EPK_interval);

    if(!cfg.getDouble(secName, "BeepInterval", klub_cfg.beep_interval))
    {
        constexpr double DEFAULT_BEEP_INTERVAL = 0.55;

        klub_cfg.beep_interval = DEFAULT_BEEP_INTERVAL;

        qWarning() << "Warning: Beep interval configuration not found."
                   << "Default beep interval:" << DEFAULT_BEEP_INTERVAL;
    }

    beepTimer->setTimeout(klub_cfg.beep_interval);

    if(!cfg.getInt(secName, "MotionControlInterval", klub_cfg.motion_control_interval))
    {
        constexpr int DEFAULT_MOTION_CONTROL_INTERVAL = 74;

        klub_cfg.motion_control_interval = DEFAULT_MOTION_CONTROL_INTERVAL;

        qWarning() << "Warning: Motion control interval configuration not found."
                   << "Default motion control interval:" << DEFAULT_MOTION_CONTROL_INTERVAL;
    }

    motion_control_timer->setTimeout(klub_cfg.motion_control_interval);

    if(!cfg.getDouble(secName, "RBHoldInterval", klub_cfg.RB_hold_interval))
    {
        constexpr double DEFAULT_RB_HOLD_INTERVAL = 1.5;

        klub_cfg.RB_hold_interval = DEFAULT_RB_HOLD_INTERVAL;

        qWarning() << "Warning: RB hold interval configuration not found."
                   << "Default RB hold interval:" << DEFAULT_RB_HOLD_INTERVAL;
    }

    if(!cfg.getDouble(secName, "RBAcceptHoldingTime", klub_cfg.RB_accept_holding_time))
    {
        constexpr double DEFAULT_RB_ACCEPT_HOLDING_TIME = 3.0;

        klub_cfg.RB_accept_holding_time = DEFAULT_RB_ACCEPT_HOLDING_TIME;

        qWarning() << "Warning: RB acceptable holding time configuration not found."
                   << "Default RB acceptable holding time:" << DEFAULT_RB_ACCEPT_HOLDING_TIME;
    }

    if(!cfg.getDouble(secName, "Vyellow", klub_cfg.V_yellow))
    {
        constexpr double DEFAULT_V_YELLOW = 60.0;

        klub_cfg.V_yellow = DEFAULT_V_YELLOW;

        qWarning() << "Warning: Permissible speed when the signal is yellow configuration not found."
                   << "Default permissible speed when the signal is yellow:" << DEFAULT_V_YELLOW;
    }

    if(!cfg.getDouble(secName, "Acceleration", klub_cfg.acceleration_slow_down))
    {
        constexpr double DEFAULT_ACCELERATION_SLOW_DOWN = 0.3;

        klub_cfg.acceleration_slow_down = DEFAULT_ACCELERATION_SLOW_DOWN;

        qWarning() << "Warning: Acceleration for Deceleration Curve Calculation configuration not found."
                   << "Default Acceleration:" << DEFAULT_ACCELERATION_SLOW_DOWN;
    }

    if(!cfg.getDouble(secName, "SpeedLimitError", klub_cfg.speed_limit_error))
    {
        constexpr double DEFAULT_SPEED_LIMIT_ERROR = 3.0;

        klub_cfg.speed_limit_error = DEFAULT_SPEED_LIMIT_ERROR;

        qWarning() << "Warning: Speed limit error configuration not found."
                   << "Default Speed limit error:" << DEFAULT_SPEED_LIMIT_ERROR;
    }

    if(!cfg.getBool(secName, "IsWorkingTSKBMDevice", klub_cfg.is_working_TSKBM_device))
    {
        qWarning() << "Warning: Is working TSKBM device configuration not found."
                   << "Default is working TSKBM device: true.";
    }

    is_working_TSKBM_device = klub_cfg.is_working_TSKBM_device;

    if(!cfg.getString(secName, "TSKBMDevicePlugin", klub_cfg.TSKBM_device_plugin))
    {
        qWarning() << "Warning: TSKBM device plugin configuration not found.";
    }
}

float KLUB::getSoundSignal(size_t idx) const
{
    return sound_states[idx].createSoundSignal();
}

void KLUB::alsn_process(int code_alsn)
{
    switch(code_alsn)
    {
    case ALSN::NO_CODE:
    {
        if ((old_code_alsn == ALSN::RED_YELLOW) && isMove())
        {
            setLampState(RED_LAMP);
            is_red.set();
        }
        else
        {
            if (!is_red.getState())
                setLampState(WHITE_LAMP);
        }

        break;
    }

    case ALSN::RED_YELLOW:
        setLampState(RED_YELLOW_LAMP);
        break;

    case ALSN::YELLOW:
        setLampState(YELLOW_LAMP);
        break;

    case ALSN::GREEN:
        setLampState(GREEN_LAMP1);
        break;

    default:
        break;
    }
}

void KLUB::soundsProcess()
{
    if((state_RB && (!state_RB_old)) ||
        (state_RB_old && (!state_RB)))
    {
        sound_states[BUTTON_SOUND].play(true);
    }

    if((state_RBS && (!state_RBS_old)) ||
        (state_RBS_old && (!state_RBS)))
    {
        sound_states[BUTTON_SOUND].play(true);
    }

    if((state_RBP && (!state_RBP_old)) ||
        (state_RBP_old && (!state_RBP)))
    {
        sound_states[BUTTON_SOUND].play(true);
    }

    if(key_epk && !key_epk_old)
    {
        sound_states[ON_SOUND].play(true);
    }

    state_RB_old = state_RB;
    state_RBS_old = state_RBS;
    state_RBP_old = state_RBP;

    key_epk_old = key_epk;
}

void KLUB::checkChangesALSN()
{
    // Однократный предупреждающий звуковой сигнал при любой смене сигнала АЛСН
    // Сброс таймера периодической проверки бдительности
    if(isALSNChange())
    {
        stopSafetyTimer();

        warningBeep();
    }
}

void KLUB::soundDecelerationCurve(double V_permissible_lim)
{
    if(code_alsn > ALSN::RED_YELLOW)
        V_permissible_lim += klub_cfg.speed_limit_error;

    if(V_permissible_lim - V_permissible > EPS_SPEED && !deceleration_curve)
    {
        // Однократный предупреждающий звуковой сигнал при снижении текущего ограничения скорости по кривой
        deceleration_curve = true;
        warningBeep();
    }
    if(V_permissible_lim - V_permissible < EPS_SPEED)
        deceleration_curve = false;
}

void KLUB::calc_speed()
{
    V_fact_ms = qAbs(omega * (wheel_diameter / 2));
    V_fact_kmh = V_fact_ms * Physics::kmh;
}

void KLUB::calc_acceleration(double t, double dt)
{
    (void) t;

    if(V_fact_ms < 1e-4)
    {
        acceleration = 0;
        return;
    }

    if(t_diff >= delta_t)
    {
        v_i[v_count] = V_fact_ms;
        t_diff = 0;
        v_count++;
    }

    if(v_count >= v_i.size())
    {
        v_count = 0;
        t_diff = 0;

        acceleration = (3 * v_i[2] - 4 * v_i[1] + v_i[0]) / 2.0 / delta_t;
    }

    t_diff += dt;
}

bool KLUB::speed_control()
{
    if(is_mode_EK)
        // Расчет ограничений скорости по ЭК
        calcSpeedLimits();
    else
        // Расчет ограничений скорости при отсутствии ЭК
        calcSpeedLimitsNoEK();

    int V_fact = qRound(V_fact_kmh);

    if(V_fact < V_permissible - klub_cfg.speed_limit_error)
        stopBeepTimer();
    else
        startBeepTimer();

    // Срыв ЭПК при превышении скорости
    if(V_fact >= V_permissible + 1)
    {
        return true;
    }

    return false;
}

bool KLUB::rollControl()
{
    if(V_fact_kmh < TRAIN_STOP_EPS_SPEED)
        is_roll_control = true;

    if(isMove() && !traction_is_zero)
        is_roll_control = false;

    return is_roll_control && traction_is_zero && isMove() ? true : false;
}

void KLUB::calcSpeedLimits()
{
    speed_limit_t V_permissible_lim;
    speed_limit_t V_target_lim;

    V_permissible_lim.value = speed_map->getCurrentLimit();

    V_target_lim.value = speed_map->getNextLimit();
    V_target_lim.coord = speed_map->getNextLimitDistance();

    limitsALSN(V_permissible_lim, V_target_lim);

    if(V_permissible_lim.value > V_green)
    {
        V_permissible_lim.value = V_green;
    }

    double v_lim = V_green;

    limit_dist = V_target_lim.coord;

    if(V_permissible_lim.value > V_target_lim.value)
    {
        v_lim = sqrt(pow(V_target_lim.value / Physics::kmh, 2) +
                     2 * klub_cfg.acceleration_slow_down * limit_dist) * Physics::kmh;
    }

    if(code_alsn == ALSN::GREEN)
        V_target = V_target_lim.value + klub_cfg.speed_limit_error;
    else
        V_target = V_target_lim.value;

    if(code_alsn < ALSN::YELLOW)
        V_permissible = min(v_lim, V_permissible_lim.value);
    else
        V_permissible = min(v_lim + klub_cfg.speed_limit_error,
                            V_permissible_lim.value + klub_cfg.speed_limit_error);

    soundDecelerationCurve(V_permissible_lim.value);
}

void KLUB::calcSpeedLimitsNoEK()
{
    switch(code_alsn)
    {
    case ALSN::GREEN:
    {
        V_permissible = V_target = V_green + klub_cfg.speed_limit_error;
        stopVYellowReduceTimer();
        break;
    }

    case ALSN::YELLOW:
    {
        if(isALSNChange())
            V_permissible = V_green + klub_cfg.speed_limit_error;

        V_target = klub_cfg.V_yellow;

        if(isMove() && V_permissible > V_target + 1.0)
            startVYellowReduceTimer();
        else
            stopVYellowReduceTimer();

        break;
    }

    case ALSN::RED_YELLOW:
    {
        // Максимально допустимая скорость движения на участке - допустимая скорость движения при жёлтом сигнале
        V_permissible = klub_cfg.V_yellow;
        V_target = 0.0;
        stopVYellowReduceTimer();
        break;
    }

    case ALSN::NO_CODE:
    {
        if((old_code_alsn == ALSN::RED_YELLOW) && isMove())
        {
            // Срыв ЭПК при движении на красный
            V_permissible = V_target = 0.0;
            stopVYellowReduceTimer();
        }
        else
        {
            if(!is_red.getState())
            {
                // Срыв ЭПК при движении > 40 км/ч на белый
                V_permissible = V_target = 40.0;
                stopVYellowReduceTimer();
            }
        }

        break;
    }

    default:
        stopVYellowReduceTimer();
        break;
    }
}

void KLUB::limitsALSN(speed_limit_t &V_permissible_lim, speed_limit_t &V_target_lim)
{
    switch(code_alsn)
    {
    case ALSN::GREEN:
        break;

    case ALSN::YELLOW:
    {
        V_target_lim.coord = distance_target;

        if(V_target_lim.value > klub_cfg.V_yellow)
            V_target_lim.value = klub_cfg.V_yellow;

        break;
    }

    case ALSN::RED_YELLOW:
    {
        // Максимально допустимая скорость движения на участке - допустимая скорость движения при жёлтом сигнале
        if(V_permissible_lim.value > klub_cfg.V_yellow)
            V_permissible_lim.value = klub_cfg.V_yellow;

        V_target_lim.coord = distance_target;
        V_target_lim.value = 0.0;
        break;
    }

    case ALSN::NO_CODE:
    {
        if((old_code_alsn == ALSN::RED_YELLOW) && isMove())
        {
            // Срыв ЭПК при движении на красный
            V_permissible_lim.coord = V_target_lim.coord = distance_target;
            V_permissible_lim.value = V_target_lim.value = 0.0;
        }
        else
        {
            if(!is_red.getState())
            {
                // Срыв ЭПК при движении > 40 км/ч на белый
                V_target_lim.coord = distance_target;
                V_target_lim.value = 40.0;
            }
        }

        break;
    }

    default:
        break;
    }
}

void KLUB::stations_process()
{
    station_idx = -1;

    if(stations.empty())
        return;

    double min_distance = STATION_SEARCH_RADIUS;

    for(size_t i = 0; i < stations.size(); ++i)
    {
        double distance = length(coord - stations[i].coord);
        if(min_distance > distance)
        {
            min_distance = distance;
            station_idx = i;
        }
    }
    if(station_idx >= 0)
        cur_station = stations[station_idx].name;
    else
        cur_station = "";
}

int KLUB::getSafetyTimerInterval(int min, int max) const
{
    if(min < max)
        return QRandomGenerator::global()->bounded(min, max);
    else
        return max;
}

void KLUB::trainMovSafetyCheck(int min, int max)
{
    isMove() ? startSafetyTimer(min, max) : stopSafetyTimer();
}

bool KLUB::isReversorBackward() const
{
    return reversor_direction == ReversorDirection::Backward ? true : false;
}

bool KLUB::isMove()
{
    return V_fact_kmh > SPEED_START_MOVEMENT ? true : false;
}

void KLUB::failureEPK()
{
    epk_power_state.reset();
    vigilanse_check_state = SignalVigilanseCheckState::LitRedTriangle;
}

void KLUB::controlResetEPKPowerState()
{
    if(is_TSKBM_failure_EPK && isMove())
    {
        vigilanse_check_state = SignalVigilanseCheckState::LitRedTriangle;
        epk_power_state.reset();
    }

    if(is_TSKBM_failure_EPK && !isMove())
        vigilanse_check_state = SignalVigilanseCheckState::FlashingRedTriangle;
}

void KLUB::setEPKPowerState()
{
    epk_power_state.set();
    is_TSKBM_failure_EPK = false;
    stopFailureEPKTimer();
    TSKBM->stopSafetyTimer();
    vigilanse_check_state = SignalVigilanseCheckState::NoneIndication;
}

void KLUB::startFailureEPKTimer() const
{
    if(!failure_EPK_timer->isStarted())
        failure_EPK_timer->start();
}

void KLUB::stopFailureEPKTimer() const
{
    if(failure_EPK_timer->isStarted())
        failure_EPK_timer->stop();
}

void KLUB::startSafetyTimer(int min, int max) const
{
    if(!safety_timer->isStarted())
    {
        safety_timer->setTimeout(getSafetyTimerInterval(min, max));
        safety_timer->start();
    }
}

void KLUB::startSafetyTimer(int interval) const
{
    if(!safety_timer->isStarted())
    {
        safety_timer->setTimeout(interval);
        safety_timer->start();
    }
}

void KLUB::stopSafetyTimer() const
{
    if(safety_timer->isStarted())
        safety_timer->stop();
}

void KLUB::startBeepTimer() const
{
    if(!beepTimer->isStarted())
        beepTimer->start();
}

void KLUB::stopBeepTimer() const
{
    if(beepTimer->isStarted())
        beepTimer->stop();
}

void KLUB::startVYellowReduceTimer() const
{
    if(!V_yellow_reduce_timer->isStarted())
        V_yellow_reduce_timer->start();
}

void KLUB::stopVYellowReduceTimer() const
{
    if(V_yellow_reduce_timer->isStarted())
        V_yellow_reduce_timer->stop();
}

void KLUB::startMotionControlTimer() const
{
    if(!motion_control_timer->isStarted())
        motion_control_timer->start();
}

void KLUB::stopMotionControlTimer() const
{
    if(motion_control_timer->isStarted())
        motion_control_timer->stop();
}

void KLUB::EPKTurnedOff()
{
    key_epk_old = false;

    cur_station = "";

    epk_power_state.set();

    is_red.reset();

    vigilanse_check_state = SignalVigilanseCheckState::NoneIndication;

    stopSafetyTimer();

    stopFailureEPKTimer();

    stopBeepTimer();

    stopVYellowReduceTimer();

    TSKBM->stopSafetyTimer();
}

bool KLUB::turnOnDisplay()
{
    return static_cast<bool>(hs_n(U_pow - 0.95 * U_nom)) ? true : false;
}

void KLUB::warningBeep()
{
    sound_states[ON_SOUND].play(true);
}

void KLUB::infoMsgNoEKMode()
{
    if(!is_mode_EK)
        code_info_msg = CodeInfoMsg::NoEKMode;
    else
        code_info_msg = CodeInfoMsg::None;
}

void KLUB::infoMsgFailureEPK()
{
    if(failure_EPK_state)
        code_info_msg = CodeInfoMsg::FailureEPK;
}

void KLUB::setLampState(size_t lamp_idx, bool state, bool clear_state)
{
    if (lamp_idx >= lamps.size())
        return;

    if (clear_state)
        std::fill(lamps.begin(), lamps.end(), 0.0);

    lamps[lamp_idx] = static_cast<float>(state);
}

void KLUB::onSafetyTimer()
{
    warningBeep();

    vigilanse_check_state = SignalVigilanseCheckState::FlashingRedTriangle;

    startFailureEPKTimer();

    stopSafetyTimer();
}

void KLUB::onFailureEPKTimer()
{
    failureEPK();

    stopFailureEPKTimer();
}

void KLUB::onTSKBMVigilanceCheck()
{
    warningBeep();

    vigilanse_check_state = SignalVigilanseCheckState::LitYellowTriangle;
}

void KLUB::onTSKBMfailureEPK()
{
    is_TSKBM_failure_EPK = true;
}

void KLUB::onVYellowReduceTimer()
{
    V_permissible -= klub_cfg.acceleration_slow_down * Physics::kmh;
}

void KLUB::onMotionControlTimer()
{
    failureEPK();
}

void KLUB::onHoldRB()
{
    setEPKPowerState();
    is_changing_ALSN = false;
}

void KLUB::onBeepTimer()
{
    sound_states[OVER_SPEED].play(true);
}

GET_SAFETY_DEVICE(KLUB)
