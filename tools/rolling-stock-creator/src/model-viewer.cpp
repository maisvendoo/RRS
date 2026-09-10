#include "model-viewer.h"

#include "graphics/common.h"

#include <vsg/animation/Animation.h>
#include <vsg/animation/AnimationManager.h>
#include <vsg/app/Camera.h>
#include <vsg/app/CloseHandler.h>
#include <vsg/app/CommandGraph.h>
#include <vsg/app/ProjectionMatrix.h>
#include <vsg/app/RenderGraph.h>
#include <vsg/app/View.h>
#include <vsg/app/ViewMatrix.h>
#include <vsg/app/Viewer.h>
#include <vsg/app/Window.h>
#include <vsg/app/WindowTraits.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/lighting/AmbientLight.h>
#include <vsg/lighting/DirectionalLight.h>
#include <vsg/maths/transform.h>
#include <vsg/nodes/Group.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/ui/ScrollWheelEvent.h>
#include <vsg/utils/Builder.h>

#include <algorithm>
#include <cmath>
#include <utility>

//------------------------------------------------------------------------------
//
//  3D-вьюпорт: окно vsg::Viewer в отдельном потоке рендера.
//  Камера орбитальная: вращение ЛКМ, зум колесом, панорама ПКМ.
//  Примитивы (сетка, превью коллизий, маркеры точек) строятся
//  через vsg::Builder и пересобираются задачами в потоке рендера.
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Орбит-камера: параметры орбиты + обработчик событий мыши
//------------------------------------------------------------------------------
class OrbitCamera final : public vsg::Inherit<vsg::Visitor, OrbitCamera>
{
public:
    explicit OrbitCamera(const vsg::ref_ptr<vsg::LookAt>& lookAt)
        : _lookAt(lookAt)
    {
    }

    /// Пересчёт LookAt из параметров орбиты
    void update()
    {
        if (!_lookAt)
        {
            return;
        }

        const double cos_el = std::cos(elevation);
        const vsg::dvec3 eye_dir(std::sin(azimuth) * cos_el,
                                 std::cos(azimuth) * cos_el,
                                 std::sin(elevation));

        _lookAt->eye = center + eye_dir * distance;
        _lookAt->center = center;
        _lookAt->up = vsg::dvec3(0.0, 0.0, 1.0);
    }

    void apply(vsg::ButtonPressEvent& event) override
    {
        if (event.handled)
        {
            return;
        }

        if (event.button == 1)
        {
            _rotating = true;
        }
        else if (event.button == 3)
        {
            _panning = true;
        }

        _last = vsg::dvec2(static_cast<double>(event.x),
                           static_cast<double>(event.y));
        _have_last = true;
        event.handled = true;
    }

    void apply(vsg::ButtonReleaseEvent& event) override
    {
        if (event.button == 1)
        {
            _rotating = false;
        }
        else if (event.button == 3)
        {
            _panning = false;
        }
    }

    void apply(vsg::MoveEvent& event) override
    {
        const vsg::dvec2 pos(static_cast<double>(event.x),
                             static_cast<double>(event.y));

        if (!_have_last)
        {
            _last = pos;
            _have_last = true;
            return;
        }

        const vsg::dvec2 delta = pos - _last;
        _last = pos;

        if (_rotating)
        {
            // Вращение: азимут вокруг вертикали + наклон
            constexpr double rotate_speed = 0.005;

            azimuth -= delta.x * rotate_speed;
            elevation += delta.y * rotate_speed;
            elevation = std::clamp(elevation, -1.55, 1.55);
            update();
            event.handled = true;
        }
        else if (_panning)
        {
            // Панорама: сдвиг центра орбиты в плоскости экрана
            const double scale = distance * 0.0016;
            const double cos_el = std::cos(elevation);
            const vsg::dvec3 eye_dir(std::sin(azimuth) * cos_el,
                                     std::cos(azimuth) * cos_el,
                                     std::sin(elevation));
            const vsg::dvec3 forward = -eye_dir;
            const vsg::dvec3 right =
                vsg::normalize(vsg::cross(forward, vsg::dvec3(0.0, 0.0, 1.0)));
            const vsg::dvec3 up = vsg::cross(right, forward);

            center -= right * (delta.x * scale);
            center += up * (delta.y * scale);
            update();
            event.handled = true;
        }
    }

    void apply(vsg::ScrollWheelEvent& event) override
    {
        if (event.handled)
        {
            return;
        }

        if (event.delta.y > 0.0f)
        {
            distance /= 1.1;
        }
        else if (event.delta.y < 0.0f)
        {
            distance *= 1.1;
        }

        distance = std::clamp(distance, 0.05, 5000.0);
        update();
        event.handled = true;
    }

