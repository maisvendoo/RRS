#ifndef EDITOR_GUI_H
#define EDITOR_GUI_H

#include "editor/MiniMap.h"
#include "editor/Validator.h"

#include <vsg/commands/Command.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>
#include <vsgImGui/imgui.h>

#include <cstddef>
#include <memory>
#include <map>
#include <string>
#include <vector>

struct EditorContext;
struct gui_settings_t;
class RouteObject;

/**
 * @brief Состояние перетаскивания отдельного объекта в инспекторе
 *        (по-объектно, чтобы дельты не смешивались между объектами).
 */
struct ObjectDragState
{
    bool dragging = false;
    vsg::dvec3 total_translation = {0.0, 0.0, 0.0};
    vsg::dvec3 total_rotation_deg = {0.0, 0.0, 0.0};
    vsg::dvec3 total_scale = {1.0, 1.0, 1.0};
};

/// ImGui-интерфейс редактора
class EditorGui : public vsg::Inherit<vsg::Command, EditorGui>
{
public:
    EditorGui(EditorContext& context, const gui_settings_t& gui_settings);

    virtual void record(vsg::CommandBuffer& commandBuffer) const override;

private:
    void draw_menu_bar() const;

    void draw_status_bar() const;

    void draw_open_route_file_dialog() const;
    void draw_new_route_dir_dialog() const;
    void draw_new_route_popup() const;
    void draw_invalid_route_popup() const;

    void draw_objects_ref_window() const;
    void draw_route_map_window() const;
    void draw_stations_window() const;
    void draw_topology_window() const;
    void draw_selected_objects_window() const;
    void draw_commands_window() const;
    void draw_track_window() const;
    void draw_progress_window() const;

    /// Окно переназначаемых клавиш (промт Этапа 1)
    void draw_key_bindings_window() const;

    /// Окно «Валидация» маршрута (промт п.33)
    void draw_validator_window() const;

    /// Окно «Model browser»: дерево категорий objects.ref + список
    /// меток с поиском, избранным и добавлением в сцену (промт п.44-45)
    void draw_model_browser_window() const;

    /// Окно «Mass edit»: фильтр по метке, замена метки, сброс
    /// масштаба, удаление отфильтрованных (промт п.34-35)
    void draw_mass_edit_window() const;

    /// Окно «Слои»: видимость слоёв и назначение слоя выделенным
    /// объектам (промт п.30-31, упрощённо)
    void draw_layers_window() const;

    /// Окно «Префабы»: сохранение выделенного набора объектов
    /// как префаба и вставка копий перед камерой (PastePrefab)
    void draw_prefabs_window() const;

    /// Вставить префаб копиями объектов перед камерой командой
    /// PastePrefab (первый объект - в точку перед камерой)
    void paste_prefab(const std::string& name) const;

    /// Секция «Генерация» окна «Путь»: опоры КС, платформа,
    /// километровые столбики, путь, деревья, вода, переезд с дорогой,
    /// терраформинг (насыпь/выемка/канава) (промт п.9-11, упрощённо)
    void draw_track_generation_section(const std::string& trajectory_name) const;

    /// Секция «Разложить объект» окна «Путь»: копии выбранного
    /// объекта вдоль траектории каждые N метров с отступом от оси
    /// (промт п.7/29, упрощённо)
    void draw_follow_path_section(const std::string& trajectory_name) const;

    /// Параметрическая стройка растягиванием вдоль пути (ТЗ п.6/61)
    void draw_build_section(const std::string& trajectory_name) const;

    /// Мини-карта (TSRE MapWindow): подложка OSM/спутник, схема путей
    void draw_minimap() const;

    /// Мини-карта (создаётся лениво в draw)
    mutable std::unique_ptr<MiniMap> minimap;

    /// Сохранить track-edit.conf целиком: профили + обвес + слои
    void save_track_edit_conf() const;

    /// Избранное Model browser: чтение из editor-settings.xml
    /// (секция Favorites, вызывается в конструкторе)
    void load_favorites();

    /// Избранное Model browser: перезапись секции Favorites
    /// в editor-settings.xml (по образцу KeyBindings::save)
    bool save_favorites() const;

    /// Метка в списке избранного
    bool is_favorite(const std::string& label) const;

