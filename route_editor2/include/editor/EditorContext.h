#ifndef EDITOR_CONTEXT_H
#define EDITOR_CONTEXT_H

#include "editor/KeyBindings.h"
#include "editor/MutexedVector.h"
#include "editor/RouteMap.h"
#include "editor/RouteObject.h"
#include "editor/TrackProfile.h"

#include <vsgImGui/RenderImGui.h>
#include "editor/commands/CommandList.h"
#include "editor/states/EditorState.h"

#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec2.h>
#include <vsg/nodes/MatrixTransform.h>

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class Camera;
class EventHandler;
class Gizmo;
class IntersectionHandler;
class MeasureTool;
class ObjectSelector;
class Route;
class SplineTool;
class Topology;
class Trajectory;
class TrajectoryPicker;
struct camera_settings_t;
struct gui_settings_t;

namespace vsg
{

class Group;
class Options;
class PagedLOD;
class Switch;
class Viewer;

}

/// Ссылка на модель из objects.ref
struct ObjectRef
{
    std::string relative_path;
    vsg::ref_ptr<vsg::PagedLOD> paged_lod;
};

/// Информация о компиляции нового узла сцены в рендерер
struct CompileInfo
{
    vsg::ref_ptr<vsg::Node> group_node;
    vsg::ref_ptr<vsg::Node> node;
    vsg::Mask mask = vsg::MASK_OFF;
};

/// Данные точки из waypoints.conf
struct WaypointData
{
    std::string trajectory_name;
    int direction = 0;
    double coord = 0.0;
    double length = 0.0;
};

/// Сгенерированный элемент обвеса пути (опора КС, платформа,
/// километровый столбик): узел в generated_group + метаданные
/// для секции <Generated> в track-edit.conf
struct GeneratedItem
{
    vsg::ref_ptr<vsg::MatrixTransform> node;
    GeneratedConfig meta;
};

/// Общее состояние редактора: настройки, сцена, маршрут, команды
struct EditorContext
{
    EditorContext(camera_settings_t& camera_settings_,
                  gui_settings_t& gui_settings_);
    ~EditorContext();

    EditorContext(const EditorContext&) = delete;
    EditorContext& operator=(const EditorContext&) = delete;

    /// Текущее состояние редактора
    EditorState state = EditorState::NO_ROUTE;

    camera_settings_t& camera_settings;
    gui_settings_t& gui_settings;

    /// Список команд для undo/redo
    CommandList commands;

    vsg::ref_ptr<vsg::Options> options;
    vsg::ref_ptr<vsg::Group> scenegraph;
    vsg::ref_ptr<Camera> camera;
    vsg::ref_ptr<vsg::Viewer> viewer;

    vsg::ref_ptr<EventHandler> event_handler;
    vsg::ref_ptr<IntersectionHandler> intersection_handler;
    vsg::ref_ptr<ObjectSelector> object_selector;
    vsg::ref_ptr<TrajectoryPicker> trajectory_picker;

    /// Статические объекты маршрута (строки route1.map)
    RouteObjects static_objects;
    std::mutex static_objects_mutex;
    std::atomic_size_t static_objects_count = 0;
    std::atomic_size_t total_static_objects_count = 0;

    /// Выделенные объекты
    RouteObjects selected_objects;

    /// Гизмо трансформации выделенных объектов (клавиша G:
    /// цикл translate -> rotate -> scale -> off)
    vsg::ref_ptr<Gizmo> gizmo;

    /// Инструмент измерения (клавиша M: клики ставят точки)
    std::unique_ptr<MeasureTool> measure_tool;

    /// Инструмент построения новых путей (клавиша N: клики ставят
    /// опорные точки сплайна, окно «Новый путь»)
    std::unique_ptr<SplineTool> spline_tool;

    /// Таблица переназначаемых клавиш (окно «Клавиши»)
    KeyBindings key_bindings;

    /// Действие, ожидающее нажатия клавиши для назначения
    /// в окне «Клавиши» (-1 - ожидания нет)
    int key_binding_wait_action = -1;

    /// Привязка перемещения к сетке (клавиша X - вкл/выкл)
    bool snap_enabled = false;

    /// Шаг сетки привязки, м (клавиши [ и ]: 1/5/10)
    double snap_step = 1.0;

    /// Рамка выделения (ЛКМ + движение): активна и её экранные
    /// координаты в пикселях; рисуется EditorGui, завершается
    /// EventHandler по отпусканию кнопки
    bool selection_rect_active = false;
    vsg::ivec2 selection_rect_start = {0, 0};
    vsg::ivec2 selection_rect_curr = {0, 0};

    /// Режим "Пути" (клавиша P): ЛКМ выбирает траекторию,
    /// выделение объектов сцены отключается
    bool trajectory_mode = false;

    /// Режим параметрической стройки вдоль пути (ТЗ редактора,
    /// п.6/61): выбираем ЧТО строить в GUI, затем ЛКМ-растягивание
    /// вдоль выбранной траектории строит объект в диапазоне
    enum class BuildMode
    {
        None = 0,       ///< стройка выключена
        Catenary,       ///< опоры КС по интервалу
        Platform,       ///< платформа-лента
        Embankment,     ///< насыпь
        Cutting,        ///< выемка
        Ditch           ///< кювет
    };

