#ifndef EDITOR_TRACK_PROFILE_H
#define EDITOR_TRACK_PROFILE_H

#include <vsg/maths/vec3.h>

#include <map>
#include <string>
#include <vector>

/// Конфигурация участка пути (одна траектория) из track-edit.conf
/// ("Система реалистичного профиля пути", ядро второго промта)
struct TrackProfile
{
    /// Тип рельсов (Р50, Р65, Р75)
    std::string rail_variant = "Р65";

    /// Материал шпал (дерево, жб)
    std::string sleeper_variant = "дерево";

    /// Материал балласта (щебень, песок)
    std::string ballast_variant = "щебень";

    /// Скрытые меши участка (рельс, шпала, балласт)
    bool rail_hidden = false;
    bool sleeper_hidden = false;
    bool ballast_hidden = false;

    /// Отмечать стыки рельсов: жёлтая поперечина каждые 25 м
    /// (промт п.12, упрощённо; опция геометрии TrackMesh)
    bool mark_joints = false;

    /// Высота начала участка, м
    double begin_elevation = 0.0;

    /// Высота конца участка, м (пересчитывается из уклона)
    double end_elevation = 0.0;

    /// Продольный уклон, тысячные (‰)
    double grade_promille = 0.0;

    /// Продольный уклон в десятичных долях (grade_promille / 1000)
    double grade_decimal = 0.0;

    /// Лимит скорости, км/ч (для авто-радиуса вертикальной кривой)
    double speed_limit = 80.0;

    /// Режим "авто" вертикальной переходной кривой
    /// (радиус выбирается по лимиту скорости)
    bool vertical_curve_auto = true;

    /// Радиус вертикальной переходной кривой, м (вне режима "авто")
    double vertical_curve_radius = 6000.0;

    /// Длина переходной кривой, м
    double transition_length = 0.0;

    /// Радиус вертикальной кривой по лимиту скорости (режим "авто"):
    /// 60 км/ч -> 2000 м, 80 -> 6000, 120 -> 15000, 160 -> 25000
    double auto_vertical_curve_radius() const;

    /// Пересчитать высоту конца из уклона и длины участка
    /// (end_elevation = begin_elevation + length * grade_decimal)
    void update_end_elevation(double length);

    /// Пересчитать уклон из высот начала и конца участка
    void update_grade(double length);
};

/// Профили всех участков маршрута по именам траекторий
using TrackProfiles = std::map<std::string, TrackProfile>;

/// Запись о сгенерированном элементе обвеса пути
/// (секция <Generated> в track-edit.conf: опоры КС, платформы,
/// километровые столбики, путь, деревья, вода, переезд из окна
/// «Путь» -> «Генерация»)
struct GeneratedConfig
{
    /// Тип элемента: CatenaryPole (опора КС), KmPost (километровый
    /// столбик), Platform (платформа), TrackMesh (путь: рельсы +
    /// шпалы + балласт), Trees (деревья), Water (вода),
    /// Crossing (переезд), Road (дорога у переезда),
    /// Embankment (насыпь), Cutting (выемка), Ditch (канава),
    /// ProposedTrack (новый путь инструментом «Новый путь», клавиша N)
    std::string kind;

    /// Имя траектории, вдоль которой построен элемент
    /// (пустая - элемент не привязан к траектории, напр. Water)
    std::string trajectory;

    /// Координата вдоль траектории, м (для платформы - начало ленты)
    double coord = 0.0;

    /// Сторона от оси пути: right / left
    std::string side = "right";

    /// Метка модели из objects.ref, если элемент построен из готовой
    /// модели (пустая - элемент собран vsg::Builder)
    std::string label;

    /// Габариты платформы, м (только для Kind == Platform);
    /// для Water Length/Width - размеры зеркала SizeX/SizeY;
    /// для Road Length - длина дороги; для Embankment Height/Wid -
    /// высота насыпи и ширина верха; для Cutting Height/Wid -
    /// глубина и ширина выемки; для Ditch Height/Wid - глубина
    /// и ширина канавы
    double length = 0.0;
    double width = 0.0;
    double height = 0.0;

