//------------------------------------------------------------------------------
//
//      Collision detection engine for RRS (based on Jolt Physics)
//      Collision world wrapper over Jolt PhysicsSystem
//
//------------------------------------------------------------------------------

#include    "collision-world.h"

#include    "collision-shape-impl.h"

#include    <Jolt/Jolt.h>

#include    <Jolt/RegisterTypes.h>
#include    <Jolt/Core/Factory.h>
#include    <Jolt/Core/TempAllocator.h>
#include    <Jolt/Core/JobSystemThreadPool.h>
#include    <Jolt/Math/Vec3.h>
#include    <Jolt/Math/Quat.h>
#include    <Jolt/Physics/PhysicsSettings.h>
#include    <Jolt/Physics/PhysicsSystem.h>
#include    <Jolt/Physics/EActivation.h>
#include    <Jolt/Physics/Body/Body.h>
#include    <Jolt/Physics/Body/BodyID.h>
#include    <Jolt/Physics/Body/BodyInterface.h>
#include    <Jolt/Physics/Body/BodyLock.h>
#include    <Jolt/Physics/Body/BodyCreationSettings.h>
#include    <Jolt/Physics/Body/BodyActivationListener.h>
#include    <Jolt/Physics/Body/MotionType.h>
#include    <Jolt/Physics/Body/MotionProperties.h>
#include    <Jolt/Physics/Collision/ContactListener.h>
#include    <Jolt/Physics/Collision/EstimateCollisionResponse.h>
#include    <Jolt/Physics/Collision/Shape/SubShapeIDPair.h>
#include    <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include    <Jolt/Physics/Collision/NarrowPhaseQuery.h>
#include    <Jolt/Physics/Collision/RayCast.h>
#include    <Jolt/Physics/Collision/CastResult.h>

#include    <atomic>
#include    <thread>
#include    <unordered_map>

#include    <algorithm>

using namespace collision;

namespace
{

//------------------------------------------------------------------------------
// BroadPhase-бакеты: статика со статикой не проверяется вовсе
//------------------------------------------------------------------------------
namespace BroadPhaseLayers
{
    static constexpr JPH::BroadPhaseLayer STATIC(0);
    static constexpr JPH::BroadPhaseLayer MOVING(1);
    static constexpr JPH::uint COUNT = 2;
}

JPH::BroadPhaseLayer broadPhaseFor(Layer layer)
{
    switch (layer)
    {
    case Layer::Train:
    case Layer::Bogie:
    case Layer::Wheel:
    case Layer::RoadVehicle:
    case Layer::Player:
        return BroadPhaseLayers::MOVING;

    default:
        return BroadPhaseLayers::STATIC;
    }
}

JPH::EMotionType toJoltMotionType(MotionType motion)
{
    switch (motion)
    {
    case MotionType::Static:    return JPH::EMotionType::Static;
    case MotionType::Kinematic: return JPH::EMotionType::Kinematic;
    case MotionType::Dynamic:   return JPH::EMotionType::Dynamic;
    }
    return JPH::EMotionType::Static;
}

/// Качество движения (CCD), Jolt v5.6.0: EMotionQuality::LinearCast
/// (имя значения в vendored Jolt - LinearCast, не Linear)
JPH::EMotionQuality toJoltMotionQuality(MotionQuality quality)
{
    switch (quality)
    {
    case MotionQuality::Discrete:   return JPH::EMotionQuality::Discrete;
    case MotionQuality::LinearCast: return JPH::EMotionQuality::LinearCast;
    }
    return JPH::EMotionQuality::Discrete;
}

/// Данные, привязанные к телу (указатель на BodyInfo лежит в Body::UserData)
struct BodyInfo
{
    void*   user_data = nullptr;
    GroupId group = no_group;
};

BodyInfo* bodyInfoOf(const JPH::Body& body)
{
    return reinterpret_cast<BodyInfo*>(
        static_cast<std::uintptr_t>(body.GetUserData()));
}

std::uint32_t packedId(const JPH::BodyID& id)
{
    return id.GetIndexAndSequenceNumber();
}

//------------------------------------------------------------------------------
/// Отображение слоёв коллизий на BroadPhase-бакеты
//------------------------------------------------------------------------------
class BroadPhaseLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:

    JPH::uint GetNumBroadPhaseLayers() const override
    {
        return BroadPhaseLayers::COUNT;
    }

    JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
    {
        return broadPhaseFor(static_cast<Layer>(inLayer));
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
    {
        switch (static_cast<JPH::BroadPhaseLayer::Type>(inLayer))
        {
        case 0:     return "STATIC";
        case 1:     return "MOVING";
        default:    return "UNKNOWN";
        }
    }
#endif
};

//------------------------------------------------------------------------------
/// Грубый фильтр: слой против BroadPhase-бакета
//------------------------------------------------------------------------------
class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:

    bool ShouldCollide(JPH::ObjectLayer inLayer1,
                       JPH::BroadPhaseLayer inLayer2) const override
    {
        // Статика не сталкивается со статикой, динамика - со всеми
        if (broadPhaseFor(static_cast<Layer>(inLayer1)) == BroadPhaseLayers::STATIC)
            return inLayer2 == BroadPhaseLayers::MOVING;

        return true;
    }
};

//------------------------------------------------------------------------------
/// Точный фильтр: пара слоёв по матрице контактов
//------------------------------------------------------------------------------
class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
{
public:

    bool ShouldCollide(JPH::ObjectLayer inLayer1,
                       JPH::ObjectLayer inLayer2) const override
    {
        return CollisionWorld::canLayersCollide(static_cast<Layer>(inLayer1),
                                                static_cast<Layer>(inLayer2));
    }
};

//------------------------------------------------------------------------------
/// Симметричная матрица контактов слоёв
//------------------------------------------------------------------------------
struct LayerPairMatrix
{
    bool table[layer_count][layer_count] = {};

    constexpr void allow(Layer a, Layer b)
    {
        table[to_index(a)][to_index(b)] = true;
        table[to_index(b)][to_index(a)] = true;
    }

    constexpr LayerPairMatrix()
    {
        // Рельеф
        allow(Layer::Terrain, Layer::Train);
        allow(Layer::Terrain, Layer::Bogie);
        allow(Layer::Terrain, Layer::Wheel);
        allow(Layer::Terrain, Layer::RoadVehicle);
        allow(Layer::Terrain, Layer::Player);

        // Рельсы и элементы пути
        allow(Layer::Rail, Layer::Wheel);
        allow(Layer::Rail, Layer::Bogie);
        allow(Layer::Track, Layer::Wheel);
        allow(Layer::Track, Layer::Bogie);
        allow(Layer::Track, Layer::Train);

        // Инфраструктура
        allow(Layer::Infrastructure, Layer::Train);
        allow(Layer::Infrastructure, Layer::Bogie);
        allow(Layer::Infrastructure, Layer::Wheel);
        allow(Layer::Infrastructure, Layer::RoadVehicle);
        allow(Layer::Infrastructure, Layer::Player);

        // Контактная сеть (пантографы - часть слоя Train)
        allow(Layer::ContactNetwork, Layer::Train);

        // Декорации
        allow(Layer::Decoration, Layer::RoadVehicle);
        allow(Layer::Decoration, Layer::Player);

        // Растительность
        allow(Layer::Vegetation, Layer::Train);
        allow(Layer::Vegetation, Layer::RoadVehicle);
        allow(Layer::Vegetation, Layer::Player);

        // Подвижной состав (самоконтакт внутри единицы ПС
        // отсекается групповым фильтром тел, а не слоями)
        allow(Layer::Train, Layer::Train);
        allow(Layer::Train, Layer::Bogie);
        allow(Layer::Train, Layer::RoadVehicle);
        allow(Layer::Bogie, Layer::Wheel);
        allow(Layer::Wheel, Layer::Wheel);
        allow(Layer::RoadVehicle, Layer::RoadVehicle);
    }
};