    vsg::dvec3 center = vsg::dvec3(0.0, 0.0, 0.0);
    double distance = 30.0;
    double azimuth = vsg::radians(35.0);
    double elevation = vsg::radians(20.0);

private:
    vsg::ref_ptr<vsg::LookAt> _lookAt;
    bool _rotating = false;
    bool _panning = false;
    bool _have_last = false;
    vsg::dvec2 _last = vsg::dvec2(0.0, 0.0);
};

//------------------------------------------------------------------------------
/// Цвет маркера физической точки по типу
//------------------------------------------------------------------------------
vsg::vec4 markerColor(PhysPointType type)
{
    switch (type)
    {
    case PhysPointType::Coupler:       return vsg::vec4(1.0f, 0.0f, 0.0f, 0.95f);
    case PhysPointType::Wheel:         return vsg::vec4(0.6f, 0.6f, 0.6f, 0.95f);
    case PhysPointType::Bogie:         return vsg::vec4(0.9f, 0.5f, 0.1f, 0.95f);
    case PhysPointType::BrakeHose:     return vsg::vec4(0.2f, 0.4f, 1.0f, 0.95f);
    case PhysPointType::EndValve:      return vsg::vec4(0.0f, 0.9f, 0.9f, 0.95f);
    case PhysPointType::Pantograph:    return vsg::vec4(1.0f, 0.0f, 1.0f, 0.95f);
    case PhysPointType::Fuel:          return vsg::vec4(0.55f, 0.3f, 0.1f, 0.95f);
    case PhysPointType::Charging:      return vsg::vec4(0.6f, 0.2f, 0.8f, 0.95f);
    case PhysPointType::PassengerDoor: return vsg::vec4(0.0f, 0.8f, 0.0f, 0.95f);
    case PhysPointType::DriverDoor:    return vsg::vec4(0.0f, 0.8f, 0.3f, 0.95f);
    case PhysPointType::Camera:        return vsg::vec4(1.0f, 1.0f, 1.0f, 0.95f);
    case PhysPointType::Sound:         return vsg::vec4(0.4f, 0.8f, 1.0f, 0.95f);
    case PhysPointType::Light:         return vsg::vec4(1.0f, 0.9f, 0.2f, 0.95f);
    case PhysPointType::Spawn:         break;
    }

    return vsg::vec4(0.0f, 1.0f, 0.4f, 0.95f);
}

//------------------------------------------------------------------------------
/// Построить сетку (шаг кратен 1/2/4... м) и оси X/Y/Z (ТЗ п.2)
//------------------------------------------------------------------------------
vsg::ref_ptr<vsg::Node> buildGrid(double scene_radius)
{
    auto builder = vsg::Builder::create();
    auto group = vsg::Group::create();

    const double extent = std::ceil(scene_radius * 1.3);

    // Шаг укрупняется, чтобы линий было не более 40 на сторону
    double step = 1.0;

    while (2.0 * extent / step > 40.0)
    {
        step *= 2.0;
    }

    const float thickness = std::max(0.01f,
                                     static_cast<float>(extent / 1000.0f));

    vsg::StateInfo state;
    state.lighting = false;

    vsg::GeometryInfo info;
    info.color = vsg::vec4(0.45f, 0.45f, 0.45f, 1.0f);

    // Линии вдоль Y (вдоль пути)
    info.dx = vsg::vec3(thickness, 0.0f, 0.0f);
    info.dy = vsg::vec3(0.0f, static_cast<float>(2.0 * extent), 0.0f);
    info.dz = vsg::vec3(0.0f, 0.0f, thickness);

    for (double x = -extent; x <= extent + 0.5 * step; x += step)
    {
        info.position = vsg::vec3(static_cast<float>(x), 0.0f, 0.0f);
        group->addChild(builder->createBox(info, state));
    }

    // Линии вдоль X (поперёк пути)
    info.dx = vsg::vec3(static_cast<float>(2.0 * extent), 0.0f, 0.0f);
    info.dy = vsg::vec3(0.0f, thickness, 0.0f);

    for (double y = -extent; y <= extent + 0.5 * step; y += step)
    {
        info.position = vsg::vec3(0.0f, static_cast<float>(y), 0.0f);
        group->addChild(builder->createBox(info, state));
    }

    // Оси: X — вправо (красная), Y — вдоль пути (зелёная), Z — вверх (синяя)
    info.dy = vsg::vec3(0.0f, thickness, 0.0f);
    info.dz = vsg::vec3(0.0f, 0.0f, thickness);
    info.position = vsg::vec3(0.0f, 0.0f, 0.0f);

    info.dx = vsg::vec3(static_cast<float>(2.0 * extent), 0.0f, 0.0f);
    info.color = vsg::vec4(0.9f, 0.2f, 0.2f, 1.0f);
    group->addChild(builder->createBox(info, state));

    info.dx = vsg::vec3(thickness, 0.0f, 0.0f);
    info.dy = vsg::vec3(0.0f, static_cast<float>(2.0 * extent), 0.0f);
    info.color = vsg::vec4(0.2f, 0.9f, 0.2f, 1.0f);
    group->addChild(builder->createBox(info, state));

    info.dy = vsg::vec3(0.0f, thickness, 0.0f);
    info.dz = vsg::vec3(0.0f, 0.0f, static_cast<float>(extent));
    info.color = vsg::vec4(0.2f, 0.4f, 1.0f, 1.0f);
    group->addChild(builder->createBox(info, state));

    return group;
}

