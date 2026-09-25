#ifndef EDITOR_TRACK_FURNITURE_H
#define EDITOR_TRACK_FURNITURE_H

#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>

#include <cstddef>
#include <limits>
#include <string>
#include <vector>

struct EditorContext;
struct TrackProfile;

namespace vsg
{

class Group;
class Node;

}

/// Генерация обвеса пути вдоль траекторий (окно «Путь», секция
/// «Генерация», промт п.9-10 в упрощённом виде):
///  - опоры контактовой сети каждые N метров;
///  - платформа-лента вдоль пути (сегменты по 10 м);
///  - километровые столбики каждые 1000 м;
///  - верхнее строение пути (рельсы + шпалы + балласт);
///  - деревья вдоль пути, плоскость воды, переезды с дорогами;
///  - терраформинг вдоль пути: насыпь, выемка, канава;
///  - новые пути по сплайну Catmull-Rom (инструмент «Новый путь»).
///
/// Если в objects.ref есть метка по подстроке (catenary / ks_pole /
/// опора / pole) - элемент строится из готовой модели (PagedLOD),
/// иначе собирается простой столб vsg::Builder-ом с flat shaderSet
/// (подход скопирован из Gizmo.cpp). Всё сгенерированное живёт
/// в context.generated_group, не попадает в route1.map и хранится
/// в track-edit.conf секциями <Generated .../>.
class TrackFurniture
{
public:
    /// Опоры КС каждые step_m метров вдоль траектории:
    /// точка пути getPosition + смещение вправо на 3.5 м
    static bool generate_catenary_poles(EditorContext& context,
        const std::string& trajectory_name, double step_m,
        double begin_m = 0.0,
        double end_m = std::numeric_limits<double>::max());

    /// Платформа-лента вдоль начала траектории (сегменты по 10 м):
    /// length/width/height - габариты, right_side - сторона от оси
    static bool generate_platform(EditorContext& context,
        const std::string& trajectory_name, double length_m,
        double width_m, double height_m, bool right_side,
        double begin_m = 0.0);

    /// Километровые столбики каждые 1000 м (маленький столбик
    /// с табличкой, смещение 2.5 м от оси)
    static bool generate_km_posts(EditorContext& context,
        const std::string& trajectory_name);

    /// Собрать геометрию верхнего строения пути по вектору позиций
    /// оси (мировые координаты): рельсы, шпалы, балласт и стыки
    /// по TrackProfile. Общая сборка для траектории (generate_
    /// track_mesh) и сплайна нового пути (generate_proposed_track);
    /// направление и правая сторона выводятся из соседних точек.
    /// sleeper_count/sleeper_step_increased - необязательные счётчики
    static vsg::ref_ptr<vsg::Node> build_track_geometry(
        EditorContext& context, const std::vector<vsg::dvec3>& points,
        const TrackProfile& profile,
        std::size_t* sleeper_count = nullptr,
        bool* sleeper_step_increased = nullptr);

    /// Верхнее строение пути на интервале [from, to] м траектории:
    /// две рельсовые полосы на междурельсовом 1520 мм, шпалы
    /// и балластная призма по TrackProfile (варианты рельсов/шпал,
    /// скрытие мешей, отметка стыков)
    static bool generate_track_mesh(EditorContext& context,
        const std::string& trajectory_name, double from, double to,
        const TrackProfile& profile);

    /// Новый путь по сплайну Catmull-Rom (инструмент «Новый путь»,
    /// клавиша N): запись ProposedTrack в context.proposed_tracks
    /// + геометрия пути вдоль сэмплов кривой (шаг 5 м)
    static bool generate_proposed_track(EditorContext& context,
        const std::string& name,
        const std::vector<vsg::dvec3>& control_points);

    /// Насыпь на интервале [from, to] м: трапецеидальная земляная
    /// лента с верхом шириной shoulder м и откосом 1:1.5
    /// (ступенчатая аппроксимация), высота height м
    static bool generate_embankment(EditorContext& context,
        const std::string& trajectory_name, double from, double to,
        double height, double shoulder);

    /// Выемка на интервале [from, to] м: приподнятые земляные
    /// стенки по краям пути (width м от оси) на глубину depth м
    static bool generate_cutting(EditorContext& context,
        const std::string& trajectory_name, double from, double to,
        double depth, double width);

    /// Канава-кювет на интервале [from, to] м: узкий заглублённый
    /// бокс на offset width/2+2.0 м от оси выбранной стороны
    static bool generate_ditch(EditorContext& context,
        const std::string& trajectory_name, double from, double to,
        bool right_side, double width, double depth);

    /// Деревья вдоль всей траектории: per_km штук на км
    /// с отступом offset_min..offset_max м от оси (side - сторона),
    /// детерминированный ПСЧ с seed из имени траектории
    static bool generate_trees_along(EditorContext& context,
        const std::string& trajectory_name, bool right_side,
        double per_km, double offset_min, double offset_max);

    /// Плоскость воды: полупрозрачный синий бокс размером
    /// size_x x size_y м с зеркалом на отметке level
    static bool generate_water(EditorContext& context,
        const vsg::dvec3& center, double size_x, double size_y,
        double level);

    /// Переезд на координате coord: настил поперёк пути
    /// + два столбика шлагбаума на ±4 м от оси + асфальтовая дорога
    /// длиной road_length м перпендикулярно пути в обе стороны
    /// (отдельная запись Kind=Road, удаляется вместе с переездом)
    static bool generate_crossing(EditorContext& context,
        const std::string& trajectory_name, double coord,
        double road_length = 50.0);

    /// Удалить весь обвес траектории: узлы из generated_group,
    /// записи из generated_items и pending_generated; дороги (Road)
    /// удаляются вместе со своими переездами (Traj + Coord)
    static bool remove_generated(EditorContext& context,
        const std::string& trajectory_name);

    /// Восстановить обвес из записей track-edit.conf (context.
    /// pending_generated) - вызывается после загрузки топологии,
    /// т.к. построение требует сэмплирования траекторий
    static void restore_all(EditorContext& context);

private:
    TrackFurniture() = default;
};

#endif // EDITOR_TRACK_FURNITURE_H