    BuildMode build_mode = BuildMode::None;

    /// Параметры стройки (окно "Стройка" в GUI)
    double build_step_m = 50.0;         ///< интервал опор/столбов, м
    double build_platform_width_m = 4.0;
    double build_platform_height_m = 0.2;
    bool build_right_side = true;
    double build_embank_height_m = 2.0;
    double build_embank_shoulder_m = 4.0;
    double build_cut_depth_m = 2.0;
    double build_cut_width_m = 8.0;
    double build_ditch_width_m = 2.0;
    double build_ditch_depth_m = 0.6;

    /// Текущее растягивание: дуговые координаты начала/конца на
    /// выбранной траектории (update по движению мыши, применение
    /// по отпусканию ЛКМ)
    bool build_drag_active = false;
    double build_begin_m = 0.0;
    double build_end_m = 0.0;

    /// Превью полосы стройки: node в сцене (создаётся EventHandler,
    /// обновляется по move, удаляется по завершению)
    vsg::ref_ptr<vsg::Node> build_preview_node;

    /// Switch превью стройки (создаётся Route вместе с подсветкой
    /// траекторий, обновляется Route::show_build_preview)
    vsg::ref_ptr<vsg::Switch> build_preview_switch;

    /// ImGui-рендер сцены (для динамического добавления текстур карты)
    vsg::ref_ptr<vsgImGui::RenderImGui> render_gui;

    /// Режим установки объекта кликом (TSRE-style): выбранная в
    /// Model browser модель ставится ЛКМ по земле (серия объектов),
    /// [ / ] - поворот 15 град, Esc - выход
    bool place_object_mode = false;
    std::string place_object_label;
    double place_object_rotation_deg = 0.0;
    vsg::dvec3 place_cursor_world = {0.0, 0.0, 0.0};
    bool place_cursor_valid = false;

    /// Гео-якорь маршрута: широта/долгота точки (0,0) мировых координат
    /// (description.xml импортированных маршрутов; для полигона -
    /// Москва по умолчанию)
    double route_latitude = 55.75;
    double route_longitude = 37.6173;

    /// Выбранная траектория (режим "Пути"): имя + указатель
    Trajectory* selected_trajectory = nullptr;
    std::string selected_trajectory_name;

    /// Профили участков пути (track-edit.conf)
    TrackProfiles track_profiles;

    /// Группа сцены для сгенерированного обвеса пути (опоры КС,
    /// платформы, километровые столбики из окна «Путь» -> «Генерация»);
    /// создаётся в конструкторе Route
    vsg::ref_ptr<vsg::Group> generated_group;

    /// Сгенерированный обвес: узлы + метаданные
    /// (синхронно с children группы generated_group)
    std::vector<GeneratedItem> generated_items;

    /// Записи <Generated> из track-edit.conf, ожидающие применения:
    /// геометрия строится после загрузки топологии (TrackFurniture)
    std::vector<GeneratedConfig> pending_generated;

    /// Записи <Layer> из track-edit.conf - применяются к объектам
    /// при их загрузке по совпадению метки и позиции
    std::vector<LayerConfig> layer_configs;

    /// Записи <Prefab> из track-edit.conf (окно «Префабы»):
    /// группы объектов по имени префаба, вставляются копиями
    /// перед камерой командой PastePrefab
    std::vector<PrefabConfig> prefab_configs;

    /// Записи <ProposedTrack> из track-edit.conf (инструмент
    /// «Новый путь», клавиша N): сплайны новых путей по опорным
    /// точкам, геометрия строится TrackFurniture-ом
    std::vector<ProposedTrackConfig> proposed_tracks;

    /// Переключатель подсветки выбранной траектории
    /// (линия цианом поверх жёлтых линий, наполняется в Route)
    vsg::ref_ptr<vsg::Switch> trajectory_highlight_switch;

    /// Буфер обмена (Ctrl+C/Ctrl+V, копии создаются при вставке)
    RouteObjects clipboard_objects;

    /// Очередь узлов на компиляцию (наполняется из фоновых потоков)
    MutexedVector<CompileInfo> compile_infos;

    /// Топология маршрута (загружается в фоновом потоке)
    std::unique_ptr<Topology> topology;
    std::mutex topology_mutex;
    std::atomic_bool topology_loaded = false;
    std::atomic_size_t topology_objects_count = 0;
    std::atomic_size_t total_topology_objects_count = 0;

    std::thread load_static_objects_thread;
    std::thread load_topology_thread;

    std::map<std::string, ObjectRef> objects_ref;
    RouteMap route_map;
    std::map<std::string, vsg::dvec3> stations_conf;
    std::map<std::string, WaypointData> waypoints_conf;

    vsg::ref_ptr<Route> route;

    /// Каталог загруженного маршрута
    std::string route_dir;

    /// Время предыдущего кадра, с
    double delta_time = 0.0;

    /// Текст строки состояния (обновляется GUI и обработчиками)
    std::string status;

    /// Нажата ли Shift (обновляется EventHandler, используется выделением)
    bool shift_pressed = false;
};

#endif // EDITOR_CONTEXT_H