//------------------------------------------------------------------------------
/// Построить полупрозрачные примитивы коллизий по параметрам секции
/// [Collision] (раскладка тел повторяет VehicleCollision::createBodies)
//------------------------------------------------------------------------------
vsg::ref_ptr<vsg::Node> buildCollisionGroup(const SceneModel::CollisionParams& p)
{
    auto builder = vsg::Builder::create();
    auto group = vsg::Group::create();

    vsg::StateInfo state;
    state.lighting = true;
    state.two_sided = true;
    state.blending = true;

    const vsg::vec4 body_color(1.0f, 0.25f, 0.25f, 0.35f);
    const vsg::vec4 bogie_color(1.0f, 0.6f, 0.2f, 0.35f);
    const vsg::vec4 wheel_color(1.0f, 0.9f, 0.2f, 0.35f);

    auto add_box = [&](const vsg::dvec3& position, double half_x,
                       double half_y, double half_z, const vsg::vec4& color)
    {
        vsg::GeometryInfo info;
        info.position = vsg::vec3(static_cast<float>(position.x),
                                  static_cast<float>(position.y),
                                  static_cast<float>(position.z));
        info.dx = vsg::vec3(static_cast<float>(2.0 * half_x), 0.0f, 0.0f);
        info.dy = vsg::vec3(0.0f, static_cast<float>(2.0 * half_y), 0.0f);
        info.dz = vsg::vec3(0.0f, 0.0f, static_cast<float>(2.0 * half_z));
        info.color = color;
        group->addChild(builder->createBox(info, state));
    };

    auto add_wheel = [&](double position_y, double position_z,
                         double radius, double half_width)
    {
        vsg::GeometryInfo info;
        info.position = vsg::vec3(0.0f, static_cast<float>(position_y),
                                  static_cast<float>(position_z));
        info.dx = vsg::vec3(static_cast<float>(2.0 * radius), 0.0f, 0.0f);
        info.dy = vsg::vec3(0.0f, static_cast<float>(2.0 * radius), 0.0f);
        info.dz = vsg::vec3(0.0f, 0.0f,
                            static_cast<float>(2.0 * half_width));
        // Цилиндр строится вдоль Z — поворачиваем ось на X (ось колёсной пары)
        info.transform =
            vsg::rotate(vsg::radians(90.0f), vsg::vec3(0.0f, 1.0f, 0.0f));
        info.color = wheel_color;
        group->addChild(builder->createCylinder(info, state));
    };

    // Кузов
    if (p.bodyHalfLength > 0.0 || p.bodyHalfWidth > 0.0 ||
        p.bodyHalfHeight > 0.0)
    {
        add_box(vsg::dvec3(0.0, 0.0, p.bodyOffsetZ),
                p.bodyHalfWidth, p.bodyHalfLength, p.bodyHalfHeight,
                body_color);
    }

    // Тележки (равномерно от -bogieOffset до +bogieOffset вдоль пути)
    for (int i = 0; i < p.numBogies; ++i)
    {
        double bogie_y = 0.0;

        if (p.numBogies > 1)
        {
            bogie_y = -p.bogieOffset +
                      2.0 * p.bogieOffset * static_cast<double>(i) /
                      static_cast<double>(p.numBogies - 1);
        }

        add_box(vsg::dvec3(0.0, bogie_y, p.bogieOffsetZ),
                p.bogieHalfWidth, p.bogieHalfLength, p.bogieHalfHeight,
                bogie_color);
    }

    // Колёсные пары: цилиндры (остаток осей — в первые тележки)
    if (p.numAxis > 0 && p.wheelRadius > 0.0 && p.numBogies > 0)
    {
        int axle_index = 0;

        for (int i = 0; i < p.numBogies && axle_index < p.numAxis; ++i)
        {
            double bogie_y = 0.0;

            if (p.numBogies > 1)
            {
                bogie_y = -p.bogieOffset +
                          2.0 * p.bogieOffset * static_cast<double>(i) /
                          static_cast<double>(p.numBogies - 1);
            }

            const int axles_here = p.numAxis / p.numBogies +
                                   ((i < p.numAxis % p.numBogies) ? 1 : 0);

            for (int j = 0; j < axles_here && axle_index < p.numAxis;
                 ++j, ++axle_index)
            {
                const double axle_y = bogie_y + p.wheelsetSpacing *
                    (static_cast<double>(j) -
                     0.5 * static_cast<double>(axles_here - 1));

                add_wheel(axle_y, p.wheelRadius, p.wheelRadius,
                          p.wheelsetHalfWidth);
            }
        }
    }

    return group;
}