constexpr LayerPairMatrix kLayerPairs;

//------------------------------------------------------------------------------
/// Слушатель контактов: пишет события в очередь.
/// Вызывается из рабочих потоков Jolt при заблокированных телах,
/// поэтому только читает тела и ничего в мире не меняет
//------------------------------------------------------------------------------
class ContactListenerImpl final : public JPH::ContactListener
{
public:

    explicit ContactListenerImpl(CollisionEventQueue& queue,
                                 float kinematic_impact_mass)
        : queue_(queue)
        , kinematic_impact_mass_(kinematic_impact_mass)
    {
    }

    JPH::ValidateResult OnContactValidate(
        const JPH::Body& body1,
        const JPH::Body& body2,
        JPH::RVec3Arg,
        const JPH::CollideShapeResult&) override
    {
        // Тела одной группы (узлы одной единицы ПС) не сталкиваются
        const BodyInfo* info1 = bodyInfoOf(body1);
        const BodyInfo* info2 = bodyInfoOf(body2);

        if (info1 != nullptr && info2 != nullptr &&
            info1->group != no_group && info1->group == info2->group)
        {
            return JPH::ValidateResult::RejectAllContactsForThisBodyPair;
        }

        return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
    }

    void OnContactAdded(const JPH::Body& body1,
                        const JPH::Body& body2,
                        const JPH::ContactManifold& manifold,
                        JPH::ContactSettings& settings) override
    {
        pushContact(EventType::ContactAdded, body1, body2, manifold, settings);
    }

    void OnContactPersisted(const JPH::Body& body1,
                            const JPH::Body& body2,
                            const JPH::ContactManifold& manifold,
                            JPH::ContactSettings& settings) override
    {
        pushContact(EventType::ContactPersisted, body1, body2, manifold, settings);
    }

    void OnContactRemoved(const JPH::SubShapeIDPair& sub_shape_pair) override
    {
        // Тела в этом колбэке недоступны, известны только их идентификаторы
        CollisionEvent event;
        event.type = EventType::ContactRemoved;
        event.body_a = packedId(sub_shape_pair.GetBody1ID());
        event.body_b = packedId(sub_shape_pair.GetBody2ID());
        queue_.push(event);
    }

private:

