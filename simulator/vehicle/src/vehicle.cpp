//------------------------------------------------------------------------------
//
//      Vehicle base class
//      (c) maisvendoo, 03/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief  Vehicle base class
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 03/09/2018
 */

#include    "vehicle.h"

#include    "CfgReader.h"
#include    "physics.h"
#include    "Journal.h"

#include    <collision-event.h>
#include    <collision-layer.h>

#include    <QDir>
#include    <QFileInfo>
#include    <QDataStream>

#include    <algorithm>
#include    <cmath>

#include    "key-symbols.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Vehicle::Vehicle(QObject* parent) : QObject(parent)
{
    std::fill(analogSignal.begin(), analogSignal.end(), 0.0f);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Vehicle::~Vehicle()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::init(QString cfg_path)
{
    Journal::instance()->info("Started base class Vehicle initialization...");
    loadConfiguration(cfg_path);
    Journal::instance()->info("Base class initialization finished");

    Journal::instance()->info("Call of Vehicle::initialization() method...");
    initialization();
    Journal::instance()->info("Custom initialization finished");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setModuleDir(QString module_dir)
{
    this->module_dir = module_dir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setModuleName(QString module_name)
{
    this->module_name = module_name;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setConfigDir(QString config_dir)
{
    this->config_dir = config_dir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setConfigName(QString config_name)
{
    this->config_name = config_name;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setRouteDir(QString route_dir)
{
    this->route_dir = route_dir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setModelIndex(size_t idx)
{
    this->model_idx = idx;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setTrainIndex(size_t idx)
{
    this->train_idx = idx;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setStateIndex(size_t idx)
{
    this->state_idx = idx;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setProfilePoint(profile_point_t point_data)
{
    this->profile_point_data = point_data;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::createCollisionBodies(collision::CollisionWorld* world)
{
    if (world == nullptr)
        return;

    // Группа = индекс ПЕ + 1 (0 - отсутствие группы):
    // тела одной ПЕ между собой не сталкиваются
    colliders.createBodies(*world,
                           static_cast<collision::GroupId>(model_idx + 1),
                           this);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::syncCollisionPose()
{
    colliders.syncPose(profile_point_data);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::onCollisionContact(const collision::CollisionEvent& event)
{
    // Повреждения накапливаются при каждом новом контакте
    applyCollisionDamage(event);

    if (is_collided)
        return;

    is_collided = true;

    const bool we_are_a = (event.user_data_a == this);
    const collision::Layer other_layer = we_are_a ? event.layer_b : event.layer_a;

    Journal::instance()->warning(QString("Vehicle #%1 COLLISION with %2 at (%3; %4; %5)")
        .arg(model_idx)
        .arg(collision::layerName(other_layer))
        .arg(static_cast<double>(event.point.x), 0, 'f', 1)
        .arg(static_cast<double>(event.point.y), 0, 'f', 1)
        .arg(static_cast<double>(event.point.z), 0, 'f', 1));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::applyCollisionDamage(const collision::CollisionEvent& event)
{
    const bool we_are_a = (event.user_data_a == this);
    const collision::Layer our_layer = we_are_a ? event.layer_a : event.layer_b;

    // Кинетическая энергия вдоль нормали контакта:
    // скорость ПЕ направлена вдоль касательной к траектории
    const dvec3& o = profile_point_data.orth;
    const dvec3& r = profile_point_data.right;
    const dvec3& u = profile_point_data.up;

    const double o_len = std::sqrt(o.x * o.x + o.y * o.y + o.z * o.z);
    if (o_len < 1e-6)
        return;

    const double cos_impact = (o.x * event.normal.x +
                               o.y * event.normal.y +
                               o.z * event.normal.z) / o_len;

    const double v_normal = velocity * cos_impact;
    const double energy = 0.5 * full_mass * v_normal * v_normal;

    if (energy <= 0.0)
        return;

    // Зона удара по направлению нормали в локальных осях ПЕ (ТЗ
    // "Реалистичный сход ПС", п.17: место контакта влияет на повреждения)
    const double lateral = std::abs(r.x * event.normal.x +
                                    r.y * event.normal.y +
                                    r.z * event.normal.z) / o_len;
    const double vertical = std::abs(u.x * event.normal.x +
                                     u.y * event.normal.y +
                                     u.z * event.normal.z) / o_len;

    VehicleDamageSystem::ImpactZone zone = VehicleDamageSystem::ImpactZone::Side;

    if (vertical > 0.7)
    {
        zone = VehicleDamageSystem::ImpactZone::Top;
    }
    else if (lateral > 0.5)
    {
        zone = VehicleDamageSystem::ImpactZone::Side;
    }
    else
    {
        // Продольный удар: спереди или сзади по знаку проекции
        zone = (cos_impact > 0.0) ? VehicleDamageSystem::ImpactZone::Front
                                  : VehicleDamageSystem::ImpactZone::Rear;
    }

    const auto component = (our_layer == collision::Layer::Train)
            ? VehicleDamageSystem::Component::Body
            : VehicleDamageSystem::Component::Bogie;

    (void) component;

    damage_system.applyImpact(energy, zone);

    // Боковой удар с высокой энергией - импульс на разгрузку колёс:
    // связь столкновения с системой схода (ТЗ, п.2, 14)
    if (zone == VehicleDamageSystem::ImpactZone::Side && energy > 200e3)
    {
        derailment.applyLateralImpact(energy, event.normal.x, event.normal.y,
                                      event.normal.z, o.x, o.y, o.z, o_len);
    }

    Journal::instance()->warning(QString("Vehicle #%1 impact damage: energy %2 kJ")
        .arg(model_idx)
        .arg(energy / 1000.0, 0, 'f', 1));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::resetDamage()
{
    damage_system.reset();
    hazard.reset();
    body_damage = 0.0f;
    bogie_damage = 0.0f;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehicleDamageSystem& Vehicle::getDamageSystem()
{
    return damage_system;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehicleHazard& Vehicle::getHazard()
{
    return hazard;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
WheelFlatSystem& Vehicle::getFlatSpots()
{
    return flat_spots;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
WheelRailAdhesion& Vehicle::getAdhesion()
{
    return adhesion;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SandSystem& Vehicle::getSand()
{
    return sand;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BrakeShoeSystem& Vehicle::getBrakeShoes()
{
    return brake_shoes;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DieselEngineSystem& Vehicle::getDiesel()
{
    return diesel;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EnergyMeterSystem& Vehicle::getEnergy()
{
    return energy;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PantographSystem& Vehicle::getPantograph()
{
    return pantograph;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DepotPowerSystem& Vehicle::getDepotPower()
{
    return depot_power;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int Vehicle::getSMEGroup() const
{
    return sme_group;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const control_signals_t& Vehicle::getControlSignalsRef() const
{
    return control_signals;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CouplingInteraction& Vehicle::getCouplingInteraction()
{
    return coupling_interaction;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::applyWindToPantograph(double wind_speed)
{
    wind_speed_for_pantograph = wind_speed;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
WindshieldSystem& Vehicle::getWindshield()
{
    return windshield;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CameraMotionFromPhysics& Vehicle::getCameraMotion()
{
    return camera_motion;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CabInteractionRegistry& Vehicle::getCabInteraction()
{
    return cab_interaction;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CargoSystem& Vehicle::getCargo()
{
    return cargo;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PassengerSystem& Vehicle::getPassengers()
{
    return passengers;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ServiceSystem& Vehicle::getService()
{
    return service_system;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CondensateSystem& Vehicle::getCondensate()
{
    return condensate_system;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
WheelWearSystem& Vehicle::getWheelWear()
{
    return wheel_wear;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TunnelAerodynamics& Vehicle::getTunnel()
{
    return tunnel_aero;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::reprofileWheels()
{
    // Обточка: ползуны срезаются вместе с изношенным слоем бандажа,
    // система износа восстанавливает профиль (коничность падает к базе)
    flat_spots.reset();
    wheel_wear.reprofile();

    Journal::instance()->info(QString(
        "[SERVICE] Vehicle #%1 wheels reprofiled (turn %2)")
        .arg(model_idx)
        .arg(wheel_wear.getReprofileCount()));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setRainIntensity(double intensity)
{
    rain_intensity = std::min(std::max(intensity, 0.0), 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setSimulationLOD(perf::SimLOD lod)
{
    sim_lod = lod;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
perf::SimLOD Vehicle::getSimulationLOD() const
{
    return sim_lod;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::collectSoundEvents(std::vector<SoundEvent>& out)
{
    const dvec3& pos = profile_point_data.position;

    // Кулдауны по типам (подготовка к аудио-потребителю, чтобы не
    // спамить сотнями событий/с): импульсные - 0.1 с, непрерывные -
    // 0.5 с. Время последней отправки - члены ПЕ, часы - sim_clock
    auto allow = [this](SoundEventType type, double cooldown_s)
    {
        const auto idx = static_cast<std::size_t>(type);

        if (idx >= 10)
            return false;

        if (sound_event_last_time[idx] > 0.0 &&
                sim_clock - sound_event_last_time[idx] < cooldown_s)
            return false;

        sound_event_last_time[idx] = sim_clock;
        return true;
    };

    // Удар ползуна: новое соударение с последнего опроса
    for (size_t i = 0; i < num_axis; ++i)
    {
        const double depth = flat_spots.getDepth(i);

        if (depth > 1e-5 && allow(SoundEventType::FlatImpact, 0.1))
        {
            SoundEvent ev;
            ev.type = SoundEventType::FlatImpact;
            ev.x = pos.x; ev.y = pos.y; ev.z = pos.z;
            ev.intensity = std::min(1.0, depth / 0.004);
            ev.rate_hz = flat_spots.getImpactRate(i, velocity, rk[i]);
            ev.vehicle_idx = model_idx;
            out.push_back(ev);
            break;
        }
    }

    // Контакт гребня (поперечная динамика): смещение осей у гребня
    if (lateral_dynamics.isEnabled())
    {
        for (size_t i = 0; i < num_axis; ++i)
        {
            if ((lateral_dynamics.getWheelContactState(i, 0) ==
                    VehicleLateralDynamics::ContactState::FlangeContact ||
                lateral_dynamics.getWheelContactState(i, 1) ==
                    VehicleLateralDynamics::ContactState::FlangeContact) &&
                allow(SoundEventType::FlangeContact, 0.5))
            {
                SoundEvent ev;
                ev.type = SoundEventType::FlangeContact;
                ev.x = pos.x; ev.y = pos.y; ev.z = pos.z;
                ev.intensity = std::min(1.0, std::abs(velocity) / 25.0);
                ev.vehicle_idx = model_idx;
                out.push_back(ev);
                break;
            }
        }

        // Боксование/юз: крип в скольжении
        for (size_t i = 0; i < num_axis; ++i)
        {
            if ((lateral_dynamics.getWheelContactState(i, 0) ==
                    VehicleLateralDynamics::ContactState::Sliding ||
                lateral_dynamics.getWheelContactState(i, 1) ==
                    VehicleLateralDynamics::ContactState::Sliding) &&
                allow(SoundEventType::WheelSlip, 0.5))
            {
                SoundEvent ev;
                ev.type = SoundEventType::WheelSlip;
                ev.x = pos.x; ev.y = pos.y; ev.z = pos.z;
                ev.intensity = 0.6;
                ev.vehicle_idx = model_idx;
                out.push_back(ev);
                break;
            }
        }
    }

    // Скрежет после схода
    if (derailment.isDerailed() && abs(velocity) > 0.3 &&
        allow(SoundEventType::DerailmentScrape, 0.5))
    {
        SoundEvent ev;
        ev.type = SoundEventType::DerailmentScrape;
        ev.x = pos.x; ev.y = pos.y; ev.z = pos.z;
        ev.intensity = std::min(1.0, abs(velocity) / 15.0);
        ev.rate_hz = derailment.getSleeperImpactRate();
        ev.vehicle_idx = model_idx;
        out.push_back(ev);
    }

    // Дуги токоприёмника
    if (pantograph.getArcRate() > 0.05 &&
        allow(SoundEventType::PantographArc, 0.5))
    {
        SoundEvent ev;
        ev.type = SoundEventType::PantographArc;
        ev.x = pos.x; ev.y = pos.y; ev.z = pos.z + 5.0;
        ev.intensity = std::min(1.0, pantograph.getArcRate() * 2.0);
        ev.vehicle_idx = model_idx;
        out.push_back(ev);
    }

    // Поток песка
    if (sand.isFeeding() && allow(SoundEventType::SandFlow, 0.5))
    {
        SoundEvent ev;
        ev.type = SoundEventType::SandFlow;
        ev.x = pos.x; ev.y = pos.y; ev.z = pos.z;
        ev.intensity = 0.5;
        ev.vehicle_idx = model_idx;
        out.push_back(ev);
    }

    // Свист колодок: высокое прижимное усилие на скорости (перед
    // остановкой)
    if (brake_shoes.isEnabled() && abs(velocity) > 2.0 &&
        abs(velocity) < 8.0)
    {
        for (size_t i = 0; i < num_axis; ++i)
        {
            if (Q_r[i + 1] > 2000.0 &&
                allow(SoundEventType::BrakeSqueal, 0.5))
            {
                SoundEvent ev;
                ev.type = SoundEventType::BrakeSqueal;
                ev.x = pos.x; ev.y = pos.y; ev.z = pos.z;
                ev.intensity = 0.4;
                ev.vehicle_idx = model_idx;
                out.push_back(ev);
                break;
            }
        }
    }

    // Стук на стыках пути (ТЗ "43-47", п.6-8): счётчик ударов осей
    // вертикальной динамики монотонный - новые удары = дельта между
    // опросами. Интенсивность от силы заброса ускорения колёсной пары
    // и скорости (нагруженное колесо бьёт сильнее), частота - от
    // скорости и шага звеньев 25 м
    if (vertical_dynamics.isEnabled())
    {
        const unsigned long joint_hits =
                vertical_dynamics.getJointImpactCount();

        if (joint_hits > last_joint_impacts && abs(velocity) > 1.0)
        {
            // Счётчик монотонный - обновляем всегда (дельта между
            // опросами = новые удары), событие шлём по кулдауну
            if (allow(SoundEventType::JointImpact, 0.1))
            {
                SoundEvent ev;
                ev.type = SoundEventType::JointImpact;
                ev.x = pos.x; ev.y = pos.y; ev.z = pos.z;
                ev.intensity = std::min(1.0,
                    0.5 * vertical_dynamics.getLastJointImpactIntensity() +
                    0.5 * std::min(abs(velocity) / 30.0, 1.0));
                ev.rate_hz = abs(velocity) / 25.0;
                ev.vehicle_idx = model_idx;
                out.push_back(ev);
            }
        }

        last_joint_impacts = joint_hits;
    }

    // Сцепка: рывки силы (выбор зазора) и обрыв (падение усилия)
    if (F_fwd != 0.0 || F_bwd != 0.0 ||
        prev_coupler_fwd != 0.0 || prev_coupler_bwd != 0.0)
    {
        const double jump = std::max(std::abs(F_fwd - prev_coupler_fwd),
                                     std::abs(F_bwd - prev_coupler_bwd));

        // Рывок в сцепке: скачок силы > 100 кН за опрос
        if (jump > 100.0e3 && allow(SoundEventType::CouplerImpact, 0.1))
        {
            SoundEvent ev;
            ev.type = SoundEventType::CouplerImpact;
            ev.x = pos.x; ev.y = pos.y; ev.z = pos.z;
            ev.intensity = std::min(jump / 1.0e6, 1.0);
            ev.vehicle_idx = model_idx;
            out.push_back(ev);
        }

        // Обрыв сцепки: усилие > 50 кН упало ниже 5 кН за опрос
        const bool break_fwd = std::abs(prev_coupler_fwd) > 50.0e3 &&
                               std::abs(F_fwd) < 5.0e3;

        const bool break_bwd = std::abs(prev_coupler_bwd) > 50.0e3 &&
                               std::abs(F_bwd) < 5.0e3;

        if ((break_fwd || break_bwd) &&
            allow(SoundEventType::CouplerBreak, 0.1))
        {
            SoundEvent ev;
            ev.type = SoundEventType::CouplerBreak;
            ev.x = pos.x; ev.y = pos.y; ev.z = pos.z;
            ev.intensity = 1.0;
            ev.vehicle_idx = model_idx;
            out.push_back(ev);
        }

        prev_coupler_fwd = F_fwd;
        prev_coupler_bwd = F_bwd;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Vehicle::isSMELead() const
{
    return sme_lead;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setCatenaryFeed(
        std::function<catenary::FeedState(double, double)> fn)
{
    catenary_feed = std::move(fn);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setRegenAcceptance(double accept_w, bool accepted)
{
    regen_accept_w = std::max(0.0, accept_w);
    regen_accepted = accepted;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Vehicle::getCatenaryFeedActive() const
{
    return static_cast<bool>(catenary_feed);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::repair()
{
    resetDamage();
    resetDerailment();
    resetCollisionState();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float Vehicle::getBodyDamage() const
{
    return body_damage;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float Vehicle::getBogieDamage() const
{
    return bogie_damage;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::resetCollisionState()
{
    if (!is_collided)
        return;

    is_collided = false;

    Journal::instance()->info(QString("Vehicle #%1 collision state cleared").arg(model_idx));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Vehicle::isCollided() const
{
    return is_collided;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setRailHeightSource(VehicleVerticalDynamics::RailHeightFn fn)
{
    vertical_dynamics.setRailHeightSource(std::move(fn));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Vehicle::hasRailHeightSource() const
{
    return vertical_dynamics.isReady();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setLateralOffsetSource(VehicleLateralDynamics::LateralOffsetFn fn)
{
    lateral_dynamics.setLateralOffsetSource(std::move(fn));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Vehicle::hasLateralOffsetSource() const
{
    return lateral_dynamics.isReady();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setLateralCantSource(std::function<double(double)> fn)
{
    cant_source = std::move(fn);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Vehicle::hasCantSource() const
{
    return static_cast<bool>(cant_source);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
WSPSystem& Vehicle::getWSP()
{
    return wsp;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WSPSystem::loadConfig(QString cfg_path, std::size_t num_axis)
{
    CfgReader cfg;
    if (!cfg.load(cfg_path))
        return;

    const QString sec = "WSP";

    cfg.getBool(sec, "Enabled", enabled);

    double value = 0.0;

    if (cfg.getDouble(sec, "SlipThreshold", value))
        slip_threshold = std::max(value, 0.1);
    if (cfg.getDouble(sec, "Period", value))
        period = std::min(std::max(value, 0.05), 2.0);

    axle_phase.assign(num_axis, 0.0);
    axle_releasing.assign(num_axis, false);
    axle_duty_off.assign(num_axis, 0.5);
    prev_slip.assign(num_axis, 0.0);
    axle_eff.assign(num_axis, 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehicleVerticalDynamics& Vehicle::getVerticalDynamics()
{
    return vertical_dynamics;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehicleLateralDynamics& Vehicle::getLateralDynamics()
{
    return lateral_dynamics;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehicleDerailment& Vehicle::getDerailment()
{
    return derailment;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Vehicle::isDerailed() const
{
    return derailment.isDerailed();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::onCouplerJerk(double energy)
{
    if (derailment.isDerailed())
        return;

    // Рывок идёт вдоль сцепки (вдоль пути), но сошедший сосед дёргает
    // под углом: боковая доля ~ половина энергии рывка
    derailment.applyLateralImpact(0.5 * energy,
                                  1.0, 0.0, 0.0,
                                  0.0, 0.0, 1.0, 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::resetDerailment()
{
    derailment.reset();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::getMassCenterHeight() const
{
    return mass_center_height;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::getMassCenterLongitudinal() const
{
    return mass_center_longitudinal;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::getMassCenterLateral() const
{
    return mass_center_lateral;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::calcAxleLoadShare()
{
    axle_load_share.assign(num_axis, 1.0);

    if (num_axis < 2)
        return;

    // Оси равномерно по длине: от -0.35L до +0.35L (крайние тележки).
    // Продольное смещение центра масс перераспределяет нагрузку линейно,
    // смещение ограничено +-25% разности нагрузок крайних осей
    const double spread = std::max(0.7 * length, 1.0);
    const double tan_shift = std::min(std::max(
        mass_center_longitudinal / spread, -0.25), 0.25);

    double sum = 0.0;
    for (size_t i = 0; i < num_axis; ++i)
    {
        // Позиция оси: от -spread/2 (перед) до +spread/2 (зад).
        // При смещении ЦМ к переду передние оси нагружаются сильнее
        const double x = -spread / 2.0 +
                spread * static_cast<double>(i) / static_cast<double>(num_axis - 1);

        axle_load_share[i] = 1.0 - 2.0 * tan_shift * x / spread;
        sum += axle_load_share[i];
    }

    // Нормировка: сумма множителей = num_axis
    if (sum > 1e-6)
    {
        for (size_t i = 0; i < num_axis; ++i)
            axle_load_share[i] *= static_cast<double>(num_axis) / sum;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setFrictionCoeff(double value)
{
    this->psi_coeff = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setDirection(std::int8_t dir)
{
    this->dir = (dir < 0) ? -1 : 1;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::addForwardForce(double value)
{
    this->F_fwd += value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::addBackwardForce(double value)
{
    this->F_bwd += value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setActiveCommonForce(size_t idx, double value)
{
    if (idx < Q_a.size())
        Q_a[idx] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setReactiveCommonForce(size_t idx, double value)
{
    if (idx < Q_r.size())
        Q_r[idx] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setPayloadCoeff(double payload_coeff)
{
    this->payload_coeff = std::clamp(payload_coeff, 0.0, 1.0);
    full_mass = empty_mass + payload_mass * this->payload_coeff;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setTrainCoord(double value)
{
    train_coord = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setVelocity(double value)
{
    velocity = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setWheelAngle(size_t i, double value)
{
    if (i < wheel_rotation_angle.size())
        wheel_rotation_angle[i] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setWheelOmega(size_t i, double value)
{
    if (i < wheel_omega.size())
        wheel_omega[i] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setPrevVehicle(Vehicle* vehicle)
{
    prev_vehicle = vehicle;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setNextVehicle(Vehicle* vehicle)
{
    next_vehicle = vehicle;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setNeedDebugMsg(bool is_needed)
{
    needDebugMsg = is_needed;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString Vehicle::getConfigDir() const
{
    return config_dir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString Vehicle::getConfigName() const
{
    return config_name;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString Vehicle::getModuleDir() const
{
    return module_dir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString Vehicle::getModuleName() const
{
    return module_name;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
size_t Vehicle::getModelIndex() const
{
    return model_idx;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
size_t Vehicle::getTrainIndex() const
{
    return train_idx;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
size_t Vehicle::getStateIndex() const
{
    return state_idx;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
profile_point_t* Vehicle::getProfilePoint()
{
    return &profile_point_data;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::int8_t Vehicle::getDirection() const
{
    return dir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::getMass() const
{
    return full_mass;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::getLength() const
{
    return length;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
size_t Vehicle::getDegressOfFreedom() const
{
    return s;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
size_t Vehicle::getNumAxis() const
{
    return num_axis;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::getWheelDiameter(size_t i) const
{
    if (i < wheel_diameter.size())
        return wheel_diameter[i];
    else
        return 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::getTrainCoord() const
{
    return train_coord;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::getVelocity() const
{
    return velocity;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::getWheelAngle(size_t i)
{
    if (i < wheel_rotation_angle.size())
        return wheel_rotation_angle[i];
    else
        return 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::getWheelOmega(size_t i)
{
    if (i < wheel_omega.size())
        return wheel_omega[i];
    else
        return 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Vehicle* Vehicle::getPrevVehicle()
{
    return prev_vehicle;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Vehicle* Vehicle::getNextVehicle()
{
    return next_vehicle;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float Vehicle::getAnalogSignal(size_t i)
{
    if (i < analogSignal.size())
        return analogSignal[i];
    else
        return 0.0f;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<float>* Vehicle::getAnalogSignals()
{
    return &analogSignal;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
device_list_t* Vehicle::getFwdConnectors()
{
    return &forward_connectors;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
device_list_t* Vehicle::getBwdConnectors()
{
    return &backward_connectors;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
device_coord_list_t* Vehicle::getRailwayConnectors()
{
    return &railway_connectors;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::initBrakeDevices(double p0, double pTM, double pFL)
{
    (void) p0;
    (void) pTM;
    (void) pFL;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::getAcceleration(state_vector_t& Y, state_vector_t& dYdt, const double& t, const double& dt)
{
    (void) t;

    // Body velocity from state vector
    double v = dir * Y[state_idx + s];
    double abs_v = abs(v);

    // Forces from wheels to vehicle body
    double F_wheels = 0.0;
    double R_wheels_fwd = 0.0;
    double R_wheels_bwd = 0.0;
    if (num_axis > 0)
    {
        auto Qa_it = Q_a.begin();
        auto Qr_it = Q_r.begin();
        for (size_t i = 0; i < num_axis; ++i)
        {
            ++Qa_it;
            ++Qr_it;
            // Wheel's angular velocity
            double w = Y[state_idx + s + 1 + i];
            double abs_w = abs(w);
            // Wheel's slip velocity
            double slip = w * rk[i] - v;
            double abs_slip = abs(slip);

            // Active torque
            double wheel_a = *Qa_it;

            // Reactive torque (fade колодок и модуляция WSP применяются
            // при чтении, без мутации хранимого Q_r - иначе многократное
            // умножение на подшагах ОДУ даёт экспоненциальное затухание;
            // WSP в фазе сброса обнуляет момент оси при юзе)
            double potential_r = pf(*Qr_it * brake_fade_eff[i] *
                                    wsp.getAxleEfficiency(i));
            double wheel_r = 0.0;
            if (abs_w > Physics::ZERO)
            {
                // Calculate maximum possible velocity change
                double dw_max = (potential_r / J_axis[i]) * dt;
                if (abs_w >= dw_max)
                {
                    wheel_r = potential_r * sign(w);
                    potential_r = 0.0;
                }
                else
                {
                    // If velocity will fall to zero, reduce reactive force
                    double e = abs_w / dw_max;
                    wheel_r = e * potential_r * sign(w);
                    potential_r = (1 - e) * potential_r;
                }
            }

            // Friction torque
            double potential_f = wheelrailFrictionReducedBySlip(psi[i], abs_slip) * axis_load[i] * rk[i];
            double wheel_f = 0.0;
            if (abs_slip > Physics::ZERO)
            {
                // Calculate maximum possible velocity change
                double dslip_max = (potential_f / J_axis[i]) * dt;
                if (abs_slip >= dslip_max)
                {
                    wheel_f = potential_f * sign(slip);
                    potential_f = 0.0;
                }
                else
                {
                    // If slip will fall to zero, reduce friction force
                    double e = abs_slip / dslip_max;
                    wheel_f = e * potential_f * sign(slip);
                    potential_f = (1 - e) * potential_f;
                }
            }

            // Apply potential reactive forces
            double tmp_r = min(abs(wheel_a - wheel_f), potential_r) * sign(wheel_a - wheel_f);
            wheel_r += tmp_r;

            double tmp_f = min(abs(wheel_a - wheel_r), potential_f) * sign(wheel_a - wheel_r);
            wheel_f += tmp_f;

            // Apply wheel-rail forces to vehicle
            F_wheels += wheel_f / rk[i];
            R_wheels_fwd += min(potential_r - tmp_r, potential_f - tmp_f) / rk[i];
            R_wheels_bwd += min(potential_r + tmp_r, potential_f + tmp_f) / rk[i];

            // Calculate and apply wheel angle acceleration
            dYdt[state_idx + s + 1 + i] = (wheel_a - wheel_r - wheel_f) / J_axis[i];
        }
    }

    // Сила на ободе для учёта энергии (P = F * v, ТЗ "Расход энергии")
    last_wheel_traction = F_wheels;

    // Calculate main resistance force
    double W = mainResist(v);

    // Common body active force
    double force_a = *Q_a.begin() + F_wheels + F_fwd - F_bwd - F_g;

    // Common body reactive force
    double potential_fwd = W + pf(Q_r[0]) + R_wheels_fwd;
    double potential_bwd = W + pf(Q_r[0]) + R_wheels_bwd;
    double force_r = 0.0;
    if (abs_v < Physics::ZERO)
    {
        if (force_a > 0)
        {
            force_r = min(force_a, potential_fwd);
        }
        if (force_a < 0)
        {
            force_r = max(force_a, - potential_bwd);
        }
    }
    else
    {
        if (v > 0)
        {
            force_r = potential_fwd;
        }
        else
        {
            force_r = - potential_bwd;
        }

        // Prediction of velocity
        double dv = ((force_a - force_r) / full_mass) * dt;
        // If velocity will fall to zero, reduce reactive force after it
        if (sign(v) != sign(v + dv))
        {
            double e = abs(v / dv);
            force_r = e * force_r;
            if (force_a > 0.0)
            {
                force_r += (1 - e) * min(force_a, potential_fwd);
            }
            if (force_a < 0.0)
            {
                force_r += (1 - e) * max(force_a, - potential_bwd);
            }
        }
    }

    // Vehicle body's acceleration
    dYdt[state_idx + s + 0] = dir * (force_a - force_r) / full_mass;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::integrationProcess(const simulator_time_t& t, const double& dt)
{
    {
        std::lock_guard lock(keyboard_mutex);
        pressed_keys.clear();
        for (const auto& pressed_keys_at_cab : pressed_keys_by_cabine)
        {
            pressed_keys.insert(pressed_keys_at_cab.begin(), pressed_keys_at_cab.end());
        }
    }

    process(t, dt);

    // Локальные часы для кулдаунов звуковых событий
    sim_clock = t.simulation_seconds;

    //=== Уровни детализации симуляции (ТЗ "Оптимизация", п.2-5) ===
    const bool lod_full = (sim_lod == perf::SimLOD::L0_Full);
    const bool lod_ge1 = (sim_lod <= perf::SimLOD::L1_Simplified);

    // Адаптивные частоты (п.4): термо 10 Гц, износ/эволюция 5 Гц.
    // При прореженном шаге передаём накопленный интервал, а не кадровый
    // dt (иначе эволюция систем замедляется в dt/интервал раз)
    thermal_accum += dt;
    wear_accum += dt;
    double thermal_dt = dt;
    double wear_dt = dt;
    const bool thermal_tick = thermal_accum >= 0.1;
    const bool wear_tick = wear_accum >= 0.2;
    if (thermal_tick)
    {
        thermal_dt = thermal_accum;
        thermal_accum = 0.0;
    }
    if (wear_tick)
    {
        wear_dt = wear_accum;
        wear_accum = 0.0;
    }

    // Продольное ускорение ПЕ (инерционный тангаж кузова, ProdVertKoleb
    // п.4, и реакция машиниста): считаем до шага вертикальной динамики
    if (dt > 1e-4)
    {
        longitudinal_accel = (velocity - prev_velocity) / dt;
        prev_velocity = velocity;
    }

    // Возвышение наружного рельса под центром ПЕ (Б16): источник
    // привязывает Train::slotStep (топология через контроллер ПЕ)
    rail_cant_mm = 0.0;

    if (cant_source)
    {
        rail_cant_mm = cant_source(static_cast<double>(dir) * train_coord);
    }

    // Вертикальная динамика: неровности пути -> подвеска -> кузов (ТЗ).
    // Возвышение наружного рельса входит как смещение сторон в контакте
    // колёс (статический крен), продольное ускорение - как момент
    // тангажа кузова (клюёт носом при торможении).
    // Вызывается из невиртуального метода: работает и при переопределённом
    // дочерними классами process()
    if (vertical_dynamics.isEnabled() && vertical_dynamics.isReady() &&
        lod_ge1)
    {
        vertical_dynamics.step(dt, velocity, dir, train_coord, full_mass,
                               rail_cant_mm, longitudinal_accel);
    }

    // Боковая ветровая нагрузка на кузов (ТЗ "43-47", п.2): сила
    // передаётся в поперечную динамику ДО её шага (смещение кузова,
    // крен и разгрузка колёс тем же механизмом, что центробежная)
    updateWindLoad(t.simulation_seconds);

    // Тоннель: пересчёт погруженности ПЕ в зоны (плавный вход/выход
    // за длину ПЕ; сопротивление складывается в mainResist)
    tunnel_aero.setCoordinate(profile_point_data.railway_coord);
    tunnel_aero.step(dt, velocity, length);

    // Поперечная динамика: виляние от коничности + крип + неровности
    // плана линии (ТЗ "Поперечная динамика"). Трение - текущее
    // колесо-рельс с учётом погодного коэффициента и песка (усреднение
    // по осям: поперечная модель получает один коэффициент трения)
    if (lateral_dynamics.isEnabled() && lateral_dynamics.isReady() &&
        lod_ge1)
    {
        double adhesion_factor = 1.0;

        if (num_axis > 0)
        {
            double sum = 0.0;

            for (size_t i = 0; i < num_axis; ++i)
            {
                sum += adhesion.getAxleFactor(i);
            }

            adhesion_factor = sum / static_cast<double>(num_axis);
        }

        const double friction = psi_coeff * wheelrailFriction(velocity) *
                adhesion_factor;

        // Нагрузки осей от вертикальной динамики (связь систем, ТЗ п.33)
        std::vector<double> load_factors;

        if (vertical_dynamics.isEnabled() && vertical_dynamics.isReady())
        {
            load_factors.resize(num_axis);

            for (size_t i = 0; i < num_axis; ++i)
            {
                load_factors[i] = std::min(
                            std::max(vertical_dynamics.getAxleLoadFactor(i), 0.0),
                            3.0);
            }
        }

        lateral_dynamics.step(dt, velocity, dir, train_coord,
                              profile_point_data.curvature, friction,
                              load_factors, rail_cant_mm);
    }

    // Система схода: постепенная, по датчикам поперечной и вертикальной
    // динамики (ТЗ "Динамика ПС", п.11-12)
    if (derailment.isEnabled())
    {
        derailment.step(dt, lateral_dynamics, vertical_dynamics, num_axis);
    }

    // Движение в сошедшем состоянии разрушает ПЕ (шпалы/балласт)
    if (derailment.isDerailed() && abs(velocity) > 0.1)
    {
        damage_system.applyDerailmentWear(dt, velocity);
    }

    // Опрокидывание: физический критерий по боковому ускорению и центру
    // масс (ТЗ "Физика после схода", п.10)
    if (derailment.isEnabled())
    {
        const bool was_rollover = derailment.isRollover();

        derailment.stepRollover(dt, velocity,
                                lateral_dynamics.getBodyLateralAcceleration());

        // Первое полное опрокидывание - удар всей массой (кузов/двигатели)
        if (derailment.isRollover() && !was_rollover)
        {
            damage_system.applyImpact(0.3 * full_mass * velocity * velocity,
                                      VehicleDamageSystem::ImpactZone::Top);
        }
    }

    //=== Адаптивные частоты (аккумуляторы выше, у LOD-блока) ===

    // Сцепление колёс с рельсами: эволюция поверхности под погодой
    // и самоочисткой проходами осей (ТЗ "Сцепление колёс с рельсами")
    if (lod_full || wear_tick)
    {
        adhesion.step(lod_full ? dt : wear_dt, velocity, num_axis);
    }

    // Интерактивная сцепка: контроль обрыва рукавов при движении
    coupling_interaction.step(dt, velocity, damage_system);

    // Груз и пассажиры: физическое изменение массы ПЕ (ТЗ "Погрузка",
    // п.3, 32): payload = груз + пассажиры; ЦМ груза смещает осевые
    // нагрузки и центр масс (п.4).
    // Полная масса = тара + фактическая масса груза/пассажиров без
    // обрезания по PayloadMass (ТЗ "Продольная динамика", п.2).
    // Если у ПЕ нет динамических систем загрузки - сохраняем
    // коэффициент, заданный конфигом поезда (setPayloadCoeff),
    // иначе он стирался бы на каждом шаге
    {
        const double extra_mass = cargo.getCargoMass() +
                passengers.getPassengerMass();

        const bool dynamic_loading = cargo.isConfigured() ||
                passengers.isConfigured();

        if (dynamic_loading)
        {
            payload_coeff = extra_mass / std::max(payload_mass, 1.0);
            full_mass = empty_mass + extra_mass;
        }
        else
        {
            full_mass = empty_mass + payload_coeff * payload_mass;
        }

        const double total_mass = std::max(full_mass, 1.0);
        const double cargo_share = std::min(extra_mass / total_mass, 1.0);
        mass_center_longitudinal = mass_center_longitudinal_base +
                cargo.getLongitudinalShift() * cargo_share;
        mass_center_lateral = mass_center_lateral_base +
                cargo.getLateralShift() * cargo_share;

        calcAxleLoadShare();
        vertical_dynamics.setStaticAxleLoads(axle_load_share);
    }

    // Шаг пассажирских потоков (посадка/высадка при открытых дверях)
    passengers.step(dt);

    // Лобовое стекло: грязь от осадков/брызг, дворники/омыватель, лёд
    if (lod_full)
    {
        windshield.step(dt, velocity, rain_intensity,
                        adhesion.getWetness(),
                        adhesion.getAirTemperature());
    }

    // Физическая реакция машиниста: фильтр «шея» на ускорениях кузова
    // (продольное ускорение вычислено выше - до шагов динамики)
    camera_motion.step(dt,
                       vertical_dynamics.getBodyAcceleration(),
                       lateral_dynamics.getBodyLateralAcceleration(),
                       longitudinal_accel,
                       vertical_dynamics.getBodyPitch(),
                       vertical_dynamics.getBodyRoll());

    // Деповское питание и батарея: заряд от внешнего источника или
    // генератора, разряд от вспомогательных потребителей (ТЗ
    // "Деповское питание"). Стояночная нагрузка низковольтной сети
    {
        const double aux_load = diesel.isRunning() ? 0.0 : 2000.0;
        depot_power.step(dt, aux_load, diesel.isRunning(),
                         abs(velocity) > 0.1);
    }

    // Снабжение (ТЗ "Снабжение локомотива"): порционная подача
    // топлива/масла/ОЖ/песка через штатные API систем-приёмников.
    // Движение при подключении обрывает рукав (Critical, отмена);
    // блокировка тяги - isMovementBlocked() у потребителя
    service_system.step(dt, velocity, diesel, sand);

    // Конденсат пневмосистемы (ТЗ "Конденсат/влажность"): точка росы
    // по Магнусу, влага при зарядке магистрали, замерзание/оттаивание
    // (термо-тик). Деградация тормозной волны - через
    // getBrakeResponseFactor() у тормозных устройств
    if (lod_full || thermal_tick)
    {
        condensate_system.step(lod_full ? dt : thermal_dt,
                               adhesion.getAirTemperature(),
                               adhesion.getHumidity(),
                               !derailment.isDerailed(),
                               abs(velocity) < 0.3);
    }

    // Дизель: тепловая модель, расход, дымность (ТЗ "Тепловая модель
    // дизеля"). Работает только при запущенном двигателе (start/stop -
    // от кабины тепловоза). Температура воздуха - от погодной системы
    if (diesel.isRunning() && (lod_full || thermal_tick))
    {
        diesel.step(lod_full ? dt : thermal_dt,
                    adhesion.getAirTemperature(), abs(velocity));
    }

    // Учёт электроэнергии от фактической силы на ободе (ТЗ "Расход
    // топлива и электроэнергии"). Приём рекуперации ограничен сетью
    // (подстанция + потребители секции, ТЗ "Рекуперация", п.5-7)
    energy.step(dt, last_wheel_traction, velocity, Uks,
                regen_accept_w, regen_accepted);

    // Питание от КС через токоприёмник (ТЗ "Контактная сеть"):
    // напряжение в точке с провалом от тока/расстояния до подстанции;
    // потеря контакта/нейтральная вставка -> 0
    if (pantograph.isRaised() && catenary_feed)
    {
        const catenary::FeedState feed =
                catenary_feed(profile_point_data.railway_coord,
                              energy.getCurrent());

        pantograph.step(dt, velocity, wind_speed_for_pantograph,
                        feed.voltage);

        Uks = (pantograph.isContactOk() && feed.powered) ? feed.voltage : 0.0;

        // Проезд нейтральной вставки под током (п.21-23 ТЗ #17):
        // включённый БВ при проходе нейтралки - ошибка машиниста
        if (feed.in_neutral && energy.getCurrent() > 30.0)
        {
            Uks = 0.0;

            if (!neutral_fault_reported)
            {
                neutral_fault_reported = true;
                Journal::instance()->critical(QString(
                    "[CATENARY] Vehicle #%1 crossing NEUTRAL INSERT "
                    "under power - main switch fault!")
                    .arg(model_idx));

                damage_system.addDamage(
                            VehicleDamageSystem::Component::Electrical, 0.3);
            }
        }
        else if (!feed.in_neutral)
        {
            neutral_fault_reported = false;
        }
    }
    else
    {
        // Токоприёмник опущен / нет КС - питания нет (сброс, иначе
        // Uks "залипал" на последнем значении)
        Uks = 0.0;
    }

    // Тормозные колодки: нагрев от реальной работы тормоза, охлаждение,
    // износ, fade (ТЗ "Тормозные колодки"). Температура воздуха - от
    // погоды (охлаждение зависит от среды, п.3/17)
    if (brake_shoes.isEnabled() && (lod_full || thermal_tick))
    {
        std::vector<double> brake_torques(num_axis);
        for (size_t i = 0; i < num_axis; ++i)
            brake_torques[i] = Q_r[i + 1];

        brake_shoes.step(lod_full ? dt : thermal_dt, brake_torques,
                         wheel_omega, velocity,
                         adhesion.getAirTemperature());
    }

    // Fade колодок для ОДУ: применяется при чтении Q_r в
    // getAcceleration (без мутации Q_r на подшагах)
    if (brake_shoes.isEnabled() && brake_shoes_apply_reactive)
    {
        if (brake_fade_eff.size() < num_axis)
            brake_fade_eff.assign(num_axis, 1.0);

        for (size_t i = 0; i < num_axis; ++i)
            brake_fade_eff[i] = brake_shoes.getAxleEfficiency(i);
    }
    else if (brake_fade_eff.size() >= num_axis)
    {
        for (size_t i = 0; i < num_axis; ++i)
            brake_fade_eff[i] = 1.0;
    }

    // Противоюзная система (ТЗ "Сцепление", п.10-11): модуляция
    // тормозного момента осей при юзе. Множитель применяется в
    // getAcceleration при чтении Q_r (по образцу brake_fade_eff),
    // сам Q_r не мутируется
    wsp.step(dt, velocity, wheel_omega, rk);

    // Пескоподача: расход из бункера, эффект на сцепление осей
    // (ТЗ "Подача песка"). Автоматика - по пробуксовке осей.
    // Только у ПЕ с настроенной секцией [Sand] (локомотивы)
    if (sand.isEnabled())
    {
        bool slip = false;

        if (abs(velocity) > 0.5)
        {
            for (size_t i = 0; i < num_axis; ++i)
            {
                const double rim = wheel_omega[i] * rk[i];
                if (std::abs(rim - velocity) > 0.15 * std::abs(velocity))
                {
                    slip = true;
                    break;
                }
            }
        }

        sand.step(dt, velocity, slip, adhesion,
                  adhesion.getWetness(), adhesion.getIce(),
                  adhesion.getContamination(),
                  adhesion.getHumidity(),
                  adhesion.getAirTemperature());
    }

    // Ползуны: образование при юзе + периодические удары при обороте
    // колёсной пары (ТЗ "Ползун")
    if (flat_spots.isEnabled() && (lod_full || wear_tick))
    {
        flat_spots.step(lod_full ? dt : wear_dt, wheel_rotation_angle,
                        wheel_omega, rk, velocity, vertical_dynamics);
    }

    // Износ колёсных пар (ТЗ "43-47", п.1): аккумуляторы энергии
    // заполняются каждый кадр (дёшево), пересчёт износа - на wear-тике
    // (ТЗ "Оптимизация", п.16)
    if (wheel_wear.isEnabled() && num_axis > 0)
    {
        for (size_t i = 0; i < num_axis; ++i)
        {
            // Энергия проскальзывания (боксование/юз): сила трения
            // скольжения x скорость скольжения колеса о рельс
            const double slip = wheel_omega[i] * rk[i] - velocity;

            if (std::abs(slip) > 0.1)
            {
                const double slip_force = psi[i] * axis_load[i];
                wheel_wear.addAxleSlipEnergy(i,
                        std::abs(slip_force * slip) * dt);
            }

            // Работа тормозных колодок: момент x угловая скорость
            if (Q_r.size() > i + 1)
            {
                const double brake_work =
                        std::abs(Q_r[i + 1] * wheel_omega[i]);

                if (brake_work > 0.0)
                    wheel_wear.addAxleBrakeEnergy(i, brake_work * dt);
            }
        }

        if (lod_full || wear_tick)
        {
            wheel_wear.step(lod_full ? dt : wear_dt,
                            velocity,
                            full_mass,
                            profile_point_data.curvature,
                            brake_shoes.isEnabled()
                                ? brake_shoes.getMaxTemperature()
                                : 0.0,
                            adhesion.getContamination());

            // Износ -> профиль -> виляние: коничность в поперечную
            // динамику (звено было разорвано). Пересчёт критической
            // скорости дорогой - обновляем только при заметном изменении
            const double conicity = wheel_wear.getConicity();

            if (std::abs(conicity - lateral_dynamics.getConicity()) > 0.005)
                lateral_dynamics.setConicity(conicity);
        }
    }

    // Опасный груз: утечки/пожар/взрыв. Пожар соседней ПЕ - источник
    // воспламенения (цепная реакция ограничена радиусом и состоянием)
    if (hazard.getCargoType() != VehicleHazard::CargoType::Normal)
    {
        bool fire_nearby = false;

        const Vehicle* prev = getPrevVehicle();
        const Vehicle* next = getNextVehicle();

        if (prev != nullptr && prev->getHazard().isOnFire())
            fire_nearby = true;
        if (next != nullptr && next->getHazard().isOnFire())
            fire_nearby = true;

        hazard.step(dt,
                    damage_system.getDamage(VehicleDamageSystem::Component::Tank),
                    damage_system.getDamage(VehicleDamageSystem::Component::Body),
                    fire_nearby);
    }

    // Синхронизация унаследованных полей повреждений с компонентной
    // системой (используются в integrationPreStep и геттерах)
    body_damage = static_cast<float>(
                damage_system.getDamage(VehicleDamageSystem::Component::Body));
    bogie_damage = static_cast<float>(
                damage_system.getDamage(VehicleDamageSystem::Component::Bogie));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::integrationPreStep(state_vector_t& Y, const double& t)
{
    train_coord = Y[state_idx];
    velocity = dir * Y[state_idx + s];

    // Calculate gravity force from profile inclination
    double weight = full_mass * Physics::g;
    double sin_beta = profile_point_data.inclination / 1000.0;
    F_g = weight * sin_beta;

    F_fwd = 0.0;
    F_bwd = 0.0;

    // Аварийное торможение после столкновения с препятствием:
    // тормозящая сила с постоянной времени ~0.5 с
    if (is_collided)
    {
        F_fwd -= velocity * full_mass * 2.0;
    }

    // Влияние повреждений ходовой на ходовые качества
    if (bogie_damage >= 1.0f)
    {
        // Ходовая разрушена: нормальное движение невозможно
        F_fwd -= velocity * full_mass * 2.0;
    }
    else if (bogie_damage > 0.0f)
    {
        // Дополнительное сопротивление движению (доля от веса)
        const double damage_resist =
            bogie_damage * damage_resist_coeff * full_mass * Physics::g;

        if (abs(velocity) > Physics::ZERO)
            F_fwd -= sign(velocity) * damage_resist;
    }

    // Движение после схода с рельсов: огромное сопротивление движению
    // по шпалам/балласту + интенсивное замедление (ТЗ "Динамика ПС", п.13;
    // уточняется в ТЗ "Физика после схода")
    if (derailment.isDerailed())
    {
        const double derailed_resist =
            derailment.getResistanceCoefficient() * full_mass * Physics::g;

        if (abs(velocity) > Physics::ZERO)
            F_fwd -= sign(velocity) * derailed_resist;

        // Аварийное торможение сошедшей ПЕ
        F_fwd -= velocity * full_mass * 0.8;
    }

    if (num_axis > 0)
    {
        // Wheel-rail friction coefficient: базовая кривая + погодный
        // коэффициент состава + состояние поверхности рельса по осям
        double psi_v = psi_coeff * wheelrailFriction(velocity);

        for (size_t i = 0; i < num_axis; i++)
        {
            wheel_rotation_angle[i] = Y[state_idx + 1 + i];
            wheel_omega[i] = Y[state_idx + s + 1 + i];
            psi[i] = psi_v * adhesion.getAxleFactor(i);
            axis_load[i] = weight / static_cast<double>(num_axis);
        }

        // Динамическая нагрузка осей от вертикальной динамики: влияет на
        // реализуемое сцепление колеса с рельсом (разгрузка оси при
        // колебаниях снижает силу трения)
        if (vertical_dynamics.isEnabled() && vertical_dynamics.isReady())
        {
            for (size_t i = 0; i < num_axis; i++)
            {
                const double factor = vertical_dynamics.getAxleLoadFactor(i);
                axis_load[i] *= std::min(std::max(factor, 0.0), 3.0);
            }
        }

        // Статическая перегрузка осей от продольного смещения центра масс
        if (!axle_load_share.empty())
        {
            for (size_t i = 0; i < num_axis; i++)
                axis_load[i] *= axle_load_share[i];
        }
    }

    preStep(t);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::integrationStep(state_vector_t& Y, const double& t, const double& dt)
{
    (void) Y;
    step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::integrationPostStep(state_vector_t& Y, const double& t)
{
    (void) Y;
    postStep(t);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString Vehicle::getDebugMsg() const
{
    return DebugMsg;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setKeyboardControl(const uint8_t& cab_num, const std::vector<uint16_t>& pressed_keys)
{
    if (cab_num >= pressed_keys_by_cabine.size())
        return;

    std::lock_guard lock(keyboard_mutex);
    pressed_keys_by_cabine[cab_num].insert(pressed_keys.begin(), pressed_keys.end());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::resetKeyboardControl(const uint8_t& cab_num)
{
    if (cab_num >= pressed_keys_by_cabine.size())
        return;

    std::lock_guard lock(keyboard_mutex);
    pressed_keys_by_cabine[cab_num].clear();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setControlSignals(const control_signals_t& control_signals)
{
    this->control_signals = control_signals;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
feedback_signals_t& Vehicle::getFeedBackSignals()
{
    return feedback_signals;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setBrakeShoesState(bool state)
{
    is_brake_shoes = state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setCurrentKind(int value)
{
    current_kind = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::setUks(double value)
{
    Uks = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::initialization()
{
    // This code may be overrided in child class
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::loadConfig(QString cfg_path)
{
    (void) cfg_path;

    // This code may be overrided in child class
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::process(const simulator_time_t &t, const double &dt)
{
    (void) t;
    (void) dt;

    // This code may be overrided in child class
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::preStep(const double& t)
{
    (void) t;

    // This code may be overrided in child class
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::step(const double& t, const double& dt)
{
    (void) t;
    (void) dt;

    // This code may be overrided in child class
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::postStep(const double& t)
{
    (void) t;

    // This code may be overrided in child class
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::mainResistCoeffs()
{
    double q = 1.0;
    if ((q0 > 1.0) && (num_axis > 0))
        q = full_mass / (1000.0 * static_cast<double>(num_axis));

    W_coef = (b0 + b1 / q) * Physics::g / 1000.0;
    W_coef_v = (b2 / q) * Physics::g * Physics::kmh / 1000.0;
    W_coef_v2 = (b3 / q) * Physics::g * Physics::kmh * Physics::kmh / 1000.0;
    W_coef_curv = 700.0 * Physics::g / 1000.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::mainResist(const double& velocity)
{
    // Основное сопротивление + "воздушный поршень" тоннеля (ТЗ
    // "43-47", п.3): прибавка плавно растёт при входе ПЕ в тоннель
    // и падает при выходе (getResistanceForce = 0 вне зон)
    return full_mass * (  W_coef
                        + W_coef_v * std::abs(velocity)
                        + W_coef_v2 * velocity * velocity
                        + W_coef_curv * std::abs(profile_point_data.curvature))
            + tunnel_aero.getResistanceForce(velocity);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::wheelrailFriction(const double& velocity)
{
    double abs_V = std::abs(velocity) * Physics::kmh;
    return psi_a + (psi_b / (psi_c + psi_d * abs_V)) + psi_e * abs_V;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Vehicle::wheelrailFrictionReducedBySlip(const double &psi, const double& slip_velocity)
{
    return psi / (1.0 + std::tanh(slip_velocity));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::addFwdConnector(Device* device)
{
    forward_connectors.push_back(device);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::addBwdConnector(Device* device)
{
    backward_connectors.push_back(device);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::addRailwayConnector(Device* device, double distance_from_center)
{
    device_coord_t dc;
    dc.device = device;
    dc.coord = std::clamp(distance_from_center, -length / 2.0, length / 2.0);
    railway_connectors.push_back(dc);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Vehicle::isShift(int cab_num) const
{
    return getKeyState(KEY_Shift_L, cab_num) || getKeyState(KEY_Shift_R, cab_num);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Vehicle::isControl(int cab_num) const
{
    return getKeyState(KEY_Control_L, cab_num) || getKeyState(KEY_Control_R, cab_num);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Vehicle::isAlt(int cab_num) const
{
    return getKeyState(KEY_Alt_L, cab_num) || getKeyState(KEY_Alt_R, cab_num);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Vehicle::getKeyState(uint16_t key, int cab_num) const
{
    if (cab_num < 0)
    {
        return pressed_keys.count(key);
    }

    if (cab_num >= pressed_keys_by_cabine.size())
    {
        return false;
    }

    return pressed_keys_by_cabine[cab_num].count(key);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::loadConfiguration(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        Journal::instance()->info("Loaded config file: " + cfg_path);

        QString secName = "Vehicle";

        double diameter = 0.0;
        double J = 0.0;
        int axis = 0;
        cfg.getDouble(secName, "EmptyMass", empty_mass);
        cfg.getDouble(secName, "PayloadMass", payload_mass);
        cfg.getDouble(secName, "Length", length);
        cfg.getInt(secName, "NumAxis", axis);
        cfg.getDouble(secName, "WheelDiameter", diameter);
        cfg.getDouble(secName, "WheelInertia", J);

        Journal::instance()->info(QString("EmptyMass: %1 kg").arg(empty_mass));
        Journal::instance()->info(QString("PayloadMass: %1 kg").arg(payload_mass));
        Journal::instance()->info(QString("Length: %1 m").arg(length));

        full_mass = empty_mass + payload_mass * payload_coeff;

        if (axis > 0)
        {
            num_axis = static_cast<size_t>(axis);

            wheel_rotation_angle.resize(num_axis);
            wheel_rotation_angle.shrink_to_fit();
            std::fill(wheel_rotation_angle.begin(), wheel_rotation_angle.end(), 0.0);

            wheel_omega.resize(num_axis);
            wheel_omega.shrink_to_fit();
            std::fill(wheel_omega.begin(), wheel_omega.end(), 0.0);

            wheel_diameter.resize(num_axis);
            wheel_diameter.shrink_to_fit();
            std::fill(wheel_diameter.begin(), wheel_diameter.end(), diameter);

            double tmp = diameter / 2.0;
            rk.resize(num_axis);
            rk.shrink_to_fit();
            std::fill(rk.begin(), rk.end(), tmp);

            J_axis.resize(num_axis);
            J_axis.shrink_to_fit();
            std::fill(J_axis.begin(), J_axis.end(), J);

            tmp = full_mass * Physics::g / static_cast<double>(num_axis);
            axis_load.resize(num_axis);
            axis_load.shrink_to_fit();
            std::fill(axis_load.begin(), axis_load.end(), tmp);

            psi.resize(num_axis);
            psi.shrink_to_fit();
            std::fill(psi.begin(), psi.end(), 0.30);

            Journal::instance()->info(QString("NumAxis: %1").arg(num_axis));
            Journal::instance()->info(QString("WheelDiameter: %1 m").arg(diameter));
            Journal::instance()->info(QString("WheelInertia: %1 kg*m^2").arg(J));
        }
        else
        {
            num_axis = 0;
            Journal::instance()->warning(QString("NumAxis is zero. Wheel's model will't used"));
        }

        QString main_resist_cfg = "";
        cfg.getString(secName, "MainResist", main_resist_cfg);
        loadMainResist(cfg_path, main_resist_cfg);

        QString wheel_cfg = "default";
        cfg.getString(secName, "WheelRailFriction", wheel_cfg);
        loadWheelRailFriction(cfg_path, wheel_cfg);

        s = 1 + num_axis;

        // Коллайдеры ПЕ (секция Collision)
        colliders.loadConfig(cfg_path, length, num_axis, diameter);

        // Вертикальная динамика ПС (секция Suspension)
        vertical_dynamics.loadConfig(cfg_path, full_mass, num_axis, length);

        // Поперечная динамика ПС (секция LateralDynamics)
        lateral_dynamics.loadConfig(cfg_path, full_mass, num_axis,
                                    length, diameter);

        // Система схода (секция Derailment)
        derailment.loadConfig(cfg_path, num_axis, model_idx);

        // Компонентные повреждения и опасный груз
        damage_system.loadConfig(cfg_path);
        hazard.loadConfig(cfg_path);

        // Ползуны колёсных пар (секция FlatSpot)
        flat_spots.loadConfig(cfg_path, num_axis);

        // Сцепление колёс с рельсами (секция Adhesion)
        adhesion.loadConfig(cfg_path);

        // Пескоподача (секция Sand)
        sand.loadConfig(cfg_path, num_axis);

        // Тормозные колодки (секция BrakeShoes)
        brake_shoes.loadConfig(cfg_path, num_axis);

        // Дизель (секция Diesel; тепловозы)
        diesel.loadConfig(cfg_path);

        // Учёт энергии (секция Energy)
        energy.loadConfig(cfg_path);

        // Токоприёмник (секция Pantograph)
        pantograph.loadConfig(cfg_path);

        // Деповское питание и АБ (секции DepotPower/Battery)
        depot_power.loadConfig(cfg_path);

        // Интерактивная сцепка (секция CouplingInteraction)
        coupling_interaction.loadConfig(cfg_path);

        // Лобовое стекло (секция Windshield)
        windshield.loadConfig(cfg_path);

        // Реакция машиниста (секция CameraMotion)
        camera_motion.loadConfig(cfg_path);

        // Реестр интерактивной кабины (секции CabElement)
        cab_interaction.loadConfig(cfg_path);

        // Грузовая система (секция CargoWagon)
        cargo.loadConfig(cfg_path);

        // Пассажирская система (секция PassengerCar)
        passengers.loadConfig(cfg_path);

        // Система снабжения (секция Service)
        service_system.loadConfig(cfg_path);

        // Конденсат пневмосистемы (секция PneumoCondensate)
        condensate_system.loadConfig(cfg_path);

        // Износ колёсных пар (секция WheelWear). База коничности -
        // из настроек поперечной динамики ([LateralDynamics] Conicity)
        wheel_wear.loadConfig(cfg_path, num_axis, diameter);
        wheel_wear.setBaseConicity(lateral_dynamics.getConicity());

        // Аэродинамика тоннеля (секция TunnelAero; зоны задаёт модель)
        tunnel_aero.loadConfig(cfg_path);

        // Противоюзная система (секция WSP; ТЗ "Сцепление", п.10-11)
        wsp.loadConfig(cfg_path, num_axis);

        // Ветровая нагрузка на кузов (секция WindLoad)
        {
            cfg.getBool("WindLoad", "Enabled", wind_load_enabled);
            cfg.getDouble("WindLoad", "LateralArea", wind_lateral_area);
            cfg.getDouble("WindLoad", "DragCoeff", wind_drag_coeff);
            cfg.getDouble("WindLoad", "AirDensity", wind_air_density);
            cfg.getDouble("WindLoad", "GustPeriod", wind_gust_period);
            cfg.getDouble("WindLoad", "GustMin", wind_gust_min);
            cfg.getDouble("WindLoad", "AppHeight", wind_app_height);

            wind_gust_period = std::max(wind_gust_period, 5.0);
            wind_gust_min = std::min(std::max(wind_gust_min, 0.1), 1.0);
            wind_app_height = std::max(wind_app_height, 0.5);
        }

        // Интерактивная сцепка (секция CouplingInteraction)
        coupling_interaction.loadConfig(cfg_path);

        // Система многих единиц (секция SME)
        {
            int ivalue = 0;
            cfg.getInt("SME", "GroupId", ivalue);
            sme_group = ivalue;
            cfg.getBool("SME", "IsLead", sme_lead);
        }
        cfg.getBool("BrakeShoes", "ApplyToReactiveForce",
                    brake_shoes_apply_reactive);

        // Центр масс для критерия опрокидывания
        derailment.setMassCenterHeight(mass_center_height);
        // Центр масс (секция MassCenter) + распределение нагрузки по осям
        {
            double value = 0.0;
            if (cfg.getDouble("MassCenter", "Height", value))
                mass_center_height = value;
            if (cfg.getDouble("MassCenter", "Longitudinal", value))
                mass_center_longitudinal = value;
            if (cfg.getDouble("MassCenter", "Lateral", value))
                mass_center_lateral = value;

            mass_center_longitudinal_base = mass_center_longitudinal;
            mass_center_lateral_base = mass_center_lateral;

            calcAxleLoadShare();
            vertical_dynamics.setStaticAxleLoads(axle_load_share);
        }

        // Пороги повреждений (секция Damage)
        cfg.getDouble("Damage", "BodyDamageThreshold", body_damage_threshold);
        cfg.getDouble("Damage", "BogieDamageThreshold", bogie_damage_threshold);
        cfg.getDouble("Damage", "DamageResistanceCoeff", damage_resist_coeff);

        // User defined configuration load
        loadConfig(cfg_path);
    }
    else
    {
        Journal::instance()->error("File " + cfg_path + " is't found");
    }

    auto cabNode = cfg.getFirstSection("Cabine");

    QFileInfo cfgFileInfo(cfg_path);
    QString cfg_dir = cfgFileInfo.absolutePath();

    while (!cabNode.isNull())
    {
        QString cabine_cfg_name = "";
        cfg.getString(cabNode, "IOControllerConfig", cabine_cfg_name);

        CfgReader cabCfg;

        QMap<int, float> inputs;

        QString cabCfgPath = cfg_dir + QDir::separator() + cabine_cfg_name + ".xml";

        if (cabCfg.load(cabCfgPath))
        {
            auto controlNode = cabCfg.getFirstSection("Control");

            while (!controlNode.isNull())
            {
                int id = 0;
                cabCfg.getInt(controlNode, "ID", id);

                double value = 0.0;
                cabCfg.getDouble(controlNode, "value", value);

                inputs.insert(id, static_cast<float>(value));

                Journal::instance()->info(QString("Init control ID=%1 value=%2").arg(id, 4).arg(value, 5, 'f', 2));

                controlNode = cabCfg.getNextSection();
            }
        }
        else
        {
            Journal::instance()->error("File " + cabCfgPath + " not found");
        }

        control_inputs.push_back(inputs);

        cabNode = cfg.getNextSection();
    }

    Q_a.resize(s);
    Q_a.shrink_to_fit();
    std::fill(Q_a.begin(), Q_a.end(), 0.0);

    Q_r.resize(s);
    Q_r.shrink_to_fit();
    std::fill(Q_r.begin(), Q_r.end(), 0.0);

    brake_fade_eff.assign(num_axis, 1.0);
    brake_fade_eff.shrink_to_fit();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::updateWindLoad(double time_s)
{
    // Нет поперечной динамики или данных о ветре - разгружаем канал
    if (!wind_load_enabled || !lateral_dynamics.isEnabled() ||
            wind_speed_for_pantograph <= 0.1)
    {
        if (lateral_dynamics.getWindLateralForce() != 0.0)
            lateral_dynamics.setWindLateralForce(0.0, wind_app_height);

        return;
    }

    // Боковая площадь кузова: из конфига или длина x типовая высота
    // габарита кузова 3.7 м (ТЗ "43-47", п.2)
    const double area = (wind_lateral_area > 0.0)
            ? wind_lateral_area
            : length * 3.7;

    // Скорость потока: скорость ветра от погоды (м/с)
    const double v = wind_speed_for_pantograph;

    // F = 0.5 * rho * Cd * A * V^2 (ТЗ "43-47", п.2)
    const double force = 0.5 * wind_air_density * wind_drag_coeff *
            area * v * v;

    // Порывы/направление: детерминированная модуляция по времени
    // (сдвиг фазы по индексу ПЕ - соседние кузова не качатся синхронно,
    // ТЗ "43-47", п.2 "порывы"). Знак задаёт сторону (вправо/влево)
    const double phase = 2.0 * Physics::PI * time_s / wind_gust_period +
            1.7 * static_cast<double>(model_idx);

    const double gust = wind_gust_min +
            (1.0 - wind_gust_min) * std::sin(phase);

    lateral_dynamics.setWindLateralForce(force * gust, wind_app_height);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::loadMainResist(QString cfg_path, QString main_resist_cfg)
{
    QFileInfo info(cfg_path);
    QDir dir(info.path());
    dir.cdUp();
    dir.cdUp();
    QString file_path = dir.path() + QDir::separator() +
                        "main-resist" + QDir::separator() +
                        main_resist_cfg + ".xml";

    CfgReader cfg;

    if (cfg.load(file_path))
    {
        QString secName = "MainResist";

        cfg.getDouble(secName, "b0", b0);
        cfg.getDouble(secName, "b1", b1);
        cfg.getDouble(secName, "b2", b2);
        cfg.getDouble(secName, "b3", b3);
        cfg.getDouble(secName, "q0", q0);

        mainResistCoeffs();
    }
    else
    {
        Journal::instance()->error("File " + file_path + " is't found");
    }
    Journal::instance()->info("Main resist formula: " + QString("w = %1 + (%2 + %3 * V + %4 * V^2) / %5")
                                                            .arg(b0).arg(b1).arg(b2).arg(b3).arg(q0));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Vehicle::loadWheelRailFriction(QString cfg_path, QString wheel_cfg)
{
    QFileInfo info(cfg_path);
    QDir dir(info.path());
    dir.cdUp();
    dir.cdUp();
    QString file_path = dir.path() + QDir::separator() +
                        "wheel-rail-friction" + QDir::separator() +
                        wheel_cfg + ".xml";

    CfgReader cfg;

    if (cfg.load(file_path))
    {
        QString secName = "WheelModel";

        cfg.getDouble(secName, "a", psi_a);
        cfg.getDouble(secName, "b", psi_b);
        cfg.getDouble(secName, "c", psi_c);
        cfg.getDouble(secName, "d", psi_d);
        cfg.getDouble(secName, "e", psi_e);

        Journal::instance()->info("Wheel model config: " + QString("%1")
                                                               .arg(file_path));
    }
    else
    {
        Journal::instance()->error("File " + file_path + " is't found");
    }
    Journal::instance()->info("Wheel friction coefficient formula: " + QString("psi = %1 + (%2 / (%3 + %4 * V)) + %5 * V")
                                                                           .arg(psi_a).arg(psi_b).arg(psi_c).arg(psi_d).arg(psi_e));
}
