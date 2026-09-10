#include "VehiclesHandler.h"

#include "Logger.h"
#include "settings.h"
#include "simulator-info-struct.h"
#include "simulator-update-struct.h"
#include "sound-manager.h"
#include "VehicleExterior.h"
#include "io-controller.h"

#include "graphics/particles.h"

#include <vsg/app/Viewer.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/lighting/SpotLight.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/MatrixTransform.h>

#include <QObject>
#include <QString>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cmath>
#include <random>
#include <string>
#include <utility>
#include <vector>

class QByteArray;

namespace
{

//------------------------------------------------------------------------------
// Коды погоды протокола: значения согласованы с weather::Type
// (simulator/weather/include/weather-system.h). Клиент не линкуется
// с библиотекой погоды, поэтому сравнивает числа
//------------------------------------------------------------------------------
constexpr quint8 WEATHER_TYPE_RAIN = 3;            ///< Rain
constexpr quint8 WEATHER_TYPE_HEAVY_RAIN = 4;      ///< HeavyRain (ливень)
constexpr quint8 WEATHER_TYPE_THUNDERSTORM = 13;   ///< гроза = ливень + ветер

//------------------------------------------------------------------------------
// Геометрия источников эффектов (ТЗ "Частицы"), м:
//  - выхлопная труба: над центром ПЕ и вперёд по ходу;
//  - колёсные пары: колея 1520 мм, точка контакта у головки рельса.
/// TODO: сдвиг трубы/число колёсных пар брать из конфига конкретной ПЕ
//------------------------------------------------------------------------------
constexpr double SMOKE_STACK_HEIGHT = 4.5;
constexpr double SMOKE_STACK_FORWARD = 2.0;

constexpr double WHEEL_LATERAL = 0.76;
constexpr double WHEEL_HEIGHT = 0.2;

//------------------------------------------------------------------------------
// Цвет дыма по коду протокола (0 - нет, 1 - чёрный, 2 - синий,
// 3 - белый, 4 - серый): тёмный при нагрузке, белый при холодном
// пуске (дизель) / уносе воды (паровоз)
//------------------------------------------------------------------------------
vsg::vec3 smoke_color_rgb(quint8 code)
{
    switch (code)
    {
    case 1: return vsg::vec3(0.08f, 0.08f, 0.08f);   // black
    case 2: return vsg::vec3(0.25f, 0.30f, 0.45f);   // blue
    case 3: return vsg::vec3(0.85f, 0.85f, 0.88f);   // white
    case 4: return vsg::vec3(0.55f, 0.55f, 0.58f);   // gray
    default: return vsg::vec3(0.7f, 0.7f, 0.7f);
    }
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehiclesHandler::VehiclesHandler(const settings_t& settings, SoundManager* sound_manager, QObject* parent)
    : QObject(parent)
    , sound_manager(sound_manager)
{
    settings_delay = (settings.vehicle_controled_update_interval + settings.client_delay) * 0.001;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
vsg::ref_ptr<vsg::Group> VehiclesHandler::getExterior() const noexcept
{
    return vehicles_node;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehicleExterior* VehiclesHandler::getCurrentVehicle()
{
#ifndef NDEBUG
    if (isUpdated())
    {
        if (cur_vehicle >= 0 && static_cast<std::size_t>(cur_vehicle) < vehicles.size())
        {
            return &(vehicles[cur_vehicle]);
        }
        else
        {
            LOG_WARN("cur_vehicle(%d) is not in range of vehicles.size(%d)", cur_vehicle, vehicles.size());
            return nullptr;
        }
    }
    else
    {
        return nullptr;
    }
#else
    return isUpdated() ? &(vehicles[cur_vehicle]) : nullptr;
#endif
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int VehiclesHandler::getCurrentVehicleIndex() const noexcept
{
    return cur_vehicle;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int VehiclesHandler::getControlledVehicleIndex() const noexcept
{
    return controlled_vehicle;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int VehiclesHandler::getCurrentTrainIndex() const noexcept
{
    if (cur_vehicle < 0 || static_cast<std::size_t>(cur_vehicle) >= vehicles.size())
        return -1;
    return vehicles[cur_vehicle].train_id;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehiclesHandler::isUpdated() const noexcept
{
    return pos_count >= 3;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int VehiclesHandler::getSpeedFactor() const noexcept
{
    if (pos_count.load(std::memory_order_relaxed) >= 1)
    {
        // Читаем из последнего записанного пакета, а не из буфера интерполяции
        size_t write_idx = pos_write.load(std::memory_order_acquire);

        if (write_idx == 0)
        {
            return 1;
        }

        return pos_buf[(write_idx - 1) % POS_BUF_SIZE].speed_factor;
    }

    return 1;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
simulator_time_t *VehiclesHandler::getDateTime()
{
    return (pos_count.load(std::memory_order_relaxed) >= 1)
        ? &pos_buf[pos_read % POS_BUF_SIZE].sim_time : nullptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString VehiclesHandler::getDebugMessage() const noexcept
{
    return debug_message;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehiclesHandler::getWeatherVisibility() const noexcept
{
    return weather_visibility.load(std::memory_order_relaxed);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehiclesHandler::getWeatherFogDensity() const noexcept
{
    return weather_fog_density.load(std::memory_order_relaxed);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
quint8 VehiclesHandler::getWeatherType() const noexcept
{
    return weather_type.load(std::memory_order_relaxed);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
vsg::dvec3 VehiclesHandler::getWindVector() const noexcept
{
    const double speed = weather_wind_speed.load(std::memory_order_relaxed);

    if (speed <= 0.0)
    {
        // Нет данных о погоде (старый сервер/штиль): константа 2 м/с
        return vsg::dvec3(2.0, 0.0, 0.0);
    }

    const double azimuth = weather_wind_direction.load(std::memory_order_relaxed);

    // Азимут ветра - от севера по часовой стрелке; мировые оси:
    // X - восток, Y - север (Z вверх)
    return vsg::dvec3(speed * std::sin(azimuth),
                      speed * std::cos(azimuth),
                      0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehiclesHandler::isRainWeather() const noexcept
{
    const quint8 type = weather_type.load(std::memory_order_relaxed);

    return (type == WEATHER_TYPE_RAIN) ||
           (type == WEATHER_TYPE_HEAVY_RAIN) ||
           (type == WEATHER_TYPE_THUNDERSTORM);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::set_headlight(vsg::ref_ptr<vsg::SpotLight> light, float intensity) noexcept
{
    headlight = std::move(light);
    headlight_intensity = intensity;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::setHeadlightsEnabled(bool enabled) noexcept
{
    headlights_enabled = enabled;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::set_particle_systems(graphics::ParticleSystem* smoke,
                                           graphics::ParticleSystem* splash) noexcept
{
    smoke_particles = smoke;
    splash_particles = splash;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
quint32 VehiclesHandler::getCassetteNoticeId() const noexcept
{
    return cassette_notice_id.load(std::memory_order_acquire);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString VehiclesHandler::getCassetteNotice() const noexcept
{
    std::lock_guard<std::mutex> lock(notice_mutex);
    return cassette_notice;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
simulator_diagnostics_update_t VehiclesHandler::getDiagnostics() const noexcept
{
    // Копия последнего снимка: буферы меняются местами только в step()
    // (поток рендера), сетевой поток пишет лишь в back по флагу
    return diagnostics_front;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::step(double t, double dt)
{
    ref_time.store(t, std::memory_order_relaxed);
    if (isUpdated())
    {
        if (!is_updated)
        {
            is_updated = true;
            emit updated();
        }
    }
    else
    {
        is_updated = false;
        return;
    }

    const double client_time = ref_time + time_difference.load(std::memory_order_relaxed);

    // Advance read head so pos_read is the first frame >= client_time
    advanceInterpolation(client_time);

    // Swap state double buffer
    const bool update_state = is_new_state;
    if (update_state)
    {
        std::swap(state_front, state_back);
        is_new_state = false;
    }

    // Swap diagnostics double buffer (ТЗ F3/F4)
    if (is_new_diagnostics.exchange(false, std::memory_order_acq_rel))
    {
        std::swap(diagnostics_front, diagnostics_back);
    }

    // Физические звуковые события (ТЗ "Аудиосистема"): раздаём пулу
    // источников SoundManager - OpenAL-контекст живёт в этом потоке
    if (sound_manager != nullptr)
    {
        std::vector<simulator_sound_event_t> events;

        {
            std::lock_guard<std::mutex> lock(sound_events_mutex);
            events.swap(pending_sound_events);
        }

        for (const auto& event : events)
        {
            sound_manager->playSoundEvent(event.type,
                                          event.x, event.y, event.z,
                                          event.intensity,
                                          event.rate_hz);
        }
    }
    else
    {
        std::lock_guard<std::mutex> lock(sound_events_mutex);
        pending_sound_events.clear();
    }

    // Interframe interpolation — clamp to [0,1] to prevent extrapolation overshoot
    const auto& frame_cur  = pos_buf[pos_read      % POS_BUF_SIZE];
    const auto& frame_prev = pos_buf[pos_read_prev  % POS_BUF_SIZE];
    const double upd_dt = frame_cur.sim_time.simulation_seconds - frame_prev.sim_time.simulation_seconds;
    const double r_raw = (upd_dt > 0.0) ? (client_time - frame_prev.sim_time.simulation_seconds) / upd_dt : 0.0;
    const double r = std::clamp(r_raw, 0.0, 1.0);
    const double k = (1.0 - r);

    for (std::size_t i = 0; i < vehicles.size(); ++i)
    {
        vehicles[i].position = vsg::dvec3(
            k * frame_prev.vehicles[i].position_x + r * frame_cur.vehicles[i].position_x,
            k * frame_prev.vehicles[i].position_y + r * frame_cur.vehicles[i].position_y,
            k * frame_prev.vehicles[i].position_z + r * frame_cur.vehicles[i].position_z
        );

        vehicles[i].orth = vsg::normalize(vsg::dvec3(
            k * frame_prev.vehicles[i].orth_x + r * frame_cur.vehicles[i].orth_x,
            k * frame_prev.vehicles[i].orth_y + r * frame_cur.vehicles[i].orth_y,
            k * frame_prev.vehicles[i].orth_z + r * frame_cur.vehicles[i].orth_z
        ));

        vehicles[i].up = vsg::normalize(vsg::dvec3(
            k * frame_prev.vehicles[i].up_x + r * frame_cur.vehicles[i].up_x,
            k * frame_prev.vehicles[i].up_y + r * frame_cur.vehicles[i].up_y,
            k * frame_prev.vehicles[i].up_z + r * frame_cur.vehicles[i].up_z
        ));

        vehicles[i].right = vsg::cross(vehicles[i].orth, vehicles[i].up);

        // Реакция камеры от физики (ТЗ "Физическая реакция машиниста"):
        // интерполяция между кадрами как у позиций (значения малы, но
        // без сглаживания вибрация дрожала бы сеткой обновлений)
        vehicles[i].cam_motion_offset = vsg::dvec3(
            k * static_cast<double>(frame_prev.vehicles[i].cam_offset_x) +
            r * static_cast<double>(frame_cur.vehicles[i].cam_offset_x),
            k * static_cast<double>(frame_prev.vehicles[i].cam_offset_y) +
            r * static_cast<double>(frame_cur.vehicles[i].cam_offset_y),
            k * static_cast<double>(frame_prev.vehicles[i].cam_offset_z) +
            r * static_cast<double>(frame_cur.vehicles[i].cam_offset_z)
        );

        vehicles[i].cam_motion_roll =
            k * static_cast<double>(frame_prev.vehicles[i].cam_tilt_roll) +
            r * static_cast<double>(frame_cur.vehicles[i].cam_tilt_roll);

        vehicles[i].cam_motion_pitch =
            k * static_cast<double>(frame_prev.vehicles[i].cam_tilt_pitch) +
            r * static_cast<double>(frame_cur.vehicles[i].cam_tilt_pitch);

        const vsg::dmat4 rotate_matrix{vehicles[i].right.x,vehicles[i].right.y,vehicles[i].right.z,0.0,
                                       vehicles[i].orth.x, vehicles[i].orth.y, vehicles[i].orth.z, 0.0,
                                       vehicles[i].up.x,   vehicles[i].up.y,   vehicles[i].up.z,   0.0,
                                       0.0,                0.0,                0.0,                1.0};

        // Apply vehicle body matrix transform
        vehicles[i].transform->matrix = vsg::translate(vehicles[i].position) * rotate_matrix;
        vehicles[i].cullnode->bound.center = vehicles[i].position;

        if (upd_dt > 0.0)
        {
            vehicles[i].velocity = vsg::dvec3(
                (frame_cur.vehicles[i].position_x - frame_prev.vehicles[i].position_x) / upd_dt,
                (frame_cur.vehicles[i].position_y - frame_prev.vehicles[i].position_y) / upd_dt,
                (frame_cur.vehicles[i].position_z - frame_prev.vehicles[i].position_z) / upd_dt
            );
        }

        // Model animations update and step
        if (update_state)
        {
            vehicles[i].train_id = state_front.vehicles[i].train_id;
            vehicles[i].orientation = state_front.vehicles[i].orientation;
            vehicles[i].prev_vehicle = state_front.vehicles[i].prev_vehicle;
            vehicles[i].next_vehicle = state_front.vehicles[i].next_vehicle;

            vehicles[i].step(static_cast<float>(t), static_cast<float>(dt), &(state_front.vehicles[i].analogSignal));
        }
        else
        {
            vehicles[i].step(static_cast<float>(t), static_cast<float>(dt));
        }

        // Sounds update
        for (auto sound_id : vehicles[i].sounds_id)
        {
            const vsg::vec3 pos = vsg::vec3(vehicles[i].position) +
                                  vsg::vec3(vehicles[i].right) * sound_manager->getLocalPositionX(sound_id) +
                                  vsg::vec3(vehicles[i].orth) * sound_manager->getLocalPositionY(sound_id) +
                                  vsg::vec3(vehicles[i].up) * sound_manager->getLocalPositionZ(sound_id);
            sound_manager->setPosition(sound_id, pos.x, pos.y, pos.z);

            if (update_state)
            {
                sound_manager->setVelocity(sound_id, vehicles[i].velocity.x, vehicles[i].velocity.y, vehicles[i].velocity.z);

                const std::size_t signal_id = sound_manager->getSignalID(sound_id);
                if (signal_id < state_front.vehicles[i].analogSignal.size())
                    sound_manager->setSoundSignal(sound_id, state_front.vehicles[i].analogSignal[signal_id]);
                else
                    sound_manager->setSoundSignal(sound_id, 0.0f);
            }
        }
    }

    // Динамический свет и частицы (ТЗ "Частицы", High+): фары, дым,
    // брызги. Оси ПЕ уже интерполированы выше; 64 частицы на CPU дёшево
    updateVehicleEffects(dt, frame_cur);
}

//------------------------------------------------------------------------------
// Динамический свет и частицы (ТЗ "Частицы"):
//  - фары: SpotLight управляемой ПЕ (создаётся RouteViewer на High+,
//    здесь только позиция/направление по интерполированным осям);
//  - дым/пар: выхлопная труба текущей ПЕ, цвет/интенсивность из
//    smoke_level/smoke_color (сервер переносит их из
//    DieselEngineSystem/SteamEngineSystem, паровоз в приоритете);
//  - брызги: нижние точки колёсных пар текущей ПЕ в дождь/ливень,
//    количество и скорость пропорциональны скорости ПЕ.
// Все системы создаются только на пресетах High/Ultra/Extreme —
// на Legacy/Low указатели пустые и метод практически ничего не делает
//------------------------------------------------------------------------------
void VehiclesHandler::updateVehicleEffects(double dt, const simulator_update_pos_t& frame)
{
    // ПСЧ визуального разброса (скорости/жизни частиц): фиксированный
    // seed - детерминированная картинка без затрат на энтропию.
    // Метод вызывается только из потока рендера (step)
    static std::mt19937 effect_rng(42u);
    std::uniform_real_distribution<double> unit_dist(0.0, 1.0);

    //--------- Фары управляемой ПЕ ---------

    if (headlight)
    {
        int light_idx = controlled_vehicle;

        if (light_idx < 0 || static_cast<std::size_t>(light_idx) >= vehicles.size())
        {
            light_idx = cur_vehicle;
        }

        if (light_idx >= 0 && static_cast<std::size_t>(light_idx) < vehicles.size())
        {
            const VehicleExterior& veh = vehicles[light_idx];

            // Направление "вперёд" кабины: локальная +Y ПЕ по знаку
            // ориентации (1 - вперёд, -1 - назад)
            const double dir = (veh.orientation >= 0) ? 1.0 : -1.0;

            const std::size_t idx = static_cast<std::size_t>(light_idx);
            const double length = (idx < vehicle_lengths.size())
                                  ? vehicle_lengths[idx]
                                  : 20.0;

            // Фара у торца кузова на высоте лобового стекла
            headlight->position = veh.position +
                                  veh.up * 1.5 +
                                  veh.orth * (dir * length * 0.45);

            // Оси ПЕ нормализованы при интерполяции
            headlight->direction = veh.orth * dir;

            // TODO: мост к реальному тумблеру фар конкретной ПЕ
            // (пока глобальный флаг settings.headlights)
            headlight->intensity = headlights_enabled ? headlight_intensity : 0.0f;
        }
        else
        {
            headlight->intensity = 0.0f;
        }
    }

    if (!smoke_particles && !splash_particles)
    {
        return;
    }

    //--------- Дым/пар из выхлопной трубы текущей ПЕ ---------

    const bool cur_valid = (cur_vehicle >= 0) &&
                           (static_cast<std::size_t>(cur_vehicle) < vehicles.size()) &&
                           (static_cast<std::size_t>(cur_vehicle) < frame.vehicles.size());

    if (smoke_particles && cur_valid)
    {
        const std::size_t idx = static_cast<std::size_t>(cur_vehicle);
        const VehicleExterior& veh = vehicles[idx];
        const auto& veh_state = frame.vehicles[idx];

        if (veh_state.smoke_level > 0)
        {
            // Уровень 1..4 -> 0.25..1: плотнее дым - интенсивнее выброс
            const double intensity = std::min(veh_state.smoke_level, quint8(4)) / 4.0;

            // 6..30 частиц/с
            const double rate = 6.0 + 24.0 * intensity;

            const double dir = (veh.orientation >= 0) ? 1.0 : -1.0;

            // Труба: над центром ПЕ, сдвинута к кабине по ходу движения
            const vsg::dvec3 pipe = veh.position +
                                    veh.up * SMOKE_STACK_HEIGHT +
                                    veh.orth * (dir * SMOKE_STACK_FORWARD);

            const vsg::dvec3 wind = getWindVector();
            const vsg::vec3 color = smoke_color_rgb(veh_state.smoke_color);

            smoke_emit_accum += rate * dt;

            while (smoke_emit_accum >= 1.0)
            {
                smoke_emit_accum -= 1.0;

                graphics::ParticleSystem::Spawn spawn;

                spawn.position = vsg::vec3(pipe);

                // Вверх 2..5 м/с (сильнее при интенсивном дыме) + ветер
                spawn.velocity = vsg::vec3(
                    wind +
                    veh.up * (2.0 + 3.0 * intensity) +
                    veh.orth * ((unit_dist(effect_rng) - 0.5) * 0.6) +
                    veh.right * ((unit_dist(effect_rng) - 0.5) * 0.6));

                spawn.color = color;
                spawn.alpha = static_cast<float>(0.25 + 0.30 * intensity);
                spawn.size_begin = 0.5f;    // рост 0.5 -> 3 м за жизнь
                spawn.size_end = 3.0f;
                spawn.lifetime = static_cast<float>(3.0 + 3.0 * unit_dist(effect_rng));

                smoke_particles->emitParticles(spawn);
            }
        }
    }

    //--------- Брызги из-под колёс в дождь ---------

    if (splash_particles && cur_valid && isRainWeather())
    {
        const std::size_t idx = static_cast<std::size_t>(cur_vehicle);
        const VehicleExterior& veh = vehicles[idx];

        const double speed = vsg::length(veh.velocity);

        if (speed > 0.5)
        {
            const double dir = (veh.orientation >= 0) ? 1.0 : -1.0;
            const double length = (idx < vehicle_lengths.size())
                                  ? vehicle_lengths[idx]
                                  : 20.0;

            // Две колёсные пары (тележки) по два колеса: точки у
            // головки рельса. TODO: конфиг осевых формул конкретной ПЕ
            const double along_offsets[2] = {length * 0.3, -length * 0.3};

            const double side_offsets[2] = {-WHEEL_LATERAL, WHEEL_LATERAL};

            // На колесо 0..20 частиц/с, пропорционально скорости ПЕ
            const double wheel_rate = std::clamp(speed * 1.0, 0.0, 20.0);

            for (double along : along_offsets)
            {
                for (double side : side_offsets)
                {
                    const vsg::dvec3 wheel = veh.position +
                                             veh.right * side +
                                             veh.orth * (dir * along) +
                                             veh.up * WHEEL_HEIGHT;

                    splash_emit_accum += wheel_rate * dt;

                    while (splash_emit_accum >= 1.0)
                    {
                        splash_emit_accum -= 1.0;

                        graphics::ParticleSystem::Spawn spawn;

                        spawn.position = vsg::vec3(wheel);

                        // Вверх 1..3 м/с + снос скоростью ПЕ и ветром
                        spawn.velocity = vsg::vec3(
                            veh.velocity * 0.3 +
                            veh.up * (1.0 + 2.0 * unit_dist(effect_rng)) +
                            veh.right * ((unit_dist(effect_rng) - 0.5) * 1.5) +
                            veh.orth * ((unit_dist(effect_rng) - 0.5) * 1.5));

                        // Полупрозрачный белый, маленькие капли
                        spawn.color = vsg::vec3(0.8f, 0.85f, 0.92f);
                        spawn.alpha = 0.35f;
                        spawn.size_begin = 0.1f;   // 0.1 -> 0.3 м
                        spawn.size_end = 0.3f;
                        spawn.lifetime = static_cast<float>(0.5 + 0.5 * unit_dist(effect_rng));

                        splash_particles->emitParticles(spawn);
                    }
                }
            }
        }
    }

    // Шаг симуляции пулов (движение/рост/fade) — каждый кадр, даже без
    // спавна: живые частицы обязаны догорать
    if (smoke_particles)
    {
        smoke_particles->step(dt);
    }

    if (splash_particles)
    {
        splash_particles->step(dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehiclesHandler::selectNextTrain() noexcept
{
    const int prev_cur_vehicle = cur_vehicle;

    // Переключаем на первый вагон предыдущего поезда
    if (vehicles[cur_vehicle].train_id <= 0)
    {
        const int new_train_id = update_trains.trains.size() - 1;
        cur_vehicle = update_trains.trains[new_train_id].first_vehicle_id;
    }
    else
    {
        const int new_train_id = vehicles[cur_vehicle].train_id - 1;
        cur_vehicle = update_trains.trains[new_train_id].first_vehicle_id;
    }

    return (cur_vehicle != prev_cur_vehicle);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehiclesHandler::selectPrevTrain() noexcept
{
    const int prev_cur_vehicle = cur_vehicle;

    // Переключаем на первый вагон следующего поезда
    if (static_cast<std::size_t>(vehicles[cur_vehicle].train_id) >= (update_trains.trains.size() - 1))
    {
        cur_vehicle = update_trains.trains[0].first_vehicle_id;
    }
    else
    {
        const int new_train_id = vehicles[cur_vehicle].train_id + 1;
        cur_vehicle = update_trains.trains[new_train_id].first_vehicle_id;
    }

    return (cur_vehicle != prev_cur_vehicle);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehiclesHandler::selectNextVehicle() noexcept
{
    const int prev_cur_vehicle = cur_vehicle;

    // Переключение по вагонам поезда вперёд
    if (vehicles[cur_vehicle].prev_vehicle >= 0)
    {
        cur_vehicle = vehicles[cur_vehicle].prev_vehicle;
    }
    else
    {
        // С первого вагона переключаемся на последний
        const int cur_train_id = vehicles[cur_vehicle].train_id;
        cur_vehicle = update_trains.trains[cur_train_id].last_vehicle_id;
    }

    return (cur_vehicle != prev_cur_vehicle);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehiclesHandler::selectPrevVehicle() noexcept
{
    const int prev_cur_vehicle = cur_vehicle;

    // Переключение по вагонам поезда назад
    if (vehicles[cur_vehicle].next_vehicle >= 0)
    {
        cur_vehicle = vehicles[cur_vehicle].next_vehicle;
    }
    else
    {
        // С последнего вагона переключаемся на первый
        const int cur_train_id = vehicles[cur_vehicle].train_id;
        cur_vehicle = update_trains.trains[cur_train_id].first_vehicle_id;
    }

    return (cur_vehicle != prev_cur_vehicle);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehiclesHandler::selectControlVehicle() noexcept
{
    auto vehicle = getCurrentVehicle();

    if (vehicle)
    {
        const int prev_contr_vehicle = controlled_vehicle;
        const int prev_contr_cabine = vehicle->controlled_cabine_idx;

        // Берём контроль над данным вагоном
        controlled_vehicle = cur_vehicle;
        // Берём контроль над данной кабиной
        vehicle->controlled_cabine_idx = vehicle->current_cabine_idx;

        return (controlled_vehicle != prev_contr_vehicle) ||
               (vehicle->controlled_cabine_idx != prev_contr_cabine);
    }
    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehiclesHandler::returnToControlledVehicle() noexcept
{
    const int prev_cur_vehicle = cur_vehicle;

    // Возврат к управляемому вагону
    cur_vehicle = controlled_vehicle;

    return (cur_vehicle != prev_cur_vehicle);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehiclesHandler::selectVehicle(int idx) noexcept
{
    if (idx < 0 || static_cast<size_t>(idx) >= vehicles.size())
        return false;

    if (idx == cur_vehicle)
        return true;

    cur_vehicle = idx;
    vehicles[cur_vehicle].current_cabine_idx = 0;
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int VehiclesHandler::getVehiclesCount() const noexcept
{
    return static_cast<int>(vehicles.size());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehiclesHandler::load(
    QByteArray& data,
    const settings_t& settings,
    vsg::ref_ptr<vsg::Options> options
)
{
    vehicles_info.deserialize(data);
    if (vehicles_info.vehicles.empty())
    {
        LOG_WARN("Server has not any vehicles");
        return false;
    }

    const std::size_t vehicle_count = vehicles_info.vehicles.size();
    LOG_INFO("Got info about %u vehicles from server", vehicle_count);

    vehicles.reserve(vehicle_count);
    vehicles_node->children.reserve(vehicle_count);

    // Длины ПЕ: позиции фар и колёсных пар (ТЗ "Частицы")
    vehicle_lengths.reserve(vehicle_count);

    for (std::size_t i = 0; i < vehicle_count; ++i)
    {
        const std::string cfg_dir = vehicles_info.vehicles[i].vehicle_config_dir.toStdString();
        const std::string cfg_file = vehicles_info.vehicles[i].vehicle_config_file.toStdString();
        const double veh_len = vehicles_info.vehicles[i].vehicle_length;

        vehicle_lengths.push_back(veh_len);

        vehicles.emplace_back(VehicleExterior());
        VehicleExterior& vehicle_exterior = vehicles.back();
        vehicle_exterior.driver_pos[vehicle_exterior.current_cabine_idx] = settings.cabine_default_pos;
        vehicle_exterior.saved_cabine_cam_fov = settings.fovy;

        if (vehicle_exterior.loadVehicle(cfg_dir, cfg_file, sound_manager, options))
        {
            LOG_INFO("Added vehicle %u / %u with model from %s / %s.xml",
                     i + 1, vehicle_count, cfg_dir.c_str(), cfg_file.c_str());
        }
        else
        {
            LOG_WARN("Added vehicle %u / %u. Fail to load model from %s / %s.xml",
                     i + 1, vehicle_count, cfg_dir.c_str(), cfg_file.c_str());
        }

        vehicle_exterior.cullnode->bound = vsg::dsphere(0.0, 0.0, 0.0, veh_len);
        vehicle_exterior.cullnode->child = vehicle_exterior.transform;
        vehicles_node->addChild(vehicle_exterior.cullnode);

        if (!vehicle_exterior.io_controls.empty())
        {
            for (auto *io_control : vehicle_exterior.io_controls)
            {
                if (io_control != nullptr)
                {
                    connect(io_control, &IOController::sigSendVehicleControlCommand,
                            this, &VehiclesHandler::sigSendVehicleControlCommand);
                }
            }
        }
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::slotGetTrainsData(QByteArray &data)
{
    update_trains.deserialize(data);
/*
    QString msg = "";
    msg += "Trains(";
    msg += QString::number(update_trains.trains.size());
    msg += "):";
    for (size_t i = 0; i < update_trains.trains.size(); ++i)
    {
        msg += "\n";
        msg += update_trains.trains[i].train_name;
        msg += ":";
        msg += QString::number(update_trains.trains[i].first_vehicle_id);
        msg += ",";
        msg += QString::number(update_trains.trains[i].last_vehicle_id);
    }
    LOG_INFO("%s", msg.toStdString().c_str());
*/
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::slotGetVehiclesPosData(QByteArray& data)
{
    const size_t slot = pos_write.load(std::memory_order_relaxed) % POS_BUF_SIZE;
    pos_buf[slot].deserialize(data);

    if (pos_buf[slot].vehicles.size() != vehicles.size())
    {
        LOG_WARN("Fail to update: get %zu positions but there are %zu vehicles",
                 pos_buf[slot].vehicles.size(), vehicles.size());
        return;
    }

    // Погода от симулятора (ТЗ "Видимость и погода")
    weather_visibility.store(pos_buf[slot].visibility_m, std::memory_order_relaxed);
    weather_fog_density.store(pos_buf[slot].fog_density, std::memory_order_relaxed);

    // Погода для эффектов рендера (ТЗ "Частицы"): тип/интенсивность/
    // ветер. Старый сервер хвост не пришлёт - останутся прежние значения
    weather_type.store(pos_buf[slot].weather_type, std::memory_order_relaxed);
    weather_wind_speed.store(pos_buf[slot].wind_speed, std::memory_order_relaxed);
    weather_wind_direction.store(pos_buf[slot].wind_direction, std::memory_order_relaxed);

    // Предупреждение кассеты регистрации ("Запись параметров движения
    // начата/окончена", ТЗ "Кассеты"): id обновляем после текста,
    // чтобы читатель не увидел новый id со старым текстом
    if (pos_buf[slot].notice_id > 0)
    {
        std::lock_guard<std::mutex> lock(notice_mutex);
        cassette_notice = pos_buf[slot].notice;
        cassette_notice_id.store(pos_buf[slot].notice_id,
                                 std::memory_order_release);
    }

    // Физические звуковые события (ТЗ "Аудиосистема"): копим для
    // разбора в кадре (OpenAL-контекст - поток рендера)
    if (!pos_buf[slot].sound_events.empty())
    {
        std::lock_guard<std::mutex> lock(sound_events_mutex);

        for (const auto& event : pos_buf[slot].sound_events)
        {
            // Ограничение очереди: переполнение вытесняет старые
            if (pending_sound_events.size() < 64)
            {
                pending_sound_events.push_back(event);
            }
        }
    }

    // Exponential smoothing of time offset (converges quickly during startup)
    const size_t count = pos_count.load(std::memory_order_relaxed);
    const double alpha = (count < 3) ? 0.5 : 0.05;
    const double td = time_difference.load(std::memory_order_relaxed);
    const double rt = ref_time.load(std::memory_order_relaxed);
    time_difference.store(td * (1.0 - alpha) +
        (pos_buf[slot].sim_time.simulation_seconds - rt - settings_delay) * alpha,
        std::memory_order_relaxed);

    // Publish: data is fully written, now make it visible to the reader
    const size_t new_write = pos_write.load(std::memory_order_relaxed) + 1;
    pos_write.store(new_write, std::memory_order_release);
    if (count < POS_BUF_SIZE)
        pos_count.store(count + 1, std::memory_order_release);

    // Initialize read heads once we have enough frames
    if (count + 1 == 3)
    {
        pos_read_prev = new_write - 3;
        pos_read      = new_write - 2;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::slotGetVehiclesStateData(QByteArray& data)
{
    if (is_new_state)
        return;

    state_back.deserialize(data);
    if (state_back.vehicles.size() == vehicles.size())
    {
        is_new_state = true;
    }
    else
    {
        LOG_WARN("Fail to update: get %zu states but there are %zu vehicles",
                 state_back.vehicles.size(), vehicles.size());
        is_new_state = false;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::slotGetVehicleControlled(QByteArray& data)
{
    if (!isUpdated())
        return;

    vehicle_controlled.deserialize(data);
    if ((vehicle_controlled.controlled_vehicle >= 0) &&
        (static_cast<std::size_t>(vehicle_controlled.controlled_vehicle) < vehicles.size()) &&
        (vehicle_controlled.current_vehicle >= 0) &&
        (static_cast<std::size_t>(vehicle_controlled.current_vehicle) < vehicles.size()))
    {
        updateDebugString();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::slotGetDiagnosticsData(QByteArray& data)
{
    // Двойной буфер: назад пишем только когда передний обменян
    if (is_new_diagnostics.load(std::memory_order_relaxed))
        return;

    diagnostics_back.deserialize(data);
    is_new_diagnostics.store(true, std::memory_order_release);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::advanceInterpolation(double client_time)
{
    // Advance read head until pos_read is the first frame with time >= client_time
    // but don't go past the latest written frame.
    // pos_write is atomic — snapshot it once to avoid torn reads in the loop.
    const size_t write_snapshot = pos_write.load(std::memory_order_acquire);
    if (write_snapshot == 0)
        return;
    const size_t latest = write_snapshot - 1;
    while (pos_read < latest &&
           client_time >= pos_buf[pos_read % POS_BUF_SIZE].sim_time.simulation_seconds)
    {
        pos_read_prev = pos_read;
        ++pos_read;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::updateDebugString()
{
    const size_t w = pos_write.load(std::memory_order_acquire);
    if (w == 0)
        return;

    auto& latest = pos_buf[(w - 1) % POS_BUF_SIZE];

    // Дата-время сервера
    debug_message = latest.sim_time.getString() + "\n";

    const int current = vehicle_controlled.current_vehicle;
    if (current >= 0
        && static_cast<std::size_t>(current) < state_front.vehicles.size()
        && static_cast<std::size_t>(current) < latest.vehicles.size())
    {
        const int current_train = state_front.vehicles[current].train_id;
        const auto& new_pos_data = latest.vehicles[current];
        debug_message += QString("Данная ПЕ: %1 | Поезд %2 | pos{%3,%4,%5} | dir{%6,%7,%8}\n")
            .arg(current, 3)
            .arg(current_train, 3)
            .arg(new_pos_data.position_x, 8, 'f', 1)
            .arg(new_pos_data.position_y, 8, 'f', 1)
            .arg(new_pos_data.position_z, 8, 'f', 1)
            .arg(new_pos_data.orth_x, 6, 'f', 3)
            .arg(new_pos_data.orth_y, 6, 'f', 3)
            .arg(new_pos_data.orth_z, 6, 'f', 3);

        debug_message += vehicle_controlled.currentDebugMsg + QString("\n");
    }
    else
    {
        debug_message += QString("\n\n");
    }

    const int control = vehicle_controlled.controlled_vehicle;
    if (control >= 0
        && control < state_front.vehicles.size()
        && control < latest.vehicles.size())
    {
        const int control_train = state_front.vehicles[control].train_id;
        const auto& new_pos_data = latest.vehicles[control];
        debug_message += QString("\nУправляемая ПЕ: %1 | Поезд %2 | pos{%3,%4,%5} | dir{%6,%7,%8}\n")
            .arg(control, 3)
            .arg(control_train, 3)
            .arg(new_pos_data.position_x, 8, 'f', 1)
            .arg(new_pos_data.position_y, 8, 'f', 1)
            .arg(new_pos_data.position_z, 8, 'f', 1)
            .arg(new_pos_data.orth_x, 6, 'f', 3)
            .arg(new_pos_data.orth_y, 6, 'f', 3)
            .arg(new_pos_data.orth_z, 6, 'f', 3);

        debug_message += vehicle_controlled.controlledDebugMsg;
    }
    else
    {
        debug_message += QString("\nУправляемая ПЕ: не выбрана\nНажмите Enter, чтобы управлять данной ПЕ");
    }
}