    void pushContact(EventType type,
                     const JPH::Body& body1,
                     const JPH::Body& body2,
                     const JPH::ContactManifold& manifold,
                     const JPH::ContactSettings& settings)
    {
        CollisionEvent event;
        event.type = type;
        event.body_a = packedId(body1.GetID());
        event.body_b = packedId(body2.GetID());

        const BodyInfo* info1 = bodyInfoOf(body1);
        const BodyInfo* info2 = bodyInfoOf(body2);
        event.user_data_a = (info1 != nullptr) ? info1->user_data : nullptr;
        event.user_data_b = (info2 != nullptr) ? info2->user_data : nullptr;

        event.layer_a = static_cast<Layer>(body1.GetObjectLayer());
        event.layer_b = static_cast<Layer>(body2.GetObjectLayer());

        const JPH::Vec3 normal = manifold.GetWorldSpaceNormal();
        event.normal = Vec3f(normal.GetX(), normal.GetY(), normal.GetZ());
        event.penetration = manifold.mPenetrationDepth;

        JPH::RVec3 contact_point = manifold.mBaseOffset;
        if (!manifold.mRelativeContactPointsOn1.empty())
            contact_point = manifold.GetWorldSpaceContactPointOn1(0);

        event.point = Vec3f(static_cast<float>(contact_point.GetX()),
                            static_cast<float>(contact_point.GetY()),
                            static_cast<float>(contact_point.GetZ()));

        //--- Импульс и относительная скорость (ТЗ "Коллизии" п.8, P1-5) ---
        // Слушатель вызывается ДО солвера (см. комментарий в
        // Jolt/Physics/Collision/ContactListener.h v5.6.0), фактический
        // импульс контакта ещё не вычислен. Оценка - штатная
        // JPH::EstimateCollisionResponse; для пар без конечных масс
        // (кинематика ПС против статики/кинематики - солвер дал бы 0) -
        // по приведённой массе и скорости сближения: скорость
        // кинематических тел Jolt знает из перемещений, статик = 0
        const JPH::Vec3 v1 = body1.GetPointVelocity(contact_point);
        const JPH::Vec3 v2 = body2.GetPointVelocity(contact_point);

        // Положительная скорость - сближение (нормаль ведёт от 1 к 2)
        const float closing = -(v2 - v1).Dot(normal);
        event.relative_velocity = static_cast<double>(closing);

        JPH::CollisionEstimationResult estimate;
        JPH::EstimateCollisionResponse(body1, body2, manifold, estimate,
                                       settings.mCombinedFriction,
                                       settings.mCombinedRestitution);

        double impulse = 0.0;
        for (JPH::uint i = 0; i < estimate.mContactImpulse.size(); ++i)
            impulse += std::max(0.0f, estimate.mContactImpulse[i]);

        if (impulse <= 0.0 && closing > 0.0f)
        {
            // Приведённая масса пары; кинематические тела имеют
            // нулевую обратную массу - подставляем условную массу ПС
            const float inv1 = body1.IsStatic() ? 0.0f :
                body1.GetMotionProperties()->GetInverseMassUnchecked();
            const float inv2 = body2.IsStatic() ? 0.0f :
                body2.GetMotionProperties()->GetInverseMassUnchecked();

            const float reduced_mass = (inv1 + inv2) > 0.0f ?
                1.0f / (inv1 + inv2) : kinematic_impact_mass_;

            impulse = static_cast<double>(reduced_mass) *
                      static_cast<double>(closing);
        }

        event.impulse = impulse;

        queue_.push(event);
    }

    CollisionEventQueue& queue_;
    float kinematic_impact_mass_;
};

//------------------------------------------------------------------------------
/// Слушатель засыпания/пробуждения тел
//------------------------------------------------------------------------------
class ActivationListenerImpl final : public JPH::BodyActivationListener
{
public:

    explicit ActivationListenerImpl(CollisionEventQueue& queue)
        : queue_(queue)
    {
    }

    void OnBodyActivated(const JPH::BodyID& body_id,
                         JPH::uint64 user_data) override
    {
        push(EventType::BodyActivated, body_id, user_data);
    }

    void OnBodyDeactivated(const JPH::BodyID& body_id,
                           JPH::uint64 user_data) override
    {
        push(EventType::BodyDeactivated, body_id, user_data);
    }

private:

    void push(EventType type, const JPH::BodyID& body_id, JPH::uint64 user_data)
    {
        CollisionEvent event;
        event.type = type;
        event.body_a = packedId(body_id);

        const BodyInfo* info = reinterpret_cast<const BodyInfo*>(
            static_cast<std::uintptr_t>(user_data));
        event.user_data_a = (info != nullptr) ? info->user_data : nullptr;

        queue_.push(event);
    }

    CollisionEventQueue& queue_;
};

//------------------------------------------------------------------------------
// Глобальная инициализация Jolt (одна на все экземпляры CollisionWorld)
//------------------------------------------------------------------------------
std::atomic<unsigned int> g_jolt_refcount{0};

void jolt_global_init()
{
    if (g_jolt_refcount.fetch_add(1) == 0)
    {
        JPH::RegisterDefaultAllocator();
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();
    }
}

