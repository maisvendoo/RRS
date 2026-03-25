#include    <autopilot.h>
#include    <QLibrary>
#include    <Journal.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Autopilot *loadAutopilot(QString lib_path)
{
    Autopilot *autopilot = nullptr;

    QLibrary lib(lib_path);

    if (lib.load())
    {
        GetAutopilot getAutopilot = reinterpret_cast<GetAutopilot>(lib.resolve("getAutopilot"));

        if (getAutopilot)
        {
            autopilot = getAutopilot();
        }
    }

    return autopilot;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::step(double t, double dt)
{
    time = t;

    /*if (!is_active)
    {
        return;
    }*/

    velocity_control(t, dt);

    vigilance_control(t, dt);

    doors_control(t, dt);

    accel_meter->setVelocity(feedback->v_cur);
    accel_meter->step(t, dt);

    rb_timer->step(t, dt);
    sand_timer->step(t, dt);
    sound_signal_timer->step(t, dt);

    Device::step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString Autopilot::getDbgMsg()
{
    QString msg = "";

    if (is_active)
    {
        msg =  " | АВТОВЕДЕНИЕ";
    }
    else
    {
        msg =  " | СОВЕТЧИК";
    }

    msg += QString(" | Vтек.: %1 км/ч | Vзад.: %2 км/ч | Уск.: %3 м/с2")
        .arg(feedback->v_cur, 4, 'f', 1)
        .arg(v_ref, 4, 'f', 1)
        .arg(accel_meter->value(), 6, 'f', 2);

    if (is_timetable_ready)
    {
        msg += QString(" | Цель: %1 | дист.: %2 | Приб.: %3 | Отпр.: %4 | Факт. приб.: %6 | Факт. отпр.: %7 | Время хода: %8 | Ск. гр.: %9")
                   .arg(timetable.stations[target_station_idx].name)
                   .arg(target_station_dist, 7, 'f', 1)
                   .arg(timetable.getStation(target_station_idx).arr_time, 5)
                   .arg(timetable.getStation(target_station_idx).dep_time, 5)
                   .arg(timetable.getStation(target_station_idx).fact_arr_time, 5)
                   .arg(timetable.getStation(target_station_idx - 1).fact_dep_time, 5)
                   .arg(delta_t, 10, 'f', 1)
                   .arg(v_tt_ref, 4, 'f', 1);
    }

    return msg;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::vigilance_control(double t, double dt)
{
    if (feedback == nullptr)
    {
        return;
    }

    // Есть сигнал контроля бдительности
    if (feedback->is_vigilance_control)
    {
        if (!rb_timer->isStarted())
        {
            // Жмем РБ
            press_RB();
            // Запускаем таймер
            rb_timer->start();
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::velocity_control(double t, double dt)
{
    if (feedback == nullptr)
        return;

    // Счисление пути
    calcTargetDistance();

    // Обработка графика
    checkTimetable(t, dt);

    // Скорость, заданная по графику
    v_ref = calcTimetableVelocity(t, dt, target_station_dist);

    if (v_ref < 0.1)
    {
        int a = 0;
    }

    // Выбираем минимум между текущим ограничением и конструкционной скоростью
    v_ref = min(calcCurrentSpeedLimit(t, dt), v_ref);

    if (v_ref < 0.1)
    {
        int a = 0;
    }

    // Рассчитываем скорость по тормозной кривой до следующего ограничения
    // (если оно больше, ну и пусть :))) )
    v_ref = min(v_ref, calcBrakeCurveSpeed(feedback->v_lim_next, feedback->limit_dist));

    if (v_ref < 0.1)
    {
        int a = 0;
    }

    // Расчитываем скорость по тормозной кривой до ближайшего сигнала
    v_ref = min(v_ref, calcAlsnSpeed(feedback->alsn_code, feedback->signal_dist, v_target));

    if (v_ref < 0.1)
    {
        int a = 0;
    }

    // Если разрешено отправление по графику
    if (is_departure_allowed)
    {
        // Действуем в соответсвии с АЛСН
        //is_motion_allowed = is_alsn_motion_allowed;
        AllowMotion(is_alsn_motion_allowed);
    }

    // Минимальная целевая скорость (для предсказания тормозного пути)
    v_target = min(feedback->v_lim_next, v_target);

    if (feedback->v_lim_next < feedback->v_lim)
        dist_target = min(feedback->v_lim_next, feedback->signal_dist);
    else
        dist_target = feedback->signal_dist;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "V_constr", v_constr);
    cfg.getDouble(secName, "BrakeAccel", a_brake_ref);
    cfg.getDouble(secName, "RefLength", ref_length);
    cfg.getDouble(secName, "RefMass", ref_mass);

    ref_mass = ref_mass * 1000.0;

    cfg.getDouble(secName, "LeadDistance_RY", lead_dist_RY);
    cfg.getDouble(secName, "LeadDistance_Y", lead_dist_Y);
    cfg.getDouble(secName, "SpeedLimit_RY", v_lim_RY);
    cfg.getDouble(secName, "SpeedDisableRelease", v_disable_release);
    cfg.getDouble(secName, "dVTractionOff", dV_traction_off);

    cfg.getDouble(secName, "ArrivalDistEps", arrival_dist_eps);
    cfg.getDouble(secName, "ArrivalBrakeDist", arrival_brake_dist);
    cfg.getDouble(secName, "StopBrakeVelocity", v_stop_brake);

    cfg.getInt(secName, "DelayTimeout", delay_timeout_min);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Autopilot::calcCurrentSpeedLimit(double t, double dt)
{
    double v_lim = feedback->v_lim;

    if (feedback->v_lim < feedback->v_lim_next)
    {
        // Запоминаем текущее, более строгое ограничение
        prev_v_lim = feedback->v_lim;
        // ЗАпоминаем длину хвоста, который надо затянуть
        tail_len = train_length;
    }
    else
    {
        // Затянут весь хвост
        if (tail_len <= 0)
        {
            v_lim = feedback->v_lim;
        }
        else // хвост не затянут, едем по старому ограничению
        {
            v_lim = prev_v_lim;
            tail_len -= feedback->v_cur *dt / Physics::kmh;
        }
    }

    return v_lim;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Autopilot::calcBrakeCurveSpeed(double v_target, double dist)
{
    double vt = v_target / Physics::kmh;

    a_brake = a_brake_ref * ref_mass * train_length / train_mass / ref_length;

    return sqrt(vt * vt + pf(2 * a_brake * dist)) * Physics::kmh;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Autopilot::calcAlsnSpeed(ALSN alsn_code, double signal_dist, double &v_target)
{
    double v_lim = v_constr;

    is_disable_release = false;

    switch (alsn_code)
    {
    case RED_YELLOW:

        v_target = 0.0;

        v_lim = cut(calcBrakeCurveSpeed(v_target, signal_dist - lead_dist_RY), 0.0, v_lim_RY);

        // Запрещаем отпускать тормоза - остановка
        if (feedback->v_cur <= v_disable_release)
        {
            is_disable_release = true;            
        }

        // Если запрещен отпуск и мы остановились - запрещаем движение
        if ( (feedback->v_cur <= v_stop_brake || signal_dist <= lead_dist_RY) && is_disable_release)
        {
            is_alsn_motion_allowed = false;
        }

        break;

    case YELLOW:

        v_target = v_lim_RY;

        v_lim = calcBrakeCurveSpeed(v_target, signal_dist - lead_dist_Y);
        is_alsn_motion_allowed = true;

        break;

    case NO_CODE:

        // Тут, по идее, будет происходить торможение, а проверка бдительности
        // не даст сорвать ЭПК
        v_lim = 40.0; // ????

        is_alsn_motion_allowed = true;

        break;

    case GREEN:

        is_alsn_motion_allowed = true;

        break;
    }

    return v_lim;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Autopilot::calcPredictVelocity(double v_cur, double dist, double accel)
{
    double v0 = v_cur / Physics::kmh;

    double tmp = v0 * v0 + 2 * accel *dist;

    double v_p = 0.0;

    if (tmp < 0)
    {
        return v_p;
    }
    else
    {
        v_p = sqrt(tmp) * Physics::kmh;
    }

    return v_predict_filter.process(v_p);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::slotInitTimeTable()
{
    // Гравик инициализирован - выход
    if (is_timetable_ready)
    {
        return;
    }

    // График пуст - выход
    if (timetable.stations.empty())
    {
        return;
    }

    // Определяем текущее положение ПЕ, где мы установлены
    emit sigGetVehicleTrajPosition(&curr_traj_name, &curr_traj_coord);

    if (curr_traj_name.isEmpty())
    {
        return;
    }

    if (curr_traj_name != prev_traj_name)
    {
        Journal::instance()->debug("TIMETABLE PROCESS: Current trajectory is " + curr_traj_name);
        prev_traj_name = curr_traj_name;
    }

    prev_traj_coord = curr_traj_coord;

    // Индек целевой станции - в самый конец графика
    target_station_idx = timetable.stations.size() - 1;

    // Определяем направление, куда нам проверять маршрут
    bool is_route_exists = false;

    // Проверяем "туда"
    target_dir = 1;

    emit sigIsRouteExists(curr_traj_name,
                          timetable.stations[target_station_idx].target_traj,
                          target_dir,
                          &is_route_exists);

    // Нет маршрута
    if (!is_route_exists)
    {
        // Проверяем обратно
        target_dir = -1;

        emit sigIsRouteExists(curr_traj_name,
                              timetable.stations[target_station_idx].target_traj,
                              target_dir,
                              &is_route_exists);

        // Нет маршрута - из нашего текущего положения не достижима даже конечная станция
        if (!is_route_exists)
        {
            // график хрень
            target_station_idx = -1;
            // Стираем загруженные данные, так как график невыполним
            timetable = autopilot_timetable_t();
            return;
        }
    }

    // Ищем ближайшую станцию по ходу нашего следования, исходя из нашего текущего положения
    while (is_route_exists)
    {
        if (target_station_idx == 0)
        {
            // Дошли до первой станции
            break;
        }

        target_station_idx--;

        emit sigIsRouteExists(curr_traj_name,
                              timetable.stations[target_station_idx].target_traj,
                              target_dir,
                              &is_route_exists);

    }

    if (!is_route_exists)
    {
        target_station_idx++;
    }

    // Запоминаем индекс целевой станции
    timetable.start_station_idx = target_station_idx;
    timetable.curr_station_idx = target_station_idx;

    // Определяем дистанцию до ближайшей станции
    emit sigGetRouteLength(vehicle_idx, curr_traj_name, curr_traj_coord,
                           timetable.stations[target_station_idx].target_traj,
                           timetable.stations[target_station_idx].coord,
                           target_dir, &target_station_dist);

    is_timetable_ready = true;    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::calcTargetDistance()
{
    if (!is_timetable_ready)
    {
        return;
    }

    //prev_traj_coord = curr_traj_coord;
    emit sigGetVehicleTrajPosition(&curr_traj_name, &curr_traj_coord);

    if (curr_traj_name != prev_traj_name)
    {
        Journal::instance()->debug("TIMETABLE PROCESS: Current trajectory is " + curr_traj_name);
        prev_traj_name = curr_traj_name;
    }

    auto st = &timetable.stations[target_station_idx];

    // Если мы оказались на участке приближения
    if (curr_traj_name == st->approach_traj)
    {
        // Включаем запросы на построение маршрута на станцию
        st->build_arr_route_request = true;
    }

    // Маршрут приема надо строить, но он еще не построен
    if (st->build_arr_route_request && !st->is_build_arr_route)
    {
        // Посылаем запрос статуса пути приема
        emit sigGetTrajStateRequest(this->vehicle_idx, curr_traj_name, st->target_traj, target_dir, ARRIVAL_REQUEST);
    }

    // Если нужен сквозной пропуск и он еще не построен
    if (st->arr_time == st->dep_time && !st->removal_traj.isEmpty())
    {
        // Если уже построен маршрут приема, но еще не построен маршрут пропуска
        if (!st->is_build_dep_route && st->is_build_arr_route)
        {
            emit sigGetTrajStateRequest(this->vehicle_idx, st->target_traj, st->removal_traj, target_dir, DEPARTURE_REQUEST);
        }
    }

    // ОПределяем дистанцию до станции-цели
    emit sigGetRouteLength(vehicle_idx, curr_traj_name, curr_traj_coord,
                           timetable.stations[target_station_idx].target_traj,
                           timetable.stations[target_station_idx].coord,
                           target_dir, &target_station_dist);

    timetable.target_station_dist = target_station_dist;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Autopilot::calcTimetableBrakeCurve(double t, double dt, double dist)
{
    double v_ref = v_constr;

    if (timetable.stations.empty())
    {
        return v_constr;
    }

    // Текущая станция
    autopilot_station_t st = timetable.stations[target_station_idx];

    // По текущей станции нет стоянки
    if (st.arr_time == st.dep_time)
    {
        is_departure_allowed = true;
        // тогда порешают другие источники торможения
        return v_ref;
    }

    if (is_departure_allowed)
    {
        return v_ref;
    }

    v_ref = cut(calcBrakeCurveSpeed(0.0, dist), 0.0, v_constr);

    // Запрещаем отпускать тормоза - остановка
    if (feedback->v_cur <= v_disable_release)
    {
        is_disable_release = true;
    }

    // Если запрещен отпуск и мы остановились - запрещаем движение
    if ( (feedback->v_cur <= v_stop_brake || target_station_dist <= arrival_brake_dist)  && is_disable_release)
    {
        is_motion_allowed = false;
    }

    return v_ref;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Autopilot::calcTimetableVelocity(double t, double dt, double dist)
{
    // Нет графика - нехер тут рассчитывать, конструкционная скорость,
    // далее порешают другие ограничения, скорость все равно будет выбрана
    // минимальная из возможных
    if (timetable.stations.empty())
    {
        return v_constr;
    }

    // Разрешено отправление, движемся с скоростью заданной по графику
    // (ограничиваем конструкционной, остальное учтено дальше)
    if (is_departure_allowed)
    {
        return min(v_tt_ref, v_constr);
    }

    // Отправление запрещено - значит идем по программной кривой торможения
    return min(v_tt_ref, calcTimetableBrakeCurve(t, dt, dist));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::checkTimetable(double t, double dt)
{
    if (timetable.stations.empty())
    {
        return;
    }

    // Текущая станция
    auto st = &timetable.stations[target_station_idx];

    // Время прибытия равно времени отправления
    if (st->arr_time == st->dep_time)
    {
        // Разрешаем отправление
        is_departure_allowed = true;
        return;
    }

    // Обработка опоздания
    if ( (st->arr_time != "-") && (st->is_arrival) && (!st->is_delay) )
    {
        double delay = pf(st->fact_arr_time_sec - st->arr_time_sec);
        st->dep_time_sec += delay;
        st->is_delay = true;
    }

    if (t >= st->dep_time_sec)
    {
        is_departure_allowed = true;

        // Если задан участок приближения, строим себе маршрут отправления
        if (!st->removal_traj.isEmpty() && !st->is_build_dep_route)
        {
            // Запрос на проверку свободности маршрута отправления
            emit sigGetTrajStateRequest(this->vehicle_idx, curr_traj_name, st->removal_traj, target_dir, DEPARTURE_REQUEST);
        }
    }
    else
    {
        if (st->target_traj == curr_traj_name)
        {
            is_departure_allowed = false;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::doors_control(double t, double dt)
{
    if (feedback == nullptr)
    {
        return;
    }

    if (timetable.stations.empty())
    {
        return;
    }

    auto st = &timetable.stations[target_station_idx];

    if (st->is_arrival && feedback->v_cur < 0.1)
    {
        if (st->is_left_platform)
        {
            openLeftDoors();
        }

        if (st->is_right_platform)
        {
            openRightDoors();
        }
    }

    if (t >= st->dep_time_sec)
    {
        if (st->is_left_platform)
        {
            closeLeftDoors();
        }

        if (st->is_right_platform)
        {
            closeRightDoors();
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::slotSetBrakeAccel(double a_brake)
{
    this->a_brake = a_brake;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::setTimetable(const autopilot_timetable_t &timetable)
{
    // Запоминаем сам график
    this->timetable = timetable;

    if (timetable.stations.empty())
    {
        return;
    }

    // Сбрасываем флаг готовности графика, чтобы он был переинициализирован
    is_timetable_ready = false;    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::slotVigilanceControl()
{
    release_RB();
    rb_timer->stop();
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::slotSandTimer()
{
    sand_timer->stop();

    sand_OFF();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::slotSoundSignal()
{
    auto ctrl = getControl();

    if (ctrl == nullptr)
    {
        return;
    }

    ctrl->whistle = ctrl->typhoid = false;

    sound_signal_timer->stop();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::slotIncTargetStation(int vehicle_idx)
{
    if (vehicle_idx != this->vehicle_idx)
    {
        return;
    }

    if (timetable.stations.empty())
    {
        return;
    }

    auto st = &timetable.stations[target_station_idx];

    // Отправиться раньше графика решил ты? Путь к темной стороне это...
    if (time < st->dep_time_sec)
    {
        return;
    }

    if (!st->is_departure && allow_inc_target_idx)
    {
        st->is_departure = true;
        st->fact_dep_time = time_str;
        st->dep_delay = static_cast<int>(st->fact_dep_time_sec - st->dep_time_sec) >= delay_timeout_min * 60;

        OnWhistle();

        QString msg = QString("TIMETABLE PROCESS: Departure from: %1 | Dep. time: %2 | Fact. dep.: %3 |")
                          .arg(st->name)                          
                          .arg(st->dep_time, 5)
                          .arg(st->fact_dep_time, 5);        

        Journal::instance()->debug(msg);

        target_station_idx++;

        timetable.curr_station_idx = target_station_idx;

        allow_inc_target_idx = false;

        if (target_station_idx > timetable.stations.size() - 1)
        {
            target_station_idx = timetable.stations.size() - 1;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::slotCalcMiddleVelocity(int vehicle_idx, double target_dist)
{
    // Не нам данные, другому автопилоту
    if (vehicle_idx != this->vehicle_idx)
    {
        return;
    }

    if (target_dist < 0)
    {
        return;
    }

    if (timetable.stations.empty())
    {
        return;
    }

    allow_inc_target_idx = true;

    // Текущая станция
    auto st = &timetable.stations[target_station_idx];

    // Фиксируем факт прибытия
    if ( (target_dist < arrival_dist_eps) && (!st->is_arrival) )
    {
        st->is_arrival = true;
        st->fact_arr_time_sec = time;

        OnWhistle();

        if (st->arr_time == "-")
        {
            st->fact_arr_time = "-";
        }
        else
        {
            st->fact_arr_time = time_str;
        }

        st->arr_delay = static_cast<int>(st->fact_arr_time_sec - st->arr_time_sec) >= delay_timeout_min * 60;

        QString msg = QString("TIMETABLE PROCESS: Arrival to: %1 | Arr. time: %2 | Fact. arr.: %3 |")
                          .arg(st->name)
                          .arg(st->arr_time, 5)
                          .arg(st->fact_arr_time, 5);        

        Journal::instance()->debug(msg);
    }

    // Рассчитываем оставшееся время хода до станции
    delta_t = st->arr_time_sec - time;

    // Уже опоздали - мчим с конструкционной (если позволят)
    if (delta_t < 0)
    {
        v_tt_ref = v_constr;
    }
    else
    {
        // Рассчитываем среднюю перегонную скорость
        v_tt_ref = target_dist * Physics::kmh / (delta_t + TIME_ZERO_EPS);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::slotSetTimeForAutopilot(QString time)
{
    this->time_str = time;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::slotGetTrajState(int vehicle_idx, QString start_traj_name, QString traj_name, int request_type, bool is_route_possible)
{
    // Если данные не наши - выходим
    if (vehicle_idx != this->vehicle_idx)
    {
        return;
    }

    // Если нет графика - выходим
    if (timetable.stations.empty())
    {
        return;
    }

    // Если указанная траектория занята или включена в другой маршрут - выходим
    if (!is_route_possible)
    {
        QString msg = QString("TIMETABLE PROCESS: route from %1 to %2 is't possible")
                          .arg(start_traj_name)
                          .arg(traj_name);

        Journal::instance()->debug(msg);

        return;
    }

    auto st = &timetable.stations[target_station_idx];

    switch (request_type)
    {
    case ARRIVAL_REQUEST:

        Journal::instance()->debug(QString("TIMETABLE PROCESS: try build route from %1 to %2").arg(start_traj_name).arg(traj_name));
        emit sigBuildTrainRoute(start_traj_name, traj_name, target_dir);
        st->is_build_arr_route = true;
        break;

    case DEPARTURE_REQUEST:

        Journal::instance()->debug(QString("TIMETABLE PROCESS: try build route from %1 to %2").arg(start_traj_name).arg(traj_name));
        emit sigBuildTrainRoute(start_traj_name, traj_name, target_dir);
        st->is_build_dep_route = true;
        break;

    case APPROACH_REQUEST:

        Journal::instance()->debug(QString("TIMETABLE PROCESS: try build route from %1 to %2").arg(st->target_traj).arg(traj_name));
        emit sigBuildTrainRoute(st->target_traj, traj_name, target_dir);
        st->is_build_arr_route = st->is_build_dep_route = true;
        break;

    default:

        break;
    }
}
