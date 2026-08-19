//------------------------------------------------------------------------------
//
//      Diesel engine system (тепловая модель дизеля, расход, дымность)
//
//------------------------------------------------------------------------------

#include    "vehicle-diesel.h"

#include    "physics.h"

#include    <CfgReader.h>
#include    <Journal.h>

#include    <algorithm>
#include    <cmath>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DieselEngineSystem::DieselEngineSystem() = default;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DieselEngineSystem::loadConfig(QString cfg_path)
{
    CfgReader cfg;
    if (!cfg.load(cfg_path))
        return;

    const QString sec = "Diesel";

    cfg.getDouble(sec, "NominalPower", nominal_power);
    cfg.getDouble(sec, "IdleRPM", idle_rpm);
    cfg.getDouble(sec, "MaxRPM", max_rpm);
    cfg.getDouble(sec, "FuelCapacity", fuel_capacity);
    cfg.getDouble(sec, "Fuel", fuel);
    cfg.getDouble(sec, "OilCapacity", oil_capacity);
    cfg.getDouble(sec, "Oil", oil);

    cfg.getDouble(sec, "TempWarm", t_warm);
    cfg.getDouble(sec, "TempHigh", t_high);
    cfg.getDouble(sec, "TempOverheat", t_overheat);
    cfg.getDouble(sec, "TempCritical", t_critical);

    cfg.getDouble(sec, "RadiatorRefTemp", radiator_ref_temp);
    cfg.getDouble(sec, "UnwarmedMargin", unwarmed_margin);

    // Снабжение: уровень/ёмкость системы охлаждения
    cfg.getDouble(sec, "CoolantCapacity", coolant_capacity);
    cfg.getDouble(sec, "CoolantAmount", coolant_amount);

    cfg.getDouble(sec, "IdleFuelRate", sfc_idle);

    double value = 0.0;
    if (cfg.getDouble(sec, "SFC0", value))
        sfc_curve[0] = value;
    if (cfg.getDouble(sec, "SFC25", value))
        sfc_curve[1] = value;
    if (cfg.getDouble(sec, "SFC50", value))
        sfc_curve[2] = value;
    if (cfg.getDouble(sec, "SFC75", value))
        sfc_curve[3] = value;
    if (cfg.getDouble(sec, "SFC100", value))
        sfc_curve[4] = value;

    //--- Турбокомпрессор (ТЗ, п.31) ---
    cfg.getDouble(sec, "TurboMaxRpm", turbo_max_rpm);
    turbo_max_rpm = std::max(turbo_max_rpm, 100.0);

    //--- Воздушный фильтр ---
    cfg.getDouble(sec, "FilterDustRate", filter_dust_rate);

    //--- Форсунки ---
    cfg.getDouble(sec, "InjectorWearRate", injector_wear_rate);

    double quality = 0.0;
    if (cfg.getDouble(sec, "FuelQuality", quality))
        fuel_quality = std::min(std::max(quality, 0.5), 1.0);

    //--- Утечки ---
    cfg.getDouble(sec, "LeakProbability", leak_probability);

    //--- Защиты аварийной остановки (ТЗ, п.23) ---
    cfg.getBool(sec, "ProtectionsEnabled", protections_enabled);
    cfg.getDouble(sec, "MinOilPressure", oil_pressure_min);
    cfg.getDouble(sec, "MaxOilTemp", oil_temp_max);
    cfg.getDouble(sec, "OverspeedPercent", overspeed_percent);
    cfg.getDouble(sec, "OverspeedTime", overspeed_time);
    cfg.getDouble(sec, "AntiShutdownTime", anti_shutdown_time);

    //--- Холодный пуск (ТЗ, п.24) ---
    cfg.getDouble(sec, "ColdStartThreshold", cold_start_threshold);
    cfg.getDouble(sec, "ColdStartCrankTime", cold_start_crank_time);
    cfg.getDouble(sec, "ColdStartReliability", cold_start_reliability);

    //--- Вероятностные неисправности (ТЗ, п.31) ---
    cfg.getDouble(sec, "MisfireProbability", misfire_probability);
    cfg.getDouble(sec, "StuckRackProbability", stuck_rack_probability);

    //--- ПСЧ: фиксированный Seed воспроизводит сценарий износа ПЕ, 0 - случайно
    int seed = 0;
    if (cfg.getInt(sec, "Seed", seed) && seed > 0)
        rng.seed(static_cast<std::mt19937::result_type>(seed));

    fuel = std::min(fuel, fuel_capacity);
    oil = std::min(oil, oil_capacity);
    coolant_capacity = std::max(coolant_capacity, 1.0);
    coolant_amount = std::min(std::max(coolant_amount, 0.0), coolant_capacity);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool DieselEngineSystem::isConfigured() const
{
    // Null-паттерн: NominalPower = 0 в конфиге - дизеля на ПЕ нет
    return nominal_power > 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DieselEngineSystem::start()
{
    if (running || cranking)
        return;

    if (fuel <= 0.0)
    {
        Journal::instance()->warning("[DIESEL] Start refused: no fuel");
        return;
    }

    // Anti-shutdown: после срабатывания защиты повторный пуск
    // заблокирован на AntiShutdownTime секунд (ТЗ, п.23)
    if (protection_lock > 0.0)
    {
        Journal::instance()->warning(QString(
            "[DIESEL] Start blocked by protection: %1 s")
            .arg(protection_lock, 0, 'f', 0));
        return;
    }

    // Холодный пуск (ТЗ, п.24): на морозе стартер крутит дольше,
    // успех вероятностен от температуры (состояние АБ упрощённо
    // учтено зависимостью от температуры)
    if (last_ambient < cold_start_threshold)
    {
        cranking = true;
        crank_timer = cold_start_crank_time;
        rpm = 150.0;                    // обороты прокрутки стартером
        smoke = Smoke::Light;
        smoke_color = 3;
        Journal::instance()->info(QString(
            "[DIESEL] Cold start at %1 C: cranking...")
            .arg(last_ambient, 0, 'f', 0));
        return;
    }

    running = true;
    rpm = idle_rpm;

    // Холодный запуск: непрогретый дизель дымит (ТЗ, п.24)
    if (coolant_t < 0.0)
    {
        smoke = Smoke::Heavy;
        smoke_color = 3;   // белый/сизый непрогретый дым
    }

    Journal::instance()->info("[DIESEL] Engine started");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DieselEngineSystem::stop()
{
    running = false;
    cranking = false;
    rpm = 0.0;
    power_kw = 0.0;
    load = 0.0;
    fuel_rate = 0.0;
    load_fuel_rate = 0.0;
    oil_pressure = 0.0;
    oil_rate = 0.0;
    smoke = Smoke::None;
    smoke_color = 0;

    // Сброс таймеров неисправностей и защит
    misfire_timer = 0.0;
    rack_stuck_timer = 0.0;
    low_oil_pressure_timer = 0.0;
    overspeed_timer = 0.0;
    overspeed_warned = false;
    cold_stabilize_time = 0.0;

    // Начало отсчёта выбега турбины (маслоголодание, ТЗ, п.31)
    turbo_stop_time = 0.0;

    Journal::instance()->info("[DIESEL] Engine stopped");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool DieselEngineSystem::isRunning() const
{
    return running;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DieselEngineSystem::setThrottle(double value)
{
    // Залипание рейки: контроллер не отвечает 1-2 с (ТЗ, п.31)
    if (rack_stuck_timer > 0.0)
        return;

    throttle = std::min(std::max(value, 0.0), 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DieselEngineSystem::setLoadDemand(double demand)
{
    load_demand = std::min(std::max(demand, 0.0), 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DieselEngineSystem::step(double dt, double air_temperature, double speed)
{
    last_ambient = air_temperature;

    // Окно блокировки повторного пуска после защиты (ТЗ, п.23)
    if (protection_lock > 0.0)
        protection_lock = std::max(0.0, protection_lock - dt);

    if (!running)
    {
        // Прокрутка стартером при холодном пуске (ТЗ, п.24)
        if (cranking)
            stepCranking(dt);

        // Остывание остановленного дизеля
        coolant_t += (air_temperature - coolant_t) * std::min(1.0, 0.0004 * dt);
        oil_t += (air_temperature - oil_t) * std::min(1.0, 0.0003 * dt);

        // Выбег и маслоголодание турбины после остановки (ТЗ, п.31)
        stepTurboShutdown(dt);
        return;
    }

    if (cold_stabilize_time > 0.0)
        cold_stabilize_time = std::max(0.0, cold_stabilize_time - dt);

    // Таймеры неисправностей (ТЗ, п.31)
    if (misfire_timer > 0.0)
        misfire_timer = std::max(0.0, misfire_timer - dt);
    if (rack_stuck_timer > 0.0)
        rack_stuck_timer = std::max(0.0, rack_stuck_timer - dt);

    //--- Обороты: регулятор ведёт к целевым (ТЗ, п.2) ---
    double target_rpm = idle_rpm + (max_rpm - idle_rpm) * throttle;

    // Изношенные форсунки: неровный холостой ход, пульсация +-3%
    // (ТЗ, п.31). Затухает по мере отхода от холостых оборотов
    pulse_phase += 9.42 * dt;          // ~1.5 Гц
    {
        const double fade = std::min(std::max(
            (1.25 * idle_rpm - rpm) / (0.25 * idle_rpm), 0.0), 1.0);
        target_rpm *= 1.0 + 0.03 * injector_wear *
                std::sin(pulse_phase) * fade;
    }

    // Перегрузка/перегрев просаживают обороты
    const double droop = 1.0 -
            0.25 * std::max(0.0, load_demand - 0.95) -
            0.30 * (1.0 - available_power);

    // После холодного пуска idle-регулятор медленнее: обороты
    // стабилизируются дольше (ТЗ, п.24)
    const double rate = rpm_rate * (cold_stabilize_time > 0.0 ? 0.35 : 1.0);

    rpm += (target_rpm * droop - rpm) * std::min(1.0, rate * dt);

    //--- Турбонаддув с лагом (ТЗ, п.17) ---
    const double target_turbo = throttle * (0.4 + 0.6 * load_demand);
    turbo += (target_turbo - turbo) * std::min(1.0, turbo_rate * dt);
    turbo_rpm = turbo * turbo_max_rpm;

    //--- Фактическая нагрузка: запрос, доступность топлива и турбо ---
    double demand = load_demand;

    // Мощность ограничена доступностью (топливо/перегрев/износ)
    demand *= available_power;

    // Забитый воздушный фильтр душит двигатель: -30% при clog = 1
    demand *= 1.0 - 0.3 * filter_clog;

    // Нагрузка следует за запросом с динамикой (прогрессия турбо)
    const double load_target = demand * (0.55 + 0.45 * turbo);
    load += (load_target - load) * std::min(1.0, 1.5 * dt);

    // Пропуск вспышки: рывок момента на 0.5 с (ТЗ, п.31)
    if (misfire_timer > 0.0)
        load *= 0.3;

    //--- Топливо (ТЗ, п.4): расход от фактической работы ---
    power_kw = nominal_power * load;

    // Удельный расход интерполяцией по нагрузке; износ ЦПГ ухудшает
    // КПД, изношенные форсунки добавляют до +10% (ТЗ, п.31)
    const double wear_penalty = (1.0 + 0.15 * wear) *
            (1.0 + 0.10 * injector_wear);
    const double x = std::min(std::max(load, 0.0), 1.0) * 4.0;
    const size_t i = std::min(static_cast<size_t>(x), size_t{3});
    const double frac = x - static_cast<double>(i);
    const double sfc = (sfc_curve[i] + (sfc_curve[i+1] - sfc_curve[i]) * frac) *
            wear_penalty;

    // кг/ч = кВт * г/(кВт*ч) / 1000; л/ч = кг/ч / 0.85
    // (плотность дизтоплива ~0.85 кг/л)
    const double load_flow_kg_h = power_kw * sfc / 1000.0;
    const double load_flow = load_flow_kg_h / fuel_density;
    fuel_rate = sfc_idle * (rpm / idle_rpm) * 0.6 + load_flow;
    load_fuel_rate = load_flow;

    // Утечка топлива: л/ч = leak * max_flow (ТЗ, п.31)
    fuel -= (fuel_rate + fuel_leak * fuel_leak_max_flow) * dt / 3600.0;
    fuel_total += fuel_rate * dt / 3600.0;

    if (fuel <= 0.0)
    {
        fuel = 0.0;
        stop();
        Journal::instance()->critical("[DIESEL] Fuel EMPTY - engine stalled");
        return;
    }

    // Низкий уровень топлива: падение давления, нестабильная работа
    if (fuel < 0.05 * fuel_capacity)
    {
        available_power = std::max(0.5, fuel / (0.05 * fuel_capacity));
    }
    else
    {
        available_power = 1.0;
    }

    // Низкий уровень ОЖ (ТЗ "Снабжение локомотива"): радиатор почти не
    // работает (перегрев в stepThermal), мощность форсированно урезана
    if (getCoolantLevel() < 0.1)
    {
        available_power = std::min(available_power, 0.5);

        if (!coolant_low_warned)
        {
            coolant_low_warned = true;
            Journal::instance()->critical(QString(
                "[DIESEL] COOLANT LEVEL LOW: %1% - power limited to 50%")
                .arg(getCoolantLevel() * 100.0, 0, 'f', 0));
        }
    }
    else
    {
        coolant_low_warned = false;
    }

    //--- Масло: давление от оборотов, температура, расход (ТЗ, п.6, 10) ---
    oil_pressure = 0.05 + 0.45 * (rpm / max_rpm) * std::max(0.3, 1.0 - wear / 2.0);

    // Расход масла: малый у исправного, растёт с износом; утечка
    // добавляет leak * max_flow (масло на асфальте - только журнал,
    // ТЗ, п.31)
    oil_rate = 0.02 + 0.5 * wear + 0.3 * wear * wear * load * load +
            oil_leak * oil_leak_max_flow;
    oil = std::max(0.0, oil - oil_rate * dt / 3600.0);

    if (oil_leak > 0.3 && !oil_spill_warned)
    {
        oil_spill_warned = true;
        Journal::instance()->warning(QString(
            "[DIESEL] Oil leak: %1 l/h - масло на пути (датчик)")
            .arg(oil_leak * oil_leak_max_flow, 0, 'f', 0));
    }

    //--- Утечка ОЖ: уровень падает (перегрев придёт через stepThermal) ---
    coolant_amount = std::max(0.0, coolant_amount -
        coolant_leak * coolant_leak_max_flow * dt / 3600.0);

    //--- Тепловая модель ---
    stepThermal(dt, power_kw, air_temperature, speed);

    //--- Ограничения от температуры (ТЗ, п.9) ---
    const TempZone zone = getTemperatureZone();

    if (zone == TempZone::Overheat)
    {
        available_power = std::min(available_power, 0.7);

        Journal::instance()->warning(QString(
            "[DIESEL] OVERHEAT: coolant %1 C, power limited")
            .arg(coolant_t, 0, 'f', 1));
    }
    else if (zone == TempZone::Critical)
    {
        stop();
        Journal::instance()->critical(QString(
            "[DIESEL] CRITICAL overheat %1 C - engine shutdown")
            .arg(coolant_t, 0, 'f', 1));
    }

    // Износ: нагрузка + высокие температуры (ТЗ, п.21)
    if (zone >= TempZone::High)
    {
        wear = std::min(1.0, wear + 0.00003 * dt);
    }

    wear = std::min(1.0, wear + 0.000002 * load * load * dt);

    //--- Воздушный фильтр: засорение от моточасов/пыли (ТЗ, п.31) ---
    // Пыльная погода позднее поднимет FilterDustRate извне
    filter_clog = std::min(1.0, filter_clog + filter_dust_rate *
        (0.3 + 0.7 * rpm / max_rpm) * dt / 3600.0);

    if (filter_clog > 0.8 && !filter_warned)
    {
        filter_warned = true;
        Journal::instance()->warning(QString(
            "[DIESEL] Air filter clogged %1% - replaceFilter() required")
            .arg(filter_clog * 100.0, 0, 'f', 0));
    }

    //--- Форсунки: износ от моточасов под нагрузкой и плохого топлива ---
    injector_wear = std::min(1.0, injector_wear + injector_wear_rate *
        (1.0 + (1.0 - fuel_quality)) * (0.4 + 0.6 * load) * dt / 3600.0);

    //--- Турбокомпрессор: температура (нагрев от выхлопа,
    //--- охлаждение маслом и набегающим воздухом), износ ---
    {
        // Равновесная температура крыльчатки
        const double turbo_target = air_temperature +
                (exhaust_t - air_temperature) * (0.55 + 0.45 * turbo) -
                40.0 * std::min(oil_pressure / 0.3, 1.0) -
                15.0 * std::min(speed / 20.0, 1.0);
        turbo_t += (turbo_target - turbo_t) * std::min(1.0, 0.05 * dt);
        turbo_t = std::max(turbo_t, air_temperature);

        // Заброс оборотов турбины выше предела: ускоренный износ
        if (turbo_rpm > 1.05 * turbo_max_rpm)
        {
            turbo_wear = std::min(1.0, turbo_wear + 0.001 * dt *
                (turbo_rpm / turbo_max_rpm - 1.05));
        }

        // Перегрев крыльчатки
        if (turbo_t > 750.0)
        {
            turbo_wear = std::min(1.0, turbo_wear +
                0.0002 * (turbo_t - 750.0) / 100.0 * dt);
        }
    }

    //--- Вероятностные утечки при высоком износе (ТЗ, п.31) ---
    if (wear > 0.5)
    {
        const double p = leak_probability * (wear - 0.5) * 2.0 * dt;

        if (rand01() < p)
        {
            const double which = rand01();
            if (which < 1.0 / 3.0 && fuel_leak <= 0.0)
            {
                fuel_leak = 0.05;
                Journal::instance()->critical(
                    "[DIESEL] FUEL LEAK detected");
            }
            else if (which < 2.0 / 3.0 && oil_leak <= 0.0)
            {
                oil_leak = 0.05;
                Journal::instance()->critical(
                    "[DIESEL] OIL LEAK detected");
            }
            else if (coolant_leak <= 0.0)
            {
                coolant_leak = 0.05;
                Journal::instance()->critical(
                    "[DIESEL] COOLANT LEAK detected");
            }
        }

        // Рост уровня существующих утечек
        fuel_leak = std::min(1.0, fuel_leak +
            0.01 * dt * (0.5 + wear));
        oil_leak = std::min(1.0, oil_leak +
            0.01 * dt * (0.5 + wear));
        coolant_leak = std::min(1.0, coolant_leak +
            0.01 * dt * (0.5 + wear));
    }

    //--- Редкие неисправности при износе > 0.7 (ТЗ, п.31) ---
    if (wear > 0.7)
    {
        const double k = (wear - 0.7) / 0.3;

        if (misfire_timer <= 0.0 && rack_stuck_timer <= 0.0)
        {
            if (rand01() < misfire_probability * k * dt)
            {
                misfire_timer = 0.5;
                Journal::instance()->warning(
                    "[DIESEL] Misfire - пропуск вспышки");
            }
            else if (rand01() < stuck_rack_probability * k * dt)
            {
                rack_stuck_timer = 1.0 + rand01();
                Journal::instance()->warning(QString(
                    "[DIESEL] Rack stuck %1 s - контроллер не отвечает")
                    .arg(rack_stuck_timer, 0, 'f', 1));
            }
        }
    }

    //--- Дымность (ТЗ, п.12-16) ---
    smoke = Smoke::None;
    smoke_color = 0;

    // Чёрный: резкое богатое включение нагрузки / изношенные форсунки
    if (load_demand - load > 0.3 || (wear > 0.7 && load > 0.5) ||
        injector_wear > 0.6)
    {
        smoke = injector_wear > 0.85 ? Smoke::Heavy : Smoke::Medium;
        smoke_color = 1;
    }
    else if (load > 0.85)
    {
        smoke = Smoke::Light;
        smoke_color = 1;
    }

    // Синий: горение масла (износ ЦПГ)
    if (wear > 0.5 && oil_rate > 0.3)
    {
        smoke = Smoke::Medium;
        smoke_color = 2;
    }

    // Белый: непрогретый дизель
    if (coolant_t < t_warm - unwarmed_margin)
    {
        smoke = Smoke::Light;
        smoke_color = 3;
    }

    if (wear > 0.9)
    {
        smoke = Smoke::Heavy;
        smoke_color = 2;
    }

    // Пропуск вспышки: клуб чёрного дыма (ТЗ, п.31)
    if (misfire_timer > 0.0)
    {
        smoke = Smoke::Heavy;
        smoke_color = 1;
    }

    //--- Защиты аварийной остановки (ТЗ, п.23) ---
    if (protections_enabled)
    {
        // Низкое давление масла при вращении (подтверждение 1 с -
        // переходный процесс пуска)
        if (oil_pressure < oil_pressure_min)
        {
            low_oil_pressure_timer += dt;

            if (low_oil_pressure_timer > 1.0)
            {
                tripProtection(QString("низкое давление масла %1 МПа")
                    .arg(oil_pressure, 0, 'f', 2));
                return;
            }
        }
        else
        {
            low_oil_pressure_timer = 0.0;
        }

        // Разнос: обороты выше 115% номинала; после OverspeedTime
        // секунд - разрушение дизеля (Engine damage 1.0 + stop)
        if (rpm > max_rpm * overspeed_percent / 100.0)
        {
            overspeed_timer += dt;

            if (!overspeed_warned)
            {
                overspeed_warned = true;
                Journal::instance()->critical(QString(
                    "[DIESEL] OVERSPEED %1 rpm (> %2%) - protection: "
                    "%3 s to destruction")
                    .arg(rpm, 0, 'f', 0)
                    .arg(overspeed_percent, 0, 'f', 0)
                    .arg(overspeed_time, 0, 'f', 0));
            }

            if (overspeed_timer >= overspeed_time)
            {
                wear = 1.0;
                tripProtection("разнос - разрушение дизеля");
                return;
            }
        }
        else
        {
            overspeed_timer = 0.0;
            overspeed_warned = false;
        }

        // Температура масла выше предела
        if (oil_t > oil_temp_max)
        {
            tripProtection(QString("температура масла %1 C")
                .arg(oil_t, 0, 'f', 1));
            return;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DieselEngineSystem::stepThermal(double dt, double power_kw,
                                     double air_temperature, double speed)
{
    //--- ОЖ: тепловыделение ~ мощности; охлаждение радиатором ---
    // В ОЖ уходит ~1.9 кВт тепловой на 1 кВт механической мощности
    const double heat = power_kw * 1.9;

    // Термостат: радиатор включается выше t_warm
    const double thermostat = std::min(std::max(
        (coolant_t - t_warm) / 10.0, 0.0), 1.0);

    // Уровень ОЖ: низкий уровень резко снижает отвод тепла радиатором
    // (0.2 - остаточная конвекция при почти пустой системе)
    const double coolant_level = std::min(std::max(
        coolant_amount / coolant_capacity, 0.0), 1.0);

    // Радиатор откалиброван удерживать равновесие ~85 C при полной
    // мощности и воздухе radiator_ref_temp; обдув движением помогает
    const double cooling = (300.0 + 2600.0 * thermostat) *
            (0.2 + 0.8 * coolant_level) *
            (1.0 + 0.25 * std::min(speed / 20.0, 1.0)) *
            (1.0 - 0.4 * std::max(0.0, (air_temperature - radiator_ref_temp)) / 40.0);

    const double loss = cooling *
            std::max(coolant_t - air_temperature, 0.0) / 60.0;

    // Теплоёмкость системы ОЖ дизеля, кДж/К
    const double C_coolant = 2500.0;

    coolant_t += (heat - loss) * dt / C_coolant;
    coolant_t = std::max(coolant_t, air_temperature - 5.0);

    //--- Масло: греется от нагрузки, маслоохладитель на ОЖ ---
    const double oil_heat = power_kw * 0.4;
    oil_t += (oil_heat - 30.0 * (oil_t - coolant_t)) * dt / 300.0;
    oil_t = std::max(oil_t, coolant_t - 5.0);

    //--- Выхлоп: от нагрузки и подачи топлива ---
    const double exhaust_target = 250.0 + 450.0 * load + 100.0 * wear;
    exhaust_t += (exhaust_target - exhaust_t) * std::min(1.0, 0.25 * dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DieselEngineSystem::stepCranking(double dt)
{
    crank_timer -= dt;
    rpm = 150.0;
    smoke = Smoke::Light;
    smoke_color = 3;        // белый дым прокрутки на морозе

    // Стартер впустую тянет немного топлива
    fuel = std::max(0.0, fuel - 2.0 * dt / 3600.0);

    if (crank_timer > 0.0)
        return;

    cranking = false;

    if (rand01() < coldStartProbability())
    {
        running = true;
        rpm = idle_rpm;
        // Обороты стабилизируются дольше: idle-регулятор медленнее
        cold_stabilize_time = 20.0;
        Journal::instance()->info("[DIESEL] Cold start succeeded");
    }
    else
    {
        rpm = 0.0;
        smoke = Smoke::None;
        smoke_color = 0;
        Journal::instance()->warning(
            "[DIESEL] Cold start FAILED - повторите пуск");
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DieselEngineSystem::stepTurboShutdown(double dt)
{
    // Выбег ротора после остановки: инерция дольше, чем наброс наддува
    turbo = std::max(0.0, turbo - turbo_rate * 0.3 * dt);
    turbo_rpm = turbo * turbo_max_rpm;

    // Остывание крыльчатки воздухом
    turbo_t += (last_ambient - turbo_t) * std::min(1.0, 0.01 * dt);
    turbo_t = std::max(turbo_t, last_ambient);

    // Масляное голодание: первые 30 с после остановки ротор ещё
    // вращается без подпора масла - износ ×3 (ТЗ, п.31)
    turbo_stop_time += dt;

    if (turbo_stop_time < 30.0 && turbo_rpm > 0.1 * turbo_max_rpm)
    {
        turbo_wear = std::min(1.0,
            turbo_wear + 0.0003 * 3.0 * dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DieselEngineSystem::tripProtection(const QString& reason)
{
    protection_lock = anti_shutdown_time;

    Journal::instance()->critical(QString(
        "[DIESEL] EMERGENCY SHUTDOWN: %1 (повторный пуск через %2 с)")
        .arg(reason)
        .arg(anti_shutdown_time, 0, 'f', 0));

    stop();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DieselEngineSystem::coldStartProbability() const
{
    // Чем холоднее - тем хуже: от ~ColdStartReliability на пороге
    // до ~0.2 при температуре на 20 градусов ниже порога
    const double t = std::min(last_ambient, cold_start_threshold);
    const double span = std::max(cold_start_threshold + 30.0, 1.0);
    const double k = std::min(std::max((t + 30.0) / span, 0.0), 1.0);

    return std::min(0.95, std::max(0.2, cold_start_reliability * k));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DieselEngineSystem::rand01()
{
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DieselEngineSystem::getRPM() const
{
    return rpm;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DieselEngineSystem::getPower() const
{
    return power_kw;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DieselEngineSystem::getAvailablePowerFactor() const
{
    return available_power;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DieselEngineSystem::getCoolantTemperature() const
{
    return coolant_t;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DieselEngineSystem::TempZone DieselEngineSystem::getTemperatureZone() const
{
    if (coolant_t < t_warm)
        return TempZone::Normal;
    if (coolant_t < t_high)
        return TempZone::Warm;
    if (coolant_t < t_overheat)
        return TempZone::High;
    if (coolant_t < t_critical)
        return TempZone::Overheat;

    return TempZone::Critical;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DieselEngineSystem::getOilTemperature() const
{
    return oil_t;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DieselEngineSystem::getOilPressure() const
{
    return oil_pressure;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DieselEngineSystem::getExhaustTemperature() const
{
    return exhaust_t;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DieselEngineSystem::getFuelLevel() const
{
    return fuel / std::max(fuel_capacity, 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DieselEngineSystem::getFuelRate() const
{
    return fuel_rate;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DieselEngineSystem::getTotalFuelConsumed() const
{
    return fuel_total;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DieselEngineSystem::getSpecificFuel() const
{
    // Удельный расход имеет смысл только под нагрузкой; на холостом ходу
    // (фактическая мощность ~ 0) возвращаем 0. Считаем по нагрузочной
    // части подачи топлива: холостой расход в г/кВт*ч не подмешиваем
    if (power_kw < 1.0)
        return 0.0;

    // л/ч -> кг/ч (плотность топлива) -> г/(кВт*ч)
    const double load_kg_h = load_fuel_rate * fuel_density;

    return load_kg_h * 1000.0 / power_kw;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DieselEngineSystem::getOilLevel() const
{
    return oil / std::max(oil_capacity, 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DieselEngineSystem::getCoolantLevel() const
{
    return coolant_amount / coolant_capacity;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DieselEngineSystem::topUpCoolant(double liters)
{
    coolant_amount = std::min(coolant_capacity,
                              coolant_amount + std::max(0.0, liters));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DieselEngineSystem::getTurboBoost() const
{
    return turbo;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DieselEngineSystem::getTurboRpm() const
{
    return turbo_rpm;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DieselEngineSystem::getTurboTemp() const
{
    return turbo_t;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DieselEngineSystem::getTurboWear() const
{
    return turbo_wear;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DieselEngineSystem::getFilterClogging() const
{
    return filter_clog;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DieselEngineSystem::replaceFilter()
{
    filter_clog = 0.0;
    filter_warned = false;
    Journal::instance()->info("[DIESEL] Air filter replaced");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DieselEngineSystem::getInjectorWear() const
{
    return injector_wear;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DieselEngineSystem::serviceInjectors()
{
    injector_wear = 0.0;
    Journal::instance()->info("[DIESEL] Injectors serviced");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DieselEngineSystem::getFuelLeak() const
{
    return fuel_leak;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DieselEngineSystem::getOilLeak() const
{
    return oil_leak;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DieselEngineSystem::getCoolantLeak() const
{
    return coolant_leak;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DieselEngineSystem::repairLeaks()
{
    fuel_leak = 0.0;
    oil_leak = 0.0;
    coolant_leak = 0.0;
    oil_spill_warned = false;
    Journal::instance()->info("[DIESEL] Leaks repaired");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool DieselEngineSystem::isCranking() const
{
    return cranking;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool DieselEngineSystem::isProtectionTripped() const
{
    return protection_lock > 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DieselEngineSystem::getRestartDelay() const
{
    return protection_lock;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DieselEngineSystem::Smoke DieselEngineSystem::getSmokeLevel() const
{
    return smoke;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString DieselEngineSystem::getSmokeColor() const
{
    switch (smoke_color)
    {
    case 1: return "black";
    case 2: return "blue";
    case 3: return "white";
    }

    return "none";
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DieselEngineSystem::getWear() const
{
    return wear;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool DieselEngineSystem::hasFault() const
{
    return wear > 0.5 ||
           fuel <= 0.05 * fuel_capacity ||
           oil < 0.1 * oil_capacity ||
           fuel_leak > 0.0 ||
           oil_leak > 0.0 ||
           coolant_leak > 0.0 ||
           filter_clog > 0.8 ||
           turbo_wear > 0.7 ||
           injector_wear > 0.9;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DieselEngineSystem::refuel(double fuel_liters, double oil_liters)
{
    fuel = std::min(fuel_capacity, fuel + std::max(0.0, fuel_liters));
    oil = std::min(oil_capacity, oil + std::max(0.0, oil_liters));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString DieselEngineSystem::getDebugMsg() const
{
    const char* zones[] = {"normal", "warm", "high", "overheat", "critical"};

    return QString("Diesel: %1 rpm, %2 kW, load %3%, OJ %4C (%5), "
                   "oil %6C/%7 MPa, EGT %8C, fuel %9 l/h (%10%), smoke %11, "
                   "coolant %12%, turbo %13 rpm/%14C (w%15%), "
                   "filter %16%, inj %17%, leaks f%18/o%19/c%20%")
            .arg(rpm, 0, 'f', 0)
            .arg(power_kw, 0, 'f', 0)
            .arg(load * 100.0, 0, 'f', 0)
            .arg(coolant_t, 0, 'f', 1)
            .arg(zones[static_cast<int>(getTemperatureZone())])
            .arg(oil_t, 0, 'f', 1)
            .arg(oil_pressure, 0, 'f', 2)
            .arg(exhaust_t, 0, 'f', 0)
            .arg(fuel_rate, 0, 'f', 1)
            .arg(getFuelLevel() * 100.0, 0, 'f', 0)
            .arg(getSmokeColor())
            .arg(getCoolantLevel() * 100.0, 0, 'f', 0)
            .arg(turbo_rpm, 0, 'f', 0)
            .arg(turbo_t, 0, 'f', 0)
            .arg(turbo_wear * 100.0, 0, 'f', 0)
            .arg(filter_clog * 100.0, 0, 'f', 0)
            .arg(injector_wear * 100.0, 0, 'f', 0)
            .arg(fuel_leak * 100.0, 0, 'f', 0)
            .arg(oil_leak * 100.0, 0, 'f', 0)
            .arg(coolant_leak * 100.0, 0, 'f', 0);
}