void jolt_global_shutdown()
{
    if (g_jolt_refcount.fetch_sub(1) == 1)
    {
        JPH::UnregisterTypes();
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;
    }
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct CollisionWorld::Impl
{
    WorldSettings settings;

    BroadPhaseLayerInterfaceImpl      broad_phase_interface;
    ObjectVsBroadPhaseLayerFilterImpl object_vs_broad_filter;
    ObjectLayerPairFilterImpl         object_pair_filter;

    CollisionEventQueue event_queue;

    std::unique_ptr<JPH::TempAllocator>  temp_allocator;
    std::unique_ptr<JPH::JobSystem>      job_system;
    std::unique_ptr<JPH::PhysicsSystem>  physics_system;

    std::unique_ptr<ContactListenerImpl>    contact_listener;
    std::unique_ptr<ActivationListenerImpl> activation_listener;

    // Владение BodyInfo: ключ - упакованный BodyID.
    // Указатель на BodyInfo продублирован в Body::UserData
    std::unordered_map<std::uint32_t, std::unique_ptr<BodyInfo>> body_infos;

    bool initialized = false;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CollisionWorld::CollisionWorld()
    : impl_(std::make_unique<Impl>())
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CollisionWorld::~CollisionWorld()
{
    shutdown();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CollisionWorld::init(const WorldSettings& settings)
{
    if (impl_->initialized)
        return true;

    impl_->settings = settings;

    jolt_global_init();

    // У TempAllocatorMalloc в Jolt v5.6.0 конструктора с параметрами нет
    // (Jolt/Core/TempAllocator.h): он не держит фиксированный буфер,
    // а выделяет память malloc'ом под каждый запрос, поэтому настройка
    // temp_allocator_size здесь не участвует. TempAllocatorImpl с буфером
    // рантайм-размера сознательно не берём: при исчерпании буфера он вызывает
    // std::abort(), что для симулятора недопустимо
    impl_->temp_allocator = std::make_unique<JPH::TempAllocatorMalloc>();

    int threads = settings.worker_threads;
    if (threads < 0)
        threads = static_cast<int>(std::thread::hardware_concurrency()) - 1;
    if (threads < 1)
        threads = 1;

    impl_->job_system = std::make_unique<JPH::JobSystemThreadPool>(
        JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, threads);

    impl_->physics_system = std::make_unique<JPH::PhysicsSystem>();
    impl_->physics_system->Init(settings.max_bodies,
                                settings.num_body_mutexes,
                                settings.max_body_pairs,
                                settings.max_contact_constraints,
                                impl_->broad_phase_interface,
                                impl_->object_vs_broad_filter,
                                impl_->object_pair_filter);

    impl_->contact_listener = std::make_unique<ContactListenerImpl>(
        impl_->event_queue, settings.kinematic_impact_mass);
    impl_->physics_system->SetContactListener(impl_->contact_listener.get());

    impl_->activation_listener = std::make_unique<ActivationListenerImpl>(
        impl_->event_queue);
    impl_->physics_system->SetBodyActivationListener(
        impl_->activation_listener.get());

    impl_->initialized = true;
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CollisionWorld::shutdown()
{
    if (!impl_->initialized)
        return;

    impl_->physics_system->SetContactListener(nullptr);
    impl_->physics_system->SetBodyActivationListener(nullptr);
    impl_->contact_listener.reset();
    impl_->activation_listener.reset();

    impl_->body_infos.clear();

    impl_->physics_system.reset();
    impl_->job_system.reset();
    impl_->temp_allocator.reset();

    impl_->initialized = false;

    jolt_global_shutdown();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CollisionWorld::isInitialized() const
{
    return impl_->initialized;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CollisionWorld::step(float dt, int collision_steps)
{
    if (!impl_->initialized)
        return;

    impl_->physics_system->Update(dt,
                                  collision_steps,
                                  impl_->temp_allocator.get(),
                                  impl_->job_system.get());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CollisionWorld::setGravity(float x, float y, float z)
{
    if (!impl_->initialized)
        return;

    impl_->physics_system->SetGravity(JPH::Vec3(x, y, z));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CollisionObject CollisionWorld::createObject(const ObjectDesc& desc)
{
    if (!impl_->initialized || !desc.shape.isValid())
        return CollisionObject();

    auto info = std::make_unique<BodyInfo>();
    info->user_data = desc.user_data;
    info->group = desc.group;

    JPH::BodyCreationSettings settings(
        desc.shape.impl_->shape.GetPtr(),
        JPH::RVec3(desc.position.x, desc.position.y, desc.position.z),
        JPH::Quat(desc.rotation.x, desc.rotation.y,
                  desc.rotation.z, desc.rotation.w),
        toJoltMotionType(desc.motion),
        static_cast<JPH::ObjectLayer>(to_index(desc.layer)));

    settings.mFriction = desc.profile.friction;
    settings.mRestitution = desc.profile.restitution;
    settings.mLinearDamping = desc.profile.linear_damping;
    settings.mAngularDamping = desc.profile.angular_damping;
    settings.mIsSensor = desc.profile.is_sensor;
    settings.mCollideKinematicVsNonDynamic = desc.collide_kinematic_vs_static;
    // CCD: LinearCast исключает туннелирование быстрых кинематических
    // тел ПС сквозь тонкие препятствия (ТЗ "Коллизии" п.24, P2-12).
    // Broadphase поддерживает из коробки: качество движения - свойство
    // тела, фильтры пар не меняются
    settings.mMotionQuality = toJoltMotionQuality(desc.motion_quality);
    settings.mUserData = static_cast<JPH::uint64>(
        reinterpret_cast<std::uintptr_t>(info.get()));

    JPH::BodyInterface& body_interface = impl_->physics_system->GetBodyInterface();

    JPH::Body* body = body_interface.CreateBody(settings);
    if (body == nullptr)
        return CollisionObject();

    const JPH::BodyID body_id = body->GetID();
    body_interface.AddBody(body_id,
        desc.motion == MotionType::Dynamic ? JPH::EActivation::Activate
                                           : JPH::EActivation::DontActivate);

    const std::uint32_t packed = packedId(body_id);
    impl_->body_infos.emplace(packed, std::move(info));

    return CollisionObject(this, packed, desc.layer);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CollisionWorld::removeObject(CollisionObject& object)
{
    if (!impl_->initialized || !object.isValid() || object.world_ != this)
        return;

    const JPH::BodyID body_id(object.body_id_);

    JPH::BodyInterface& body_interface = impl_->physics_system->GetBodyInterface();
    body_interface.RemoveBody(body_id);
    body_interface.DestroyBody(body_id);

    impl_->body_infos.erase(object.body_id_);

    object.world_ = nullptr;
    object.body_id_ = JPH::BodyID::cInvalidBodyID;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CollisionWorld::pollEvent(CollisionEvent& event)
{
    return impl_->event_queue.poll(event);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CollisionWorld::clearEvents()
{
    impl_->event_queue.clear();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CollisionWorld::optimizeBroadPhase()
{
    if (!impl_->initialized)
        return;

    impl_->physics_system->OptimizeBroadPhase();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
JPH::PhysicsSystem* CollisionWorld::physicsSystem()
{
    return impl_->physics_system.get();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CollisionWorld::canLayersCollide(Layer a, Layer b)
{
    return kLayerPairs.table[to_index(a)][to_index(b)];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CollisionWorld::raycast(const Vec3f& position,
                             const Vec3f& direction,
                             float max_dist,
                             Vec3f& point,
                             Vec3f& normal)
{
    if (!impl_->initialized)
        return false;

    const JPH::RVec3 origin(position.x, position.y, position.z);
    const JPH::Vec3 dir(direction.x, direction.y, direction.z);

    const float dir_length = dir.Length();
    if (dir_length < 1.0e-8f || max_dist <= 0.0f)
        return false;

    // Максимальная дальность в Jolt задаётся длиной направления луча:
    // RRayCast::mDirection - "Direction and length of the ray"
    // (Jolt/Physics/Collision/RayCast.h). Третий параметр CastRay -
    // BroadPhaseLayerFilter, а не дистанция (сигнатура в
    // Jolt/Physics/Collision/NarrowPhaseQuery.h), поэтому нормализуем
    // направление, масштабируем до max_dist и фильтры оставляем по умолчанию
    const JPH::RRayCast ray(origin, dir * (max_dist / dir_length));

    JPH::RayCastResult result;

    if (!impl_->physics_system->GetNarrowPhaseQuery().CastRay(ray, result))
    {
        return false;
    }

    const JPH::RVec3 hit = ray.GetPointOnRay(result.mFraction);

    // У RayCastResult нет поля mBody - только mBodyID, mFraction и
    // mSubShapeID2 (Jolt/Physics/Collision/CastResult.h). Для нормали
    // блокируем тело на чтение через lock-интерфейс мира
    // (паттерн из Jolt/Physics/Body/BodyLock.h)
    JPH::BodyLockRead body_lock(impl_->physics_system->GetBodyLockInterface(),
                                result.mBodyID);
    if (!body_lock.Succeeded())
        return false;

    // Нормаль в точке попадания (Jolt/Physics/Body/Body.h)
    const JPH::Vec3 n = body_lock.GetBody().GetWorldSpaceSurfaceNormal(
                result.mSubShapeID2, hit);

    point = Vec3f(static_cast<float>(hit.GetX()),
                  static_cast<float>(hit.GetY()),
                  static_cast<float>(hit.GetZ()));

    normal = Vec3f(n.GetX(), n.GetY(), n.GetZ());

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Vec3f CollisionWorld::objectPosition(std::uint32_t body_id) const
{
    if (!impl_->initialized)
        return Vec3f();

    const JPH::RVec3 position = impl_->physics_system->GetBodyInterface()
        .GetPosition(JPH::BodyID(body_id));

    return Vec3f(static_cast<float>(position.GetX()),
                 static_cast<float>(position.GetY()),
                 static_cast<float>(position.GetZ()));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Quatf CollisionWorld::objectRotation(std::uint32_t body_id) const
{
    if (!impl_->initialized)
        return Quatf();

    const JPH::Quat rotation = impl_->physics_system->GetBodyInterface()
        .GetRotation(JPH::BodyID(body_id));

    return Quatf(rotation.GetX(), rotation.GetY(),
                 rotation.GetZ(), rotation.GetW());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CollisionWorld::setObjectPositionRotation(std::uint32_t body_id,
                                               const Vec3f& position,
                                               const Quatf& rotation)
{
    if (!impl_->initialized)
        return;

    impl_->physics_system->GetBodyInterface().SetPositionAndRotation(
        JPH::BodyID(body_id),
        JPH::RVec3(position.x, position.y, position.z),
        JPH::Quat(rotation.x, rotation.y, rotation.z, rotation.w),
        JPH::EActivation::Activate);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CollisionWorld::setObjectActive(std::uint32_t body_id, bool active)
{
    if (!impl_->initialized)
        return;

    JPH::BodyInterface& body_interface = impl_->physics_system->GetBodyInterface();

    if (active)
        body_interface.ActivateBody(JPH::BodyID(body_id));
    else
        body_interface.DeactivateBody(JPH::BodyID(body_id));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void* CollisionWorld::objectUserData(std::uint32_t body_id) const
{
    auto it = impl_->body_infos.find(body_id);
    if (it == impl_->body_infos.end())
        return nullptr;

    return it->second->user_data;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CollisionWorld::setObjectUserData(std::uint32_t body_id, void* user_data)
{
    auto it = impl_->body_infos.find(body_id);
    if (it == impl_->body_infos.end())
        return;

    it->second->user_data = user_data;
}