    /// Интервал вдоль траектории, м (Kind == TrackMesh,
    /// Embankment, Cutting, Ditch)
    double from = 0.0;
    double to = 0.0;

    /// Плотность деревьев на км и отступы от оси, м
    /// (только для Kind == Trees)
    double per_km = 0.0;
    double offset_min = 0.0;
    double offset_max = 0.0;

    /// Мировая позиция центра и уровень воды, м (только для Water)
    double pos_x = 0.0;
    double pos_y = 0.0;
    double level = 0.0;
};

/// Запись о слое объекта (секция <Layer> в track-edit.conf):
/// объект опознаётся по метке и позиции на момент сохранения
struct LayerConfig
{
    /// Метка объекта (ключ objects.ref)
    std::string object_label;

    /// Позиция объекта в момент сохранения
    vsg::dvec3 position = {0.0, 0.0, 0.0};

    /// Имя слоя
    std::string name = "default";
};

/// Запись об объекте внутри префаба (секция <Prefab> в
/// track-edit.conf, окно «Префабы»): объект опознаётся по метке
/// и позиции на момент сохранения, как у слоёв
struct PrefabConfig
{
    /// Имя префаба (общее у всех объектов одного префаба)
    std::string name;

    /// Метка объекта (ключ objects.ref)
    std::string object_label;

    /// Позиция объекта в момент сохранения
    vsg::dvec3 position = {0.0, 0.0, 0.0};

    /// Поворот объекта в момент сохранения, градусы
    vsg::dvec3 rotation_deg = {0.0, 0.0, 0.0};

    /// Масштаб объекта в момент сохранения
    vsg::dvec3 scale = {1.0, 1.0, 1.0};
};

/// Запись о новом пути (секция <ProposedTrack> в track-edit.conf,
/// инструмент «Новый путь», клавиша N): сплайн Catmull-Rom по
/// опорным точкам, вдоль которого строится геометрия пути
struct ProposedTrackConfig
{
    /// Имя нового пути (общее у записи и Generated Kind=ProposedTrack)
    std::string name;

    /// Опорные точки сплайна в мировых координатах
    std::vector<vsg::dvec3> points;
};

/// Загрузка профилей из track-edit.conf
/// (отсутствие файла - не ошибка, возвращается пустой список)
bool load_track_profiles(const std::string& path, TrackProfiles& profiles);

/// Сохранение профилей в track-edit.conf
bool save_track_profiles(const std::string& path, const TrackProfiles& profiles);

/// Загрузка track-edit.conf целиком: профили участков + записи
/// сгенерированного обвеса + записи слоёв, префабов и новых путей
bool load_track_edit_data(const std::string& path, TrackProfiles& profiles,
    std::vector<GeneratedConfig>& generated,
    std::vector<LayerConfig>& layers,
    std::vector<PrefabConfig>& prefabs,
    std::vector<ProposedTrackConfig>& proposed_tracks);

/// Сохранение track-edit.conf целиком: профили участков + записи
/// сгенерированного обвеса + записи слоёв, префабов и новых путей
bool save_track_edit_data(const std::string& path,
    const TrackProfiles& profiles,
    const std::vector<GeneratedConfig>& generated,
    const std::vector<LayerConfig>& layers,
    const std::vector<PrefabConfig>& prefabs,
    const std::vector<ProposedTrackConfig>& proposed_tracks);

struct EditorContext;

/// Загрузка track-edit.conf в контекст: профили в track_profiles,
/// обвес - в pending_generated (применяется после загрузки топологии),
/// слои - в layer_configs, префабы - в prefab_configs, новые пути -
/// в proposed_tracks
bool load_track_edit_config(const std::string& path, EditorContext& context);

/// Сохранение track-edit.conf из контекста: профили + обвес
/// (generated_items) + слои объектов (layer != "default") + префабы
/// (prefab_configs) + новые пути (proposed_tracks).
/// Формат route1.map не затрагивается
bool save_track_edit_config(const std::string& path, EditorContext& context);

#endif // EDITOR_TRACK_PROFILE_H