    /// Добавить/убрать метку из избранного (с сохранением файла)
    void toggle_favorite(const std::string& label) const;

    /// Рамка выделения поверх окон (ImGui foreground draw list)
    void draw_selection_rect() const;

    void add_object(const std::string& label) const;

    ObjectDragState& get_drag_state(const RouteObject* object) const;

    void save_objects_matrixes() const;

    void handle_translation_drag(
        std::size_t index,
        vsg::ref_ptr<RouteObject> object,
        ObjectDragState& drag_state
    ) const;

    void handle_rotation_drag(
        std::size_t index,
        vsg::ref_ptr<RouteObject> object,
        ObjectDragState& drag_state
    ) const;

    void handle_scale_drag(
        std::size_t index,
        vsg::ref_ptr<RouteObject> object,
        ObjectDragState& drag_state
    ) const;

    /// Создать структуру каталогов нового маршрута и открыть его
    bool create_route_structure(const std::string& parent_dir,
        const std::string& name) const;

private:
    EditorContext& context_;
    const gui_settings_t& gui_settings;

    ImGuiViewport* viewport = nullptr;

    /// Родительский каталог для нового маршрута (окно New route)
    mutable std::string new_route_parent_dir;

    /// Имя нового маршрута (окно New route)
    mutable char new_route_name[256] = {};

    /// Состояния перетаскивания по каждому объекту-инспектору
    mutable std::map<const RouteObject*, ObjectDragState> drag_states_;

    /// Результаты последней валидации маршрута (окно «Валидация»)
    mutable std::vector<ValidationIssue> validation_issues;
    mutable bool validation_done = false;

    /// Model browser: строка поиска и выбранная категория (0 - «Все»)
    mutable char model_browser_filter[256] = {};
    mutable int model_browser_category = 0;

    /// Избранные метки Model browser
    /// (editor-settings.xml, секция Favorites)
    mutable std::vector<std::string> favorites_;

    /// Mass edit: фильтр по текущей метке и новая метка
    mutable char mass_edit_filter[256] = {};
    mutable char mass_edit_new_label[256] = {};

    /// Секция «Генерация» окна «Путь»: параметры обвеса
    mutable double catenary_step = 50.0;
    mutable double platform_length = 50.0;
    mutable double platform_width = 4.0;
    mutable double platform_height = 0.25;
    mutable int platform_side_index = 0;

    /// Секция «Генерация» окна «Путь»: параметры деревьев,
    /// воды и переезда (дорога у переезда - промт п.11)
    mutable int trees_side_index = 0;
    mutable double trees_per_km = 100.0;
    mutable double trees_offset_min = 10.0;
    mutable double trees_offset_max = 30.0;
    mutable double crossing_coord = 0.0;
    mutable double crossing_road_length = 50.0;

    /// Секция «Генерация»: терраформинг вдоль пути
    /// (насыпь/выемка/канава)
    mutable double embankment_from = 0.0;
    mutable double embankment_to = 100.0;
    mutable double embankment_height = 2.0;
    mutable double embankment_shoulder = 4.6;
    mutable double cutting_from = 0.0;
    mutable double cutting_to = 100.0;
    mutable double cutting_depth = 2.0;
    mutable double cutting_width = 8.0;
    mutable double ditch_from = 0.0;
    mutable double ditch_to = 100.0;
    mutable double ditch_width = 1.0;
    mutable double ditch_depth = 0.8;
    mutable int ditch_side_index = 0;

    /// Секция «Разложить объект»: метка, шаг, отступ и поворот
    mutable char follow_path_label[256] = {};
    mutable double follow_path_step = 25.0;
    mutable double follow_path_offset = 5.0;
    mutable bool follow_path_rotate = true;

    /// Окно «Слои»: имя нового слоя, индекс слоя для назначения
    /// и состояние видимости слоёв (имя -> видимость)
    mutable char layer_new_name[128] = {};
    mutable int layer_assign_index = 0;
    mutable std::map<std::string, bool> layer_visibility_;

    /// Окно «Префабы»: имя нового префаба и выбранный в списке
    mutable char prefab_name[128] = {};
    mutable std::string selected_prefab;
};

#endif // EDITOR_GUI_H
