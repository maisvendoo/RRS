#ifndef SCENE_MODEL_H
#define SCENE_MODEL_H

#include "mesh-roles.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>

#include <QString>
#include <QStringList>

#include <vector>

namespace vsg
{

class Animation;
class Group;
class Node;

} // namespace vsg

//------------------------------------------------------------------------------
//
//  Сведения об одном узле графа загруженной модели ПС
//
//------------------------------------------------------------------------------
struct SceneNodeInfo
{
    QString path;                       ///< Путь в графе, например "0/2/1"
    QString name;                       ///< Имя объекта модели (или класс vsg)
    QString displayName;                ///< Display-имя (можно переименовать)
    MeshRole role = MeshRole::None;     ///< Игровая роль
    bool visible = true;                ///< Видимость меша в вьюпорте
    bool hasGeometry = false;           ///< Узел (или поддерево) содержит геометрию
    vsg::ref_ptr<vsg::Node> node;       ///< Сам узел
    vsg::ref_ptr<vsg::Group> parent;    ///< Родительская группа
    size_t indexInParent = 0;           ///< Позиция узла в родителе
    vsg::dvec3 aabbMin = vsg::dvec3(0.0, 0.0, 0.0); ///< AABB узла, минимум
    vsg::dvec3 aabbMax = vsg::dvec3(0.0, 0.0, 0.0); ///< AABB узла, максимум
};

//------------------------------------------------------------------------------
//
//  Модель сцены ПС: загруженная glTF-модель + плоский список узлов дерева.
//  Оси локальной системы модели: X — вправо, Y — вдоль пути, Z — вверх.
//
//------------------------------------------------------------------------------
class SceneModel
{
public:
    /// Параметры секции [Collision] конфига ПС
    /// (формат симулятора, simulator/vehicle/src/vehicle-collision.cpp)
    struct CollisionParams
    {
        bool valid = false;             ///< Роли назначены, данные вычислены

        double bodyHalfLength = 0.0;    ///< BodyHalfLength, м
        double bodyHalfWidth = 0.0;     ///< BodyHalfWidth, м
        double bodyHalfHeight = 0.0;    ///< BodyHalfHeight, м
        double bodyOffsetZ = 0.0;       ///< BodyOffsetZ, м

        int numBogies = 0;              ///< NumBogies
        double bogieHalfLength = 0.0;   ///< BogieHalfLength, м
        double bogieHalfWidth = 0.0;    ///< BogieHalfWidth, м
        double bogieHalfHeight = 0.0;   ///< BogieHalfHeight, м
        double bogieOffset = 0.0;       ///< BogieOffset, м
        double bogieOffsetZ = 0.0;      ///< BogieOffsetZ, м

        double wheelsetSpacing = 0.0;   ///< WheelsetSpacing, м
        double wheelsetHalfWidth = 0.0; ///< WheelsetHalfWidth, м
        double wheelRadius = 0.0;       ///< WheelRadius, м
        int numAxis = 0;                ///< Число осей (по ролям Wheel)
    };

    /// Загрузить модель (glTF/GLB через vsgXchange); false + текст ошибки при неудаче
    bool loadModel(const QString& path, QString* error = nullptr);

    /// Модель загружена
    bool isLoaded() const;

    /// Полностью очистить модель
    void clear();

    /// Путь к файлу модели
    const QString& modelPath() const;

    /// Корневой узел загруженной модели
    vsg::ref_ptr<vsg::Node> root() const;

    /// Сводный AABB модели (min/max)
    void overallBounds(vsg::dvec3& min, vsg::dvec3& max) const;

    /// Радиус ограничивающей сферы модели, м
    double boundRadius() const;

    /// Плоский список узлов (в порядке обхода графа)
    const std::vector<SceneNodeInfo>& nodes() const;
    std::vector<SceneNodeInfo>& nodes();

    /// Анимации glTF-модели, собранные при загрузке из узлов
    /// vsg::AnimationGroup (загрузчик vsgXchange оборачивает
    /// анимированную сцену в AnimationGroup)
    const std::vector<vsg::ref_ptr<vsg::Animation>>& animations() const;

    /// Узел по пути ("0/2/1"); nullptr, если не найден
    SceneNodeInfo* nodeByPath(const QString& path);

    /// Присвоить роли всем узлам эвристикой по имени (ТЗ п.3)
    /// \return число узлов, которым назначена роль
    int autoAssignRoles();

    /// Вычислить параметры [Collision] по AABB мешей с ролями
    /// Body/Bogie*/Trolley/Wheel (ТЗ п.19)
    CollisionParams collisionFromRoles() const;

    /// Варианты LOD в папке модели: [базовый, lod1, lod2, ...]
    /// (ТЗ п.20 — только обнаружение и превью, генерации нет)
    QStringList findLodVariants() const;

private:
    /// Рекурсивный обход графа, заполняющий nodes_
    void traverse(vsg::Group* parent, vsg::Node* node, const QString& path,
                  size_t index);

    QString model_path_;
    vsg::ref_ptr<vsg::Node> root_;
    std::vector<SceneNodeInfo> nodes_;
    std::vector<vsg::ref_ptr<vsg::Animation>> animations_;
    vsg::dvec3 overall_min_ = vsg::dvec3(0.0, 0.0, 0.0);
    vsg::dvec3 overall_max_ = vsg::dvec3(0.0, 0.0, 0.0);
    bool loaded_ = false;
};

#endif // SCENE_MODEL_H
