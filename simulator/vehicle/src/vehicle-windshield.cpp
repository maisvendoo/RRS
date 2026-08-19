//------------------------------------------------------------------------------
//
//      Windshield system (динамическое загрязнение лобового стекла)
//
//------------------------------------------------------------------------------

#include    "vehicle-windshield.h"

#include    <CfgReader.h>

#include    <algorithm>
#include    <cmath>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
WindshieldSystem::WindshieldSystem() = default;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WindshieldSystem::loadConfig(QString cfg_path)
{
    CfgReader cfg;
    if (!cfg.load(cfg_path))
        return;

    const QString sec = "Windshield";

    cfg.getDouble(sec, "SplashRate", splash_rate);
    cfg.getDouble(sec, "RainRate", rain_rate);
    cfg.getDouble(sec, "WiperCleanRate", wiper_clean_rate);
    cfg.getDouble(sec, "WashUse", wash_use);
    cfg.getDouble(sec, "DefrostRate", defrost_rate);

    // Снежная плёнка и дворники (ТЗ, п.3, 8-10)
    cfg.getDouble(sec, "SnowRate", snow_rate);
    cfg.getBool(sec, "AutoWipers", auto_wipers);
    cfg.getDouble(sec, "WiperDelay", wiper_delay);
    cfg.getDouble(sec, "WiperArc", wiper_arc);
    cfg.getDouble(sec, "WiperCenterU", wiper_center_u);
    cfg.getDouble(sec, "WiperCenterV", wiper_center_v);
    cfg.getDouble(sec, "WiperLength", wiper_length);

    // Ограничения здравого смысла
    snow_rate = std::min(std::max(snow_rate, 0.0), 1.0);
    wiper_delay = std::min(std::max(wiper_delay, 0.0), 5.0);
    wiper_arc = std::min(std::max(wiper_arc, 30.0), 180.0);
    wiper_center_u = std::min(std::max(wiper_center_u, 0.0), 1.0);
    wiper_center_v = std::min(std::max(wiper_center_v, 0.0), 1.0);
    wiper_length = std::min(std::max(wiper_length, 0.1), 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WindshieldSystem::setWipers(int mode)
{
    // Ручной выбор режима снимает авторежим
    if (mode != 1)
        auto_engaged = false;

    wiper_mode = std::min(std::max(mode, 0), 3);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WindshieldSystem::wash()
{
    if (washer_fluid > 0.0)
    {
        wash_timer = 2.0;

        // Расход на удар (ТЗ, п.10): 0.02 доли бака
        washer_fluid = std::max(0.0, washer_fluid - wash_use);

        // Дворники сработают через 0.7 с: пара взмахов поверх режима
        wiper_delay_timer = wiper_delay;
        wash_wipes_pending = 2;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WindshieldSystem::setDefroster(bool on)
{
    defroster = on;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WindshieldSystem::step(double dt, double velocity,
                            double rain_intensity, double track_wetness,
                            double air_temperature, bool snowing)
{
    const double abs_v = std::abs(velocity);
    const double speed_factor = std::min(abs_v / 25.0, 1.0);

    //--- Капли-частицы / снежная плёнка (ТЗ, п.2, 8) ---

    if (snowing)
    {
        // Снег не даёт капель: растёт снежная плёнка (сильнее на скорости);
        // интенсивность берётся от того же параметра осадков
        snow_film += snow_rate * rain_intensity *
                (0.3 + speed_factor) * dt;
    }
    else
    {
        spawnDroplets(dt, rain_intensity, speed_factor);
        stepDroplets(dt, speed_factor);
    }

    //--- Накопление (ТЗ, п.2-7) ---

    // Осадки на стекло: сильнее на скорости (встречный поток)
    water_film += rain_rate * rain_intensity * (0.3 + speed_factor) * dt;

    // Грязь: брызги из-под колёс на мокром пути (влага поднимает грязь)
    if (track_wetness > 0.2 && abs_v > 1.0)
    {
        dirt += splash_rate * track_wetness * speed_factor * dt;
    }

    // После высыхания плёнки остаётся грязь
    if (water_film > 0.7)
    {
        water_film = 0.7;
        dirt += 0.005 * dt;
    }

    //--- Зима: замерзание (ТЗ, п.14-16) ---

    if (air_temperature < -1.0 && (water_film > 0.05 || rain_intensity > 0.3))
    {
        const double freeze = std::min(water_film, 0.02 * dt *
                                       std::min(-air_temperature / 10.0, 1.0));
        water_film -= freeze;
        ice += freeze;
    }

    // Обогрев греет и сушит (плавит и снежную плёнку, ТЗ, п.8)
    if (defroster)
    {
        ice -= defrost_rate * dt;
        water_film -= defrost_rate * 0.5 * dt;
        snow_film -= defrost_rate * 0.7 * dt;
    }
    else if (air_temperature > 2.0)
    {
        ice -= 0.002 * dt * (air_temperature - 2.0);
        snow_film -= 0.001 * dt * (air_temperature - 2.0);
    }

    ice = std::min(std::max(ice, 0.0), 1.0);

    //--- Авто-режим (ключ AutoWipers, ТЗ, п.9) ---

    if (auto_wipers)
    {
        if (water_film > 0.3 && wiper_mode == 0)
        {
            // Вода выше порога - автоматически INT
            wiper_mode = 1;
            auto_engaged = true;
        }
        else if (auto_engaged && wiper_mode == 1 && water_film < 0.05)
        {
            // Стекло высохло - авторежим выключает дворники
            wiper_mode = 0;
            auto_engaged = false;
        }
    }

    //--- Взмахи дворников (зона очистки, ТЗ, п.3) ---

    stepWipers(dt, rain_intensity, ice);

    //--- Очистка ---

    // Дворники работают против воды и грязи, но не льда; износ щёток
    // снижает эффективность и оставляет грязевые полосы. Снег чистится
    // хуже (x0.5, ТЗ, п.8)
    if (wiper_mode > 0 && ice < 0.2)
    {
        const double efficiency = (1.0 - 0.6 * wiper_wear) *
                static_cast<double>(wiper_mode) / 3.0;

        water_film -= wiper_clean_rate * efficiency * dt;
        dirt -= wiper_clean_rate * 0.5 * efficiency * dt;
        snow_film -= wiper_clean_rate * 0.5 * 0.5 * efficiency * dt;

        // Работа щёток на сухом стекле изнашивает их
        if (water_film < 0.05 && dirt < 0.1)
        {
            wiper_wear = std::min(1.0, wiper_wear + 0.0002 * dt *
                                  static_cast<double>(wiper_mode));
        }
    }

    // Омыватель: импульс смывает и грязь, и плёнку
    if (wash_timer > 0.0)
    {
        wash_timer -= dt;
        dirt -= 0.15 * dt;
        water_film -= 0.05 * dt;
    }

    // Естественное высыхание/стекание
    water_film -= 0.01 * dt * (1.0 + speed_factor);

    water_film = std::min(std::max(water_film, 0.0), 1.0);
    dirt = std::min(std::max(dirt, 0.0), 1.0);
    snow_film = std::min(std::max(snow_film, 0.0), 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WindshieldSystem::spawnDroplets(double dt, double rain_intensity,
                                     double speed_factor)
{
    // Интенсивность спавна: от осадков и встречного потока, капель/с
    const double rate = 40.0 * rain_intensity * (0.3 + speed_factor);

    drop_spawn_acc += rate * dt;

    while (drop_spawn_acc >= 1.0 && droplets_.size() < kMaxDroplets)
    {
        drop_spawn_acc -= 1.0;

        Droplet drop;

        // Капли появляются по всей площади, крупнее - сверху
        drop.u = nextFloat();
        drop.v = 0.5f + 0.5f * nextFloat();
        drop.radius = 0.4f + 1.2f * nextFloat();
        drop.life = 6.0f + 10.0f * nextFloat();

        droplets_.push_back(drop);
    }

    // Переполнение аккумулятора при полном стекле - сбрасываем
    if (droplets_.size() >= kMaxDroplets)
        drop_spawn_acc = 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WindshieldSystem::stepDroplets(double dt, double speed_factor)
{
    // Стекание: скорость пропорциональна радиусу (крупные катятся
    // быстрее), встречный поток ускоряет снос; вышедшие за низ
    // и испарившиеся капли снимаются
    for (std::size_t i = 0; i < droplets_.size(); )
    {
        Droplet& drop = droplets_[i];

        drop.life -= static_cast<float>(dt);
        drop.v -= drop.radius * 0.035f *
                  static_cast<float>(0.5 + speed_factor) *
                  static_cast<float>(dt);

        if (drop.life <= 0.0f || drop.v < 0.0f)
        {
            droplets_[i] = droplets_.back();
            droplets_.pop_back();
        }
        else
        {
            ++i;
        }
    }

    // Слияние близких капель (ТЗ, п.2): радиус складывается,
    // время жизни - максимум из двух
    for (std::size_t i = 0; i < droplets_.size(); ++i)
    {
        for (std::size_t j = i + 1; j < droplets_.size(); )
        {
            const float du = droplets_[i].u - droplets_[j].u;
            const float dv = droplets_[i].v - droplets_[j].v;
            const float dist2 = du * du + dv * dv;

            const float touch = 0.004f + 0.002f *
                    (droplets_[i].radius + droplets_[j].radius);

            if (dist2 < touch * touch)
            {
                droplets_[i].radius += droplets_[j].radius;
                droplets_[i].life = std::max(droplets_[i].life,
                                             droplets_[j].life);

                droplets_[j] = droplets_.back();
                droplets_.pop_back();
            }
            else
            {
                ++j;
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WindshieldSystem::stepWipers(double dt, double rain_intensity,
                                  double ice_level)
{
    // После омывателя дворники срабатывают с задержкой (ТЗ, п.10):
    // пока идёт задержка - взмахов нет
    if (wiper_delay_timer > 0.0)
        wiper_delay_timer -= dt;

    int mode = wiper_mode;

    // Серия взмахов после омывателя: временное включение низкого режима
    if (wash_wipes_pending > 0 && wiper_delay_timer <= 0.0 && mode == 0)
        mode = 2;

    if (mode == 0)
        return;

    // Период взмаха по режиму (ТЗ, п.4): низкий 1 Гц, высокий 2 Гц;
    // INT - взмах ~0.5 Гц (период 2 с) с паузой от интенсивности осадков
    const double period = (mode == 3) ? 0.5 :
                          (mode == 2) ? 1.0 : 2.0;

    wiper_phase += dt / period;

    if (wiper_phase < 1.0)
        return;

    // Взмах завершён: очистка зоны (капли/грязь/снег в секторе)
    wiper_phase = 0.0;
    wiperStroke(ice_level);

    if (wash_wipes_pending > 0)
        --wash_wipes_pending;

    // INT: пауза между взмахами - 4 с при дожде, 12 с при мороси
    // (линейно между ними); задаётся отрицательной фазой цикла
    if (mode == 1)
    {
        const double k = std::min(std::max(rain_intensity / 0.5, 0.0), 1.0);
        const double pause = 12.0 - 8.0 * k;

        wiper_phase = -pause / period;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WindshieldSystem::wiperStroke(double ice)
{
    // По льду щётки проскальзывают - очистки нет (ТЗ, п.14-16)
    if (ice >= 0.2)
        return;

    const double efficiency = 1.0 - 0.6 * wiper_wear;

    // Капли в зоне снимаются, вне зоны остаются (реалистично, ТЗ, п.3)
    droplets_.erase(std::remove_if(droplets_.begin(), droplets_.end(),
        [this](const Droplet& drop)
        {
            return isInWiperZone(drop.u, drop.v);
        }),
        droplets_.end());

    // Доля площади стекла в зоне очистки (грубо: сектор / полукруг)
    const double coverage = std::min(wiper_arc / 180.0, 1.0) *
            std::min(wiper_length * 2.0, 1.0);

    const double gain = wiper_clean_rate * efficiency * coverage;

    // Снег чистится хуже (x0.5), вода/грязь - обычно
    water_film = std::max(0.0, water_film - gain * 0.5);
    dirt = std::max(0.0, dirt - gain * 0.3);
    snow_film = std::max(0.0, snow_film - gain * 0.5 * 0.5);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool WindshieldSystem::isInWiperZone(float u, float v) const
{
    // Зона очистки - сектор: ось дворника (wiper_center_u/v), метёт
    // вверх в пределах половины wiper_arc от вертикали, вылет щётки -
    // wiper_length; ниже оси дворник не метёт
    const float du = u - static_cast<float>(wiper_center_u);
    const float dv = v - static_cast<float>(wiper_center_v);

    if (dv < 0.0f)
        return false;

    const float dist = std::sqrt(du * du + dv * dv);

    if (dist > static_cast<float>(wiper_length))
        return false;

    // Отклонение точки от вертикали (угла взмаха), рад
    const float ang = std::atan2(du, dv);

    const float half_arc = static_cast<float>(
                wiper_arc * 3.14159265f / 180.0f * 0.5f);

    return std::abs(ang) <= half_arc;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float WindshieldSystem::nextFloat()
{
    // xorshift32: быстро и детерминированно, без глобального rand()
    std::uint32_t x = rng;

    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;

    rng = x;

    // 0..1 (без 1.0)
    return static_cast<float>(x >> 8) * (1.0f / 16777216.0f);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WindshieldSystem::getDirt() const
{
    return dirt;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WindshieldSystem::getIce() const
{
    return ice;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WindshieldSystem::getSnowFilm() const
{
    return snow_film;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const std::vector<WindshieldSystem::Droplet>& WindshieldSystem::getDroplets() const
{
    return droplets_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WindshieldSystem::getVisibilityFactor() const
{
    // Видимость из кабины: грязь + плёнка + лёд + снег (ТЗ, п.17)
    double factor = 1.0;

    factor -= 0.55 * dirt;
    factor -= 0.35 * water_film;
    factor -= 0.8 * ice;
    factor -= 0.45 * snow_film;

    return std::min(std::max(factor, 0.0), 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WindshieldSystem::getWasherFluid() const
{
    return washer_fluid;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WindshieldSystem::getWiperWear() const
{
    return wiper_wear;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int WindshieldSystem::getWipers() const
{
    return wiper_mode;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString WindshieldSystem::getDebugMsg() const
{
    return QString("Windshield: dirt %1%, water %2%, ice %3%, snow %4%, "
                   "drops %5, visibility %6%, wipers %7, washer %8%")
            .arg(dirt * 100.0, 0, 'f', 0)
            .arg(water_film * 100.0, 0, 'f', 0)
            .arg(ice * 100.0, 0, 'f', 0)
            .arg(snow_film * 100.0, 0, 'f', 0)
            .arg(static_cast<uint>(droplets_.size()))
            .arg(getVisibilityFactor() * 100.0, 0, 'f', 0)
            .arg(wiper_mode)
            .arg(washer_fluid * 100.0, 0, 'f', 0);
}