//------------------------------------------------------------------------------
/// Построить маркеры-сферы физических точек
//------------------------------------------------------------------------------
vsg::ref_ptr<vsg::Node> buildMarkersGroup(const std::vector<PhysPoint>& points,
                                          double marker_radius)
{
    auto builder = vsg::Builder::create();
    auto group = vsg::Group::create();

    vsg::StateInfo state;
    state.lighting = true;
    state.two_sided = true;
    state.blending = true;

    vsg::GeometryInfo info;

    info.dx = vsg::vec3(static_cast<float>(2.0 * marker_radius), 0.0f, 0.0f);
    info.dy = vsg::vec3(0.0f, static_cast<float>(2.0 * marker_radius), 0.0f);
    info.dz = vsg::vec3(0.0f, 0.0f,
                        static_cast<float>(2.0 * marker_radius));

    for (const PhysPoint& point : points)
    {
        info.position = vsg::vec3(static_cast<float>(point.x),
                                  static_cast<float>(point.y),
                                  static_cast<float>(point.z));
        info.color = markerColor(point.type);
        group->addChild(builder->createSphere(info, state));
    }

    return group;
}

//------------------------------------------------------------------------------
/// Построить трёхцветные стрелки осей XYZ в центре модели (промт п.38):
/// X — красная (вправо), Y — зелёная (вдоль пути), Z — синяя (вверх).
/// Стрелки — цилиндр + конус через vsg::Builder (по образцу сетки),
/// геометрия строится вдоль локальной Z и поворачивается на целевую ось
//------------------------------------------------------------------------------
vsg::ref_ptr<vsg::Node> buildAxesMarker(const vsg::dvec3& center,
                                        double scene_radius)
{
    auto builder = vsg::Builder::create();
    auto group = vsg::Group::create();

    vsg::StateInfo state;
    state.lighting = true;

    // Длина стрелок масштабируется под модель, но в разумных пределах
    const double length = std::clamp(0.55 * scene_radius, 0.8, 4.0);
    const double shaft_length = 0.65 * length;
    const double tip_length = length - shaft_length;
    const double shaft_radius = std::max(0.015, 0.018 * length);
    const double tip_radius = 3.0 * shaft_radius;

    struct AxisSpec
    {
        vsg::vec4 color;     ///< Цвет стрелки
        vsg::vec3 direction; ///< Направление оси в мире
        vsg::mat4 rotation;  ///< Поворот локальной Z на направление оси
    };

    const AxisSpec axes[3] =
    {
        // X (вправо): поворот вокруг Y на +90 град переводит Z в X
        {vsg::vec4(0.9f, 0.2f, 0.2f, 1.0f),
         vsg::vec3(1.0f, 0.0f, 0.0f),
         vsg::rotate(vsg::radians(90.0f), vsg::vec3(0.0f, 1.0f, 0.0f))},
        // Y (вдоль пути): поворот вокруг X на -90 град переводит Z в Y
        {vsg::vec4(0.2f, 0.9f, 0.2f, 1.0f),
         vsg::vec3(0.0f, 1.0f, 0.0f),
         vsg::rotate(vsg::radians(-90.0f), vsg::vec3(1.0f, 0.0f, 0.0f))},
        // Z (вверх): без поворота
        {vsg::vec4(0.2f, 0.4f, 1.0f, 1.0f),
         vsg::vec3(0.0f, 0.0f, 1.0f),
         vsg::rotate(vsg::radians(0.0f), vsg::vec3(0.0f, 0.0f, 1.0f))}
    };

    const vsg::vec3 origin(static_cast<float>(center.x),
                           static_cast<float>(center.y),
                           static_cast<float>(center.z));

    for (const AxisSpec& axis : axes)
    {
        // Стержень — цилиндр вдоль оси
        vsg::GeometryInfo shaft_info;
        shaft_info.position = origin + axis.direction *
                static_cast<float>(0.5 * shaft_length);
        shaft_info.dx = vsg::vec3(static_cast<float>(2.0 * shaft_radius),
                                  0.0f, 0.0f);
        shaft_info.dy = vsg::vec3(0.0f,
                                  static_cast<float>(2.0 * shaft_radius),
                                  0.0f);
        shaft_info.dz = vsg::vec3(0.0f, 0.0f,
                                  static_cast<float>(shaft_length));
        shaft_info.transform = axis.rotation;
        shaft_info.color = axis.color;
        group->addChild(builder->createCylinder(shaft_info, state));

        // Кончик — конус (строится вдоль локальной Z, как цилиндр)
        vsg::GeometryInfo tip_info;
        tip_info.position = origin + axis.direction *
                static_cast<float>(shaft_length + 0.5 * tip_length);
        tip_info.dx = vsg::vec3(static_cast<float>(2.0 * tip_radius),
                                0.0f, 0.0f);
        tip_info.dy = vsg::vec3(0.0f,
                                static_cast<float>(2.0 * tip_radius),
                                0.0f);
        tip_info.dz = vsg::vec3(0.0f, 0.0f,
                                static_cast<float>(tip_length));
        tip_info.transform = axis.rotation;
        tip_info.color = axis.color;
        group->addChild(builder->createCone(tip_info, state));
    }

    return group;
}

