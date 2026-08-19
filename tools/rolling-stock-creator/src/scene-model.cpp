#include "scene-model.h"

#include "graphics/common.h"

#include <vsg/io/FileSystem.h>
#include <vsg/io/Options.h>
#include <vsg/io/read.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/Node.h>
#include <vsg/utils/ComputeBounds.h>

#include <QDir>
#include <QFileInfo>

#include <algorithm>
#include <cmath>
#include <string>

//------------------------------------------------------------------------------
//
//  Модель сцены ПС: загрузка glTF через vsgXchange, построение дерева
//  объектов, назначение ролей и генерация параметров коллизий.
//
//------------------------------------------------------------------------------
namespace
{

/// Имя объекта: значение "name", записанное загрузчиком, либо имя класса vsg
QString nodeObjectName(const vsg::Node* node)
{
    std::string value;

    if (node != nullptr && node->getValue("name", value) && !value.empty())
    {
        return QString::fromStdString(value);
    }

    if (node != nullptr)
    {
        return QString::fromLatin1(node->className());
    }

    return QStringLiteral("(null)");
}

/// AABB поддерева узла (в локальных осях узла); false, если геометрии нет
bool nodeBounds(const vsg::Node* node, vsg::dvec3& min, vsg::dvec3& max)
{
    if (node == nullptr)
    {
        return false;
    }

    vsg::ComputeBounds compute_bounds;
    node->accept(compute_bounds);

    if (!compute_bounds.bounds.valid())
    {
        return false;
    }

    min = compute_bounds.bounds.min;
    max = compute_bounds.bounds.max;

    return true;
}

/// Роль относится к кузову
bool isBodyRole(MeshRole role)
{
    return role == MeshRole::Body;
}

/// Роль относится к тележке
bool isBogieRole(MeshRole role)
{
    return role == MeshRole::Bogie1 || role == MeshRole::Bogie2 ||
           role == MeshRole::Trolley;
}

/// Роль относится к колёсной паре
bool isWheelRole(MeshRole role)
{
    return role == MeshRole::Wheel;
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool SceneModel::loadModel(const QString& path, QString* error)
{
    clear();

    if (path.isEmpty())
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("Путь к модели пуст");
        }

        return false;
    }

