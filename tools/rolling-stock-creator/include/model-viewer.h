#ifndef MODEL_VIEWER_H
#define MODEL_VIEWER_H

#include "phys-points.h"
#include "scene-model.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Group.h>
#include <vsg/maths/vec3.h>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <QString>
#include <thread>
#include <vector>

namespace vsg
{

class Animation;
class Camera;
class Group;
class LookAt;
class Node;
class Viewer;

} // namespace vsg

class OrbitCamera;

//------------------------------------------------------------------------------
//
//  3D-вьюпорт модели ПС: отдельное окно vsg::Viewer поверх Qt-интерфейса
//  (по образцу интеграции vsg+Qt в viewer проекта), рендер — в отдельном
//  потоке. Орбит-камера: вращение ЛКМ, зум колесом, панорама ПКМ.
//
//  Все изменения сцены выполняются задачами в потоке рендера
//  (между кадрами), поэтому изменяющие методы потокобезопасны.
//
//------------------------------------------------------------------------------
class ModelViewer
{
public:
    ModelViewer() = default;
    ~ModelViewer();

    ModelViewer(const ModelViewer&) = delete;
    ModelViewer& operator=(const ModelViewer&) = delete;

    /// Вьюпорт запущен
    bool isRunning() const;

    /// Запустить окно с указанной моделью; false + error() при неудаче
    bool show(vsg::ref_ptr<vsg::Node> scene,
              const vsg::dvec3& bounds_min, const vsg::dvec3& bounds_max);

    /// Остановить рендер и закрыть окно (блокирует до завершения потока)
    void stop();

    /// Текст последней ошибки
    const QString& error() const;

    /// Показать/скрыть узел модели (модификация графа — в потоке рендера)
    void setNodeVisible(vsg::ref_ptr<vsg::Group> parent, size_t index,
                        vsg::ref_ptr<vsg::Node> node, bool visible);

    /// Перестроить полупрозрачные примитивы коллизий поверх модели (ТЗ п.19)
    void setCollisionPreview(const SceneModel::CollisionParams& params);

    /// Перестроить маркеры-сферы физических точек (ТЗ п.14)
    void setPointMarkers(const std::vector<PhysPoint>& points,
                         double marker_radius);

    /// Маркер центра масс: жёлтая сфера в заданной точке модели
    /// (visible=false — скрыть маркер)
    void setComMarker(bool visible, const vsg::dvec3& position,
                      double radius);

    /// Дебаг-вид (промт п.38): трёхцветные стрелки осей XYZ
    /// в центре модели (X — красная, Y — зелёная, Z — синяя);
    /// строится по образцу сетки/маркеров (vsg::Builder)
    void setAxesMarker(bool visible);

    /// Дебаг-вид (промт п.38): полупрозрачный бокс габарита 1Т
    /// (3.7 м по ширине X, 5.3 м по высоте Z, низ на уровне УГР),
    /// длина — по bbox модели (blending, по образцу превью коллизий)
    void setGabaritMarker(bool visible, double length);

    /// Запустить проигрывание анимации glTF в вьюпорте
    /// (speed — множитель скорости, loop — зацикливание;
    /// обновлением управляет vsg::AnimationManager вьювера)
    void playAnimation(vsg::ref_ptr<vsg::Animation> animation,
                       double speed, bool loop);

    /// Остановить проигрывание конкретной анимации
    void stopAnimation(vsg::ref_ptr<vsg::Animation> animation);

    /// Остановить все проигрываемые анимации
    void stopAllAnimations();

    /// Показать/скрыть сетку и оси
    void setGridVisible(bool visible);

    /// Навести камеру на точку (фокус на выбранном объекте)
    void focusOn(const vsg::dvec3& center, double radius);

    /// Переключить проекцию: true — ортографическая, false — перспективная
    void setOrthographic(bool ortho);

private:
    /// Объекты, живущие и используемые только в потоке рендера
    struct ViewportObjects
    {
        vsg::ref_ptr<vsg::Viewer> viewer;         ///< Вьювер
        vsg::ref_ptr<vsg::Group> root;            ///< Корень сцены
        vsg::ref_ptr<vsg::Group> model_holder;    ///< Слот загруженной модели
        vsg::ref_ptr<vsg::Group> grid_slot;       ///< Слот сетки (вкл/выкл)
        vsg::ref_ptr<vsg::Node> grid_content;     ///< Содержимое сетки и осей
        vsg::ref_ptr<vsg::Group> collision_slot;  ///< Слот превью коллизий
        vsg::ref_ptr<vsg::Group> markers_slot;    ///< Слот маркеров точек
        vsg::ref_ptr<vsg::Group> com_slot;        ///< Слот маркера центра масс
        vsg::ref_ptr<vsg::Group> axes_slot;       ///< Слот стрелок осей ПС (дебаг-вид)
        vsg::ref_ptr<vsg::Group> gabarit_slot;    ///< Слот бокса габарита (дебаг-вид)
        vsg::ref_ptr<vsg::Camera> camera;         ///< Камера вьюпорта
        vsg::ref_ptr<vsg::LookAt> look_at;        ///< Матрица взгляда камеры
        OrbitCamera* orbit = nullptr;             ///< Параметры орбит-камеры
        vsg::dvec3 scene_center = vsg::dvec3(0.0, 0.0, 0.0); ///< Центр bbox сцены, м
        double scene_radius = 10.0;               ///< Радиус модели, м
        double window_aspect = 1.6;               ///< Соотношение сторон окна
        bool grid_visible = true;                 ///< Сетка видима
    };

    /// Поставить задачу в очередь (выполняется в потоке рендера)
    void postTask(std::function<void()>&& task);

    /// Выполнить накопленные задачи (только в потоке рендера)
    void processTasks();

    /// Основной цикл потока рендера
    void renderLoop(vsg::ref_ptr<vsg::Node> scene,
                    const vsg::dvec3& bounds_min,
                    const vsg::dvec3& bounds_max);

    /// Уведомить GUI-поток о результате инициализации
    void notifyInit(bool ok, const QString& message);

    std::thread render_thread_;
    std::atomic<bool> stop_flag_{false};
    std::atomic<bool> running_{false};

    std::mutex tasks_mutex_;
    std::vector<std::function<void()>> tasks_;

    std::mutex init_mutex_;
    std::condition_variable init_cv_;
    int init_state_ = 0;          ///< 0 — идёт, 1 — успех, -1 — ошибка
    QString last_error_;

    ViewportObjects* objects_ = nullptr; ///< Только в потоке рендера
};

#endif // MODEL_VIEWER_H