//------------------------------------------------------------------------------
/// Построить полупрозрачный бокс габарита 1Т (промт п.38):
/// 3.7 м по ширине (X) x 5.3 м по высоте (Z), низ на уровне головки
/// рельса (z=0), длина по Y — из bbox модели; blending как у коллизий
//------------------------------------------------------------------------------
vsg::ref_ptr<vsg::Node> buildGabaritMarker(double length, double center_y)
{
    constexpr double gabarit_width = 3.7;  ///< Габарит 1Т: полная ширина, м
    constexpr double gabarit_height = 5.3; ///< Габарит 1Т: высота от УГР, м

    auto builder = vsg::Builder::create();

    vsg::StateInfo state;
    state.lighting = true;
    state.two_sided = true;
    state.blending = true;

    vsg::GeometryInfo info;
    info.position = vsg::vec3(0.0f, static_cast<float>(center_y),
                              static_cast<float>(0.5 * gabarit_height));
    info.dx = vsg::vec3(static_cast<float>(gabarit_width), 0.0f, 0.0f);
    info.dy = vsg::vec3(0.0f, static_cast<float>(length), 0.0f);
    info.dz = vsg::vec3(0.0f, 0.0f,
                        static_cast<float>(gabarit_height));
    info.color = vsg::vec4(0.2f, 0.85f, 0.3f, 0.16f);

    return builder->createBox(info, state);
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ModelViewer::~ModelViewer()
{
    stop();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ModelViewer::isRunning() const
{
    return running_.load();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const QString& ModelViewer::error() const
{
    return last_error_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModelViewer::postTask(std::function<void()>&& task)
{
    const std::lock_guard<std::mutex> lock(tasks_mutex_);
    tasks_.push_back(std::move(task));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModelViewer::processTasks()
{
    std::vector<std::function<void()>> tasks;

    {
        const std::lock_guard<std::mutex> lock(tasks_mutex_);
        tasks.swap(tasks_);
    }

    for (const std::function<void()>& task : tasks)
    {
        if (stop_flag_.load())
        {
            break;
        }

        try
        {
            task();
        }
        catch (const vsg::Exception&)
        {
            // Ошибку задачи (например, нехватку памяти GPU) не роняем: рендер
            // продолжается, уведомление GUI не требуется
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModelViewer::notifyInit(bool ok, const QString& message)
{
    {
        const std::lock_guard<std::mutex> lock(init_mutex_);
        init_state_ = ok ? 1 : -1;

        if (!ok)
        {
            last_error_ = message;
        }
    }

    init_cv_.notify_all();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ModelViewer::show(vsg::ref_ptr<vsg::Node> scene,
                       const vsg::dvec3& bounds_min,
                       const vsg::dvec3& bounds_max)
{
    stop();

    {
        const std::lock_guard<std::mutex> lock(init_mutex_);
        init_state_ = 0;
        last_error_.clear();
    }

    stop_flag_.store(false);
    running_.store(true);

    render_thread_ = std::thread(&ModelViewer::renderLoop, this, scene,
                                 bounds_min, bounds_max);

    std::unique_lock<std::mutex> lock(init_mutex_);
    init_cv_.wait(lock, [this]() { return init_state_ != 0; });
    lock.unlock();

    if (init_state_ < 0)
    {
        if (render_thread_.joinable())
        {
            render_thread_.join();
        }

        running_.store(false);
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModelViewer::stop()
{
    if (running_.load())
    {
        stop_flag_.store(true);

        // Просим вьювер закрыться из потока рендера; если цикл уже
        // завершён, задача просто не выполнится, а join вернётся сразу
        postTask([this]()
        {
            if (objects_ != nullptr && objects_->viewer != nullptr)
            {
                objects_->viewer->close();
            }
        });
    }

    if (render_thread_.joinable())
    {
        render_thread_.join();
    }

    running_.store(false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModelViewer::setNodeVisible(vsg::ref_ptr<vsg::Group> parent, size_t index,
                                 vsg::ref_ptr<vsg::Node> node, bool visible)
{
    postTask([parent, index, node, visible]()
    {
        if (parent == nullptr || node == nullptr)
        {
            return;
        }

        vsg::Group::Children& children = parent->children;

        if (visible)
        {
            // Защита от повторного добавления
            for (const vsg::ref_ptr<vsg::Node>& child : children)
            {
                if (child == node)
                {
                    return;
                }
            }

            const size_t position = std::min(index, children.size());
            children.insert(children.begin() +
                            static_cast<long>(position), node);
        }
        else
        {
            for (size_t i = 0; i < children.size(); ++i)
            {
                if (children[i] == node)
                {
                    children.erase(children.begin() +
                                   static_cast<long>(i));
                    return;
                }
            }
        }
    });
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModelViewer::setCollisionPreview(const SceneModel::CollisionParams& params)
{
    postTask([this, params]()
    {
        if (objects_ == nullptr || objects_->collision_slot == nullptr ||
            objects_->viewer == nullptr)
        {
            return;
        }

        objects_->collision_slot->children.clear();

        if (params.valid)
        {
            objects_->collision_slot->addChild(
                        buildCollisionGroup(params));
            objects_->viewer->compile();
        }
    });
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModelViewer::setPointMarkers(const std::vector<PhysPoint>& points,
                                  double marker_radius)
{
    postTask([this, points, marker_radius]()
    {
        if (objects_ == nullptr || objects_->markers_slot == nullptr ||
            objects_->viewer == nullptr)
        {
            return;
        }

        objects_->markers_slot->children.clear();

        if (!points.empty())
        {
            objects_->markers_slot->addChild(
                        buildMarkersGroup(points, marker_radius));
            objects_->viewer->compile();
        }
    });
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModelViewer::setComMarker(bool visible, const vsg::dvec3& position,
                               double radius)
{
    postTask([this, visible, position, radius]()
    {
        if (objects_ == nullptr || objects_->com_slot == nullptr ||
            objects_->viewer == nullptr)
        {
            return;
        }

        objects_->com_slot->children.clear();

        if (visible)
        {
            auto builder = vsg::Builder::create();

            vsg::StateInfo state;
            state.lighting = true;
            state.two_sided = true;
            state.blending = true;

            // Жёлтая полупрозрачная сфера — центр масс (ТЗ п.14)
            vsg::GeometryInfo info;
            info.position = vsg::vec3(static_cast<float>(position.x),
                                      static_cast<float>(position.y),
                                      static_cast<float>(position.z));
            info.dx = vsg::vec3(static_cast<float>(2.0 * radius), 0.0f, 0.0f);
            info.dy = vsg::vec3(0.0f, static_cast<float>(2.0 * radius), 0.0f);
            info.dz = vsg::vec3(0.0f, 0.0f,
                                static_cast<float>(2.0 * radius));
            info.color = vsg::vec4(1.0f, 0.85f, 0.1f, 0.95f);

            objects_->com_slot->addChild(builder->createSphere(info, state));
            objects_->viewer->compile();
        }
    });
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModelViewer::setAxesMarker(bool visible)
{
    postTask([this, visible]()
    {
        if (objects_ == nullptr || objects_->axes_slot == nullptr ||
            objects_->viewer == nullptr)
        {
            return;
        }

        objects_->axes_slot->children.clear();

        if (visible)
        {
            objects_->axes_slot->addChild(
                        buildAxesMarker(objects_->scene_center,
                                        objects_->scene_radius));
            objects_->viewer->compile();
        }
    });
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModelViewer::setGabaritMarker(bool visible, double length)
{
    postTask([this, visible, length]()
    {
        if (objects_ == nullptr || objects_->gabarit_slot == nullptr ||
            objects_->viewer == nullptr)
        {
            return;
        }

        objects_->gabarit_slot->children.clear();

        if (visible)
        {
            objects_->gabarit_slot->addChild(
                        buildGabaritMarker(std::max(length, 1.0),
                                           objects_->scene_center.y));
            objects_->viewer->compile();
        }
    });
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModelViewer::playAnimation(vsg::ref_ptr<vsg::Animation> animation,
                                double speed, bool loop)
{
    postTask([this, animation, speed, loop]()
    {
        if (objects_ == nullptr || objects_->viewer == nullptr ||
            animation == nullptr)
        {
            return;
        }

        // Параметры проигрывания: скорость и режим повтора
        animation->speed = speed;
        animation->mode = loop ? vsg::Animation::REPEAT
                               : vsg::Animation::ONCE;

        // AnimationManager создан конструктором vsg::Viewer и обновляется
        // в Viewer::update() каждого кадра
        if (objects_->viewer->animationManager != nullptr)
        {
            objects_->viewer->animationManager->play(animation);
        }
    });
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModelViewer::stopAnimation(vsg::ref_ptr<vsg::Animation> animation)
{
    postTask([this, animation]()
    {
        if (objects_ == nullptr || objects_->viewer == nullptr ||
            animation == nullptr)
        {
            return;
        }

        if (objects_->viewer->animationManager != nullptr)
        {
            objects_->viewer->animationManager->stop(animation);
        }
    });
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModelViewer::stopAllAnimations()
{
    postTask([this]()
    {
        if (objects_ == nullptr || objects_->viewer == nullptr)
        {
            return;
        }

        if (objects_->viewer->animationManager != nullptr)
        {
            objects_->viewer->animationManager->stop();
        }
    });
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModelViewer::setGridVisible(bool visible)
{
    postTask([this, visible]()
    {
        if (objects_ == nullptr || objects_->grid_slot == nullptr)
        {
            return;
        }

        if (visible == objects_->grid_visible)
        {
            return;
        }

        objects_->grid_visible = visible;
        objects_->grid_slot->children.clear();

        if (visible && objects_->grid_content != nullptr)
        {
            objects_->grid_slot->addChild(objects_->grid_content);
        }
    });
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModelViewer::focusOn(const vsg::dvec3& center, double radius)
{
    postTask([this, center, radius]()
    {
        if (objects_ == nullptr || objects_->orbit == nullptr)
        {
            return;
        }

        objects_->orbit->center = center;
        objects_->orbit->distance = std::max(3.0 * radius, 0.5);
        objects_->orbit->update();
    });
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModelViewer::setOrthographic(bool ortho)
{
    postTask([this, ortho]()
    {
        if (objects_ == nullptr || objects_->camera == nullptr ||
            objects_->orbit == nullptr)
        {
            return;
        }

        const double aspect = (objects_->window_aspect > 0.1)
                                  ? objects_->window_aspect : 1.6;

        if (ortho)
        {
            // Полувысота кадра — как у перспективной камеры с fovy 45 град
            const double half_height = objects_->orbit->distance *
                                       std::tan(vsg::radians(22.5));
            const double half_width = half_height * aspect;

            objects_->camera->projectionMatrix = vsg::Orthographic::create(
                        -half_width, half_width, -half_height, half_height,
                        0.01, 5000.0);
        }
        else
        {
            objects_->camera->projectionMatrix = vsg::Perspective::create(
                        vsg::radians(45.0), aspect, 0.01, 5000.0);
        }
    });
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModelViewer::renderLoop(vsg::ref_ptr<vsg::Node> scene,
                             const vsg::dvec3& bounds_min,
                             const vsg::dvec3& bounds_max)
{
    vsg::ref_ptr<vsg::Options> options = create_default_vsg_options();
    options->setValue("disable_gltf", false);

    // Настройки окна вьюпорта
    vsg::ref_ptr<vsg::WindowTraits> traits = vsg::WindowTraits::create();
    traits->windowTitle =
            "RRS Rolling Stock Creator — 3D-вьюпорт "
            "(ЛКМ — вращение, колесо — зум, ПКМ — панорама, Esc — закрыть)";
    traits->width = 1280;
    traits->height = 800;

    vsg::ref_ptr<vsg::Window> window;

    try
    {
        window = vsg::Window::create(traits);

        if (window == nullptr)
        {
            notifyInit(false, QStringLiteral("Не удалось создать окно Vulkan"));
            running_.store(false);
            return;
        }
    }
    catch (const vsg::Exception& exception)
    {
        notifyInit(false, QStringLiteral("Ошибка Vulkan: %1").arg(
                       QString::fromStdString(exception.message)));
        running_.store(false);
        return;
    }

    // Все объекты вьюпорта живут в этом потоке
    ViewportObjects objects;
    objects_ = &objects;

    const vsg::dvec3 scene_center = 0.5 * (bounds_min + bounds_max);
    const vsg::dvec3 scene_size = bounds_max - bounds_min;
    const double scene_radius = std::max(0.5 * vsg::length(scene_size), 1.0);

    objects.scene_radius = scene_radius;
    objects.scene_center = scene_center;
    objects.window_aspect = static_cast<double>(window->extent2D().width) /
                            static_cast<double>(window->extent2D().height);

    // Сцена: модель + слоты сетки/коллизий/маркеров
    objects.root = vsg::Group::create();

    objects.model_holder = vsg::Group::create();
    objects.root->addChild(objects.model_holder);

    if (scene != nullptr)
    {
        objects.model_holder->addChild(scene);
    }

    objects.grid_slot = vsg::Group::create();
    objects.root->addChild(objects.grid_slot);

    objects.collision_slot = vsg::Group::create();
    objects.root->addChild(objects.collision_slot);

    objects.markers_slot = vsg::Group::create();
    objects.root->addChild(objects.markers_slot);

    objects.com_slot = vsg::Group::create();
    objects.root->addChild(objects.com_slot);

    // Слоты дебаг-вида (промт п.38): стрелки осей и бокс габарита
    objects.axes_slot = vsg::Group::create();
    objects.root->addChild(objects.axes_slot);

    objects.gabarit_slot = vsg::Group::create();
    objects.root->addChild(objects.gabarit_slot);

    objects.grid_content = buildGrid(scene_radius);
    objects.grid_slot->addChild(objects.grid_content);

    // Освещение (без него PBR-модель отображалась бы чёрной)
    auto ambient = vsg::AmbientLight::create();
    ambient->color = vsg::vec3(1.0f, 1.0f, 1.0f);
    ambient->intensity = 0.55f;
    objects.root->addChild(ambient);

    auto sun = vsg::DirectionalLight::create();
    sun->color = vsg::vec3(1.0f, 1.0f, 1.0f);
    sun->intensity = 0.9f;
    sun->direction = vsg::normalize(vsg::vec3(-0.4f, -0.7f, -0.6f));
    objects.root->addChild(sun);

    // Камера и орбит-камера
    objects.look_at = vsg::LookAt::create();

    const vsg::ref_ptr<vsg::Perspective> perspective =
            vsg::Perspective::create(vsg::radians(45.0),
                                     objects.window_aspect,
                                     std::max(0.01, scene_radius * 0.005),
                                     std::max(scene_radius * 1000.0, 100.0));

    objects.camera = vsg::Camera::create(perspective, objects.look_at,
                                         vsg::ViewportState::create(
                                             window->extent2D()));

    objects.orbit = new OrbitCamera(objects.look_at);
    objects.orbit->center = scene_center;
    objects.orbit->distance = scene_radius * 3.0;
    objects.orbit->update();

    // Вьювер
    vsg::ref_ptr<vsg::View> view = vsg::View::create();
    view->camera = objects.camera;
    view->addChild(objects.root);

    vsg::ref_ptr<vsg::RenderGraph> render_graph =
            vsg::RenderGraph::create(window, view);

    vsg::ref_ptr<vsg::CommandGraph> command_graph =
            vsg::CommandGraph::create(window, render_graph);

    objects.viewer = vsg::Viewer::create();
    objects.viewer->addWindow(window);
    objects.viewer->addEventHandler(vsg::CloseHandler::create(objects.viewer));
    objects.viewer->addEventHandler(
                vsg::ref_ptr<OrbitCamera>(objects.orbit));
    objects.viewer->assignRecordAndSubmitTaskAndPresentation({command_graph});

    try
    {
        objects.viewer->compile();
        notifyInit(true, QString());
    }
    catch (const vsg::Exception& exception)
    {
        notifyInit(false, QStringLiteral("Ошибка компиляции сцены: %1").arg(
                       QString::fromStdString(exception.message)));
        objects_ = nullptr;
        running_.store(false);
        return;
    }

    // Цикл рендера: задачи выполняются между кадрами
    while (!stop_flag_.load() && objects.viewer->advanceToNextFrame())
    {
        try
        {
            objects.viewer->handleEvents();
            processTasks();
            objects.viewer->update();
            objects.viewer->recordAndSubmit();
            objects.viewer->present();
        }
        catch (const vsg::Exception&)
        {
            break;
        }
    }

    objects.viewer->close();

    // Освобождение ссылок в потоке, где они созданы
    objects.orbit = nullptr;
    objects_ = nullptr;
    running_.store(false);
}