    if (!vsg::fileExists(path.toStdString()))
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("Файл не найден: %1").arg(path);
        }

        return false;
    }

    // Опции как в viewer: все загрузчики vsgXchange (glTF/assimp и пр.)
    vsg::ref_ptr<vsg::Options> options = create_default_vsg_options();
    options->setValue("disable_gltf", false);
    options->paths.insert(options->paths.begin(),
                          QFileInfo(path).absoluteDir()
                          .absolutePath().toStdString());

    vsg::ref_ptr<vsg::Node> node;

    try
    {
        node = vsg::read(path.toStdString(), options).cast<vsg::Node>();
    }
    catch (const vsg::Exception& exception)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("Ошибка vsg при загрузке: %1")
                         .arg(QString::fromStdString(exception.message));
        }

        return false;
    }

    if (node == nullptr)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("Не удалось загрузить модель: %1").arg(path);
        }

        return false;
    }

    root_ = node;
    model_path_ = path;
    loaded_ = true;

    // Сводный AABB всей модели
    nodeBounds(root_.get(), overall_min_, overall_max_);

    // Корень загруженной модели может сам быть группой — обходим детей
    if (vsg::Group* group = root_->cast<vsg::Group>(); group != nullptr)
    {
        size_t index = 0;

        for (const vsg::ref_ptr<vsg::Node>& child : group->children)
        {
            traverse(group, child.get(), QString::number(index), index);
            ++index;
        }
    }
    else
    {
        // Единственный узел без детей-групп
        SceneNodeInfo info;
        info.path = QStringLiteral("0");
        info.name = nodeObjectName(root_.get());
        info.displayName = info.name;
        info.node = root_;
        info.indexInParent = 0;
        info.hasGeometry = nodeBounds(root_.get(), info.aabbMin, info.aabbMax);
        nodes_.push_back(info);
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool SceneModel::isLoaded() const
{
    return loaded_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SceneModel::clear()
{
    nodes_.clear();
    root_ = nullptr;
    model_path_.clear();
    overall_min_ = vsg::dvec3(0.0, 0.0, 0.0);
    overall_max_ = vsg::dvec3(0.0, 0.0, 0.0);
    loaded_ = false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const QString& SceneModel::modelPath() const
{
    return model_path_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
vsg::ref_ptr<vsg::Node> SceneModel::root() const
{
    return root_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SceneModel::overallBounds(vsg::dvec3& min, vsg::dvec3& max) const
{
    min = overall_min_;
    max = overall_max_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double SceneModel::boundRadius() const
{
    if (!loaded_)
    {
        return 1.0;
    }

    const vsg::dvec3 size = overall_max_ - overall_min_;
    const double radius = 0.5 * vsg::length(size);

    return (radius > 0.01) ? radius : 1.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const std::vector<SceneNodeInfo>& SceneModel::nodes() const
{
    return nodes_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<SceneNodeInfo>& SceneModel::nodes()
{
    return nodes_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SceneNodeInfo* SceneModel::nodeByPath(const QString& path)
{
    for (SceneNodeInfo& info : nodes_)
    {
        if (info.path == path)
        {
            return &info;
        }
    }

    return nullptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SceneModel::traverse(vsg::Group* parent, vsg::Node* node,
                          const QString& path, size_t index)
{
    if (node == nullptr)
    {
        return;
    }

    SceneNodeInfo info;
    info.path = path;
    info.name = nodeObjectName(node);
    info.displayName = info.name;
    info.node = node;
    info.parent = parent;
    info.indexInParent = index;
    info.hasGeometry = nodeBounds(node, info.aabbMin, info.aabbMax);
    nodes_.push_back(info);

    // Обход детей: группой является и vsg::Group, и vsg::MatrixTransform
    if (vsg::Group* group = node->cast<vsg::Group>(); group != nullptr)
    {
        size_t child_index = 0;

        for (const vsg::ref_ptr<vsg::Node>& child : group->children)
        {
            traverse(group, child.get(),
                     QStringLiteral("%1/%2").arg(path).arg(child_index),
                     child_index);
            ++child_index;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int SceneModel::autoAssignRoles()
{
    int assigned = 0;

    for (SceneNodeInfo& info : nodes_)
    {
        // Роль имеет смысл только для узлов с геометрией,
        // уже назначенную роль эвристика не трогает
        if (!info.hasGeometry || info.role != MeshRole::None)
        {
            continue;
        }

        const MeshRole guessed = guessMeshRole(info.name);

        if (guessed != MeshRole::None)
        {
            info.role = guessed;
            ++assigned;
        }
    }

    return assigned;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SceneModel::CollisionParams SceneModel::collisionFromRoles() const
{
    CollisionParams params;

    // AABB ролей кузова
    std::vector<std::pair<vsg::dvec3, vsg::dvec3>> body_boxes;
    std::vector<std::pair<vsg::dvec3, vsg::dvec3>> bogie_boxes;
    std::vector<std::pair<vsg::dvec3, vsg::dvec3>> wheel_boxes;

    for (const SceneNodeInfo& info : nodes_)
    {
        if (!info.hasGeometry)
        {
            continue;
        }

        if (isBodyRole(info.role))
        {
            body_boxes.emplace_back(info.aabbMin, info.aabbMax);
        }
        else if (isBogieRole(info.role))
        {
            bogie_boxes.emplace_back(info.aabbMin, info.aabbMax);
        }
        else if (isWheelRole(info.role))
        {
            wheel_boxes.emplace_back(info.aabbMin, info.aabbMax);
        }
    }

    // Кузов: box по объединению AABB (ТЗ п.19)
    if (!body_boxes.empty())
    {
        vsg::dvec3 bmin = body_boxes.front().first;
        vsg::dvec3 bmax = body_boxes.front().second;

        for (const auto& box : body_boxes)
        {
            bmin = vsg::dvec3(std::min(bmin.x, box.first.x),
                              std::min(bmin.y, box.first.y),
                              std::min(bmin.z, box.first.z));
            bmax = vsg::dvec3(std::max(bmax.x, box.second.x),
                              std::max(bmax.y, box.second.y),
                              std::max(bmax.z, box.second.z));
        }

        params.bodyHalfLength = 0.5 * (bmax.y - bmin.y);
        params.bodyHalfWidth = 0.5 * (bmax.x - bmin.x);
        params.bodyHalfHeight = 0.5 * (bmax.z - bmin.z);
        params.bodyOffsetZ = 0.5 * (bmax.z + bmin.z);
        params.valid = true;
    }

    // Тележки: box по объединению AABB тележек, смещение — центр дальней тележки
    if (!bogie_boxes.empty())
    {
        vsg::dvec3 bmin = bogie_boxes.front().first;
        vsg::dvec3 bmax = bogie_boxes.front().second;
        double offset = 0.0;
        double offset_z = 0.0;

        for (const auto& box : bogie_boxes)
        {
            bmin = vsg::dvec3(std::min(bmin.x, box.first.x),
                              std::min(bmin.y, box.first.y),
                              std::min(bmin.z, box.first.z));
            bmax = vsg::dvec3(std::max(bmax.x, box.second.x),
                              std::max(bmax.y, box.second.y),
                              std::max(bmax.z, box.second.z));
            offset = std::max(offset, std::abs(0.5 * (box.first.y + box.second.y)));
            offset_z += 0.5 * (box.first.z + box.second.z);
        }

        params.numBogies = static_cast<int>(bogie_boxes.size());
        params.bogieHalfLength = 0.5 * (bmax.y - bmin.y);
        params.bogieHalfWidth = 0.5 * (bmax.x - bmin.x);
        params.bogieHalfHeight = 0.5 * (bmax.z - bmin.z);
        params.bogieOffset = offset;
        params.bogieOffsetZ = offset_z / static_cast<double>(bogie_boxes.size());
        params.valid = true;
    }

    // Колёсные пары: цилиндр по AABB колеса
    if (!wheel_boxes.empty())
    {
        double radius_sum = 0.0;
        double half_width_sum = 0.0;
        std::vector<double> axis_y;

        for (const auto& box : wheel_boxes)
        {
            // Радиус — по вертикальному (Z) размеру колеса
            radius_sum += 0.5 * (box.second.z - box.first.z);
            half_width_sum += 0.5 * (box.second.x - box.first.x);
            axis_y.push_back(0.5 * (box.first.y + box.second.y));
        }

        params.numAxis = static_cast<int>(wheel_boxes.size());
        params.wheelRadius = radius_sum / static_cast<double>(wheel_boxes.size());
        params.wheelsetHalfWidth =
            half_width_sum / static_cast<double>(wheel_boxes.size());
        params.valid = true;

        // Межосевое расстояние — минимальный шаг между осями вдоль пути
        std::sort(axis_y.begin(), axis_y.end());

        constexpr double min_gap = 0.1;

        for (size_t i = 1; i < axis_y.size(); ++i)
        {
            const double gap = axis_y[i] - axis_y[i - 1];

            if (gap <= min_gap)
            {
                continue;
            }

            if (params.wheelsetSpacing <= 0.0 || gap < params.wheelsetSpacing)
            {
                params.wheelsetSpacing = gap;
            }
        }
    }

    return params;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QStringList SceneModel::findLodVariants() const
{
    QStringList variants;

    if (!loaded_)
    {
        return variants;
    }

    variants << model_path_;

    const QFileInfo base_info(model_path_);
    const QDir dir = base_info.absoluteDir();
    const QString suffix = base_info.suffix().toLower();

    // Ищем рядом с моделью файлы вида <имя>_lod1 / <имя>-lod1 / *lod1*
    const QStringList entries =
        dir.entryList(QStringList() << QStringLiteral("*.gltf")
                                    << QStringLiteral("*.glb"),
                      QDir::Files, QDir::Name);

    for (const QString& entry : entries)
    {
        const QString lower = entry.toLower();

        if (lower == base_info.fileName().toLower())
        {
            continue;
        }

        if (!lower.contains(QStringLiteral("lod")))
        {
            continue;
        }

        if (!suffix.isEmpty() && QFileInfo(entry).suffix().toLower() != suffix)
        {
            continue;
        }

        variants << dir.absoluteFilePath(entry);
    }

    variants.removeDuplicates();

    return variants;
}
