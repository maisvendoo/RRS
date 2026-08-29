#pragma once
#ifndef VIEWER_SETTINGS_H
#define VIEWER_SETTINGS_H

#include "tcp-client.h"

#include <vsg/maths/vec3.h>
#include <vsg/maths/vec4.h>

#include <cmath>
#include <string>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct settings_t final
{
    tcp_config_t tcp_config;   ///< TCP-Client settings

    int vehicles_pos_update_interval = 70;      ///< Interval for vehicles positions update, ms
    int vehicles_state_update_interval = 100;   ///< Interval for vehicles states update, ms
    int vehicle_controled_update_interval = 70; ///< Interval for vehicle controlled debug strings update, ms
    int train_profile_update_interval = 1000;    ///< Interval for trains' path profiles update, ms
    double train_profile_backward = 4000.0;      ///< Profile extent backward from train middle, m
    double train_profile_forward = 4000.0;       ///< Profile extent forward from train middle, m
    int client_delay = 100;                     ///< Client delay for smoothing network's delays

    std::string route_dir_name;        ///< Route directory name
    std::string route_dir_full_path;   ///< Route directory path

    std::string notify_level = "INFO";      ///< Notify level
    uint32_t targetPagedLODs = 64000;       ///< Set number of maximum PagedLOD with loaded high-resolution subgraphs
    uint32_t read_threads = 1;              ///< Number of background threads for loading 3d-models
    uint32_t operation_threads = 0;         ///< Number of background threads for loading textures of 3d-models
    bool disable_culling_node = false;      ///< Disable automatic cullnode for models
    bool disable_native_gltf_loader = false;///< Use assimp loader for .gltf models
    bool draw_models_two_sided = false;     ///< Draw all model's faces two sided
    double cullingScreenHeightRatio = 0.005;///< Ratio of screen height that a bounding sphere of 3d-model needs to occupy to be visible, 0.0-1.0
    double culling_tiles_size_0 = 4000.0;   ///< Large-scale tile size for cascade culling of route objects, m
    double culling_tiles_size_1 = 32000.0;  ///< Small-scale tile size for cascade culling of route objects, m

    double stations_text_font_size = 10.0;              ///< Font size of station name labels in scene
    vsg::dvec3 stations_text_shift = {0.0, 0.0, 15.0};  ///< Station name labels shift, m
    double stations_text_scale_distance = 500.0;         ///< Distance after which station name labels are scaled down with distance, m

    std::string name = "viewer";///< Window title
    int x = 50;                 ///< Window horizontal position
    int y = 50;                 ///< Window vertical position
    int width = 1280;           ///< Window width
    int height = 720;           ///< Window height
    int physical_device = 0;    ///< Physical device
    int screen_number = 0;      ///< Screen number
    bool fullscreen = false;    ///< Fullscreen flag
    bool vsync = true;          ///< Vertical sync flag
    bool window_decoration = true;  ///< Set/unset window decorations

    bool double_buffer = true;  ///< Set/unset double buffering
    int samples = 1;            ///< Set number of antialiasing samples
    int depthFormat = 1;        ///< Set depth buffer format
    int max_fps = 60;           ///< Maximum frames per second (0 = unlimited)

    bool shadow = false;                        ///< Shadow flag
    double shadow_distance = 50.0;              ///< Shadow compute distance
    int shadow_cascade = 1;                     ///< Shadow cascade
    int shadow_resolution = 4096;               ///< Shadow map resolution
    int num_lights = 200;                       ///< Maximum lights in scene
    double ambient_intensity = 0.5;             ///< Ambient light intensity
    vsg::dvec3 ambient_color = {1.0, 1.0, 1.0}; ///< Ambient light color
    double sun_intensity = 5.0;                 ///< Directional light intensity
    vsg::dvec3 sun_color = {1.0, 1.0, 1.0};     ///< Directional light color

    double view_distance = 2000.0;  ///< View distance
    double zNear = 0.1;
//    double zFar = 2000.0;
    double fovy = 64.0;         ///< Vertical view angle
    double fovy_min = 2.0;      ///< Vertical view angle min
    double fovy_max = 100.0;    ///< Vertical view angle max
    double pitch_min = -70.0;   ///< Vertical angle down max
    double pitch_max = 70.0;    ///< Vertical angle up max

    vsg::dvec3 free_cam_start = {2.5, 1900.0, 1.75};///< Free camera start position
    vsg::dvec3 free_cam_init_pos = {2.5, 0.0, 1.75};///< Free camera initial position
    double free_cam_speed_keyboard = 5.0;           ///< Free camera initial speed
    double free_cam_speed_mouse = 5.0;              ///< Free camera initial speed
    double free_cam_speed_coeff = std::sqrt(2.0);   ///< Free camera speed coeff
    double free_cam_rotate_keyboard = 0.01;         ///< Free camera rotation initial speed
    double free_cam_rotate_mouse = 0.01;            ///< Free camera rotation initial speed
    double free_cam_height_step = 0.2;              ///< Free camera vertical shift step
    double free_cam_fovy_coeff = std::cbrt(2.0);    ///< Free camera FovY coeff

    vsg::dvec3 cabine_default_pos = {0.0, 0.0, 3.5};///< Driver default initial position
    double cabine_speed_keyboard = 0.5;             ///< Cabine camera initial speed
    double cabine_speed_mouse = 0.5;                ///< Cabine camera initial speed
    double cabine_speed_coeff = std::sqrt(2.0);     ///< Cabine camera speed coeff
    double cabine_rotate_keyboard = 0.01;           ///< Cabine camera rotation initial speed
    double cabine_rotate_mouse = 0.01;              ///< Cabine camera rotation initial speed
    double cabine_height_step = 0.1;                ///< Cabine camera vertical shift
    double cabine_fovy_coeff = std::cbrt(2.0);      ///< Cabine camera FovY coeff
    double cabine_z_min = -1.0;                     ///< Cabine camera relative vertical shift limit
    double cabine_z_max = 0.5;                      ///< Cabine camera relative vertical shift limit

    vsg::dvec3 ext_cam_init_pos = {0.0, 0.0, 1.75};         ///< External camera initial position
    double ext_cam_init_angle_H = -45.0;                    ///< External camera initial horizontal angle
    double ext_cam_init_angle_V = 10.0;                     ///< External camera initial vertical angle
    double ext_cam_init_distance = 20.0;                    ///< External camera initial distance
    double ext_cam_speed_keyboard = 2.0;                    ///< External camera initial speed
    double ext_cam_speed_mouse = 2.0;                       ///< External camera initial speed
    double ext_cam_speed_coeff = std::sqrt(2.0);            ///< External camera speed coeff
    double ext_cam_rotate_keyboard = 1.0;                   ///< External camera rotation initial speed
    double ext_cam_rotate_mouse = 1.0;                      ///< External camera rotation initial speed
    double ext_cam_height_step = 0.1;                       ///< External camera vertical shift
    double ext_cam_dist_coeff = std::cbrt(std::cbrt(2.0));  ///< External camera distance coeff
    double ext_cam_dist_min = 1.0;                          ///< External camera minimal distance

    double follow_cam_init_shift_forward = 15.0;    ///< Follow camera initial position forward shift
    double follow_cam_init_shift_right = 10.0;      ///< Follow camera initial position right shift
    double follow_cam_init_shift_up = 1.75;         ///< Follow camera initial position height shift
    double follow_cam_fwd_velocity_coeff = 4.0;     ///< Follow camera forward shift by velocity coeff
    double follow_cam_speed_keyboard = 2.0;         ///< Follow camera initial speed
    double follow_cam_speed_mouse = 2.0;            ///< Follow camera initial speed
    double follow_cam_speed_coeff = std::sqrt(2.0); ///< Follow camera speed coeff
    double follow_cam_height_step = 0.1;            ///< Follow camera vertical shift
    double follow_cam_fovy_coeff = std::cbrt(2.0);  ///< Follow camera FovY coeff

    bool enableDebugLayer = false;
    bool enableDebugUtils = false;
    double allocatedMemoryLimit = 1.0;

    // Цвета виджетов интерфейса (HUD), RGBA в диапазоне 0.0 - 1.0
    vsg::vec4 hud_background = {0.0f, 0.0f, 0.0f, 0.8f};           ///< Фон виджетов интерфейса
    vsg::vec4 hud_text = {1.0f, 1.0f, 1.0f, 1.0f};                 ///< Текст интерфейса
    vsg::vec4 hud_button_off = {1.0f, 0.75f, 0.75f, 0.8f};         ///< Ненажатые кнопки
    vsg::vec4 hud_button_on = {0.75f, 1.0f, 0.75f, 0.8f};          ///< Нажатые кнопки
    vsg::vec4 hud_button_hovered = {1.0f, 1.0f, 0.75f, 0.8f};      ///< Кнопки при наведении
    vsg::vec4 hud_button_inactive = {0.3f, 0.3f, 0.3f, 0.8f};      ///< Неактивные кнопки
    vsg::vec4 hud_button_inactive_text = {0.5f, 0.5f, 0.5f, 1.0f}; ///< Текст неактивных кнопок

    // Цвета выделения поездов и предупреждений, RGBA в диапазоне 0.0 - 1.0
    vsg::vec4 hud_current_train = {1.0f, 1.0f, 0.0f, 1.0f};       ///< Текущий поезд
    vsg::vec4 hud_controlled_train = {0.0f, 1.0f, 0.0f, 1.0f};    ///< Управляемый поезд
    vsg::vec4 hud_warning_text = {1.0f, 0.0f, 0.0f, 1.0f};        ///< Текст предупреждений

    // Цвета строк графика (HUD), RGBA в диапазоне 0.0 - 1.0
    vsg::vec4 hud_timetable_delay = {1.0f, 0.5f, 0.31f, 1.0f};    ///< Пройдено с опозданием
    vsg::vec4 hud_timetable_past = {0.0f, 0.5f, 0.0f, 1.0f};      ///< Пройдено по расписанию
    vsg::vec4 hud_timetable_current = {1.0f, 1.0f, 0.0f, 1.0f};   ///< Текущая
    vsg::vec4 hud_timetable_future = {0.5f, 0.5f, 0.5f, 1.0f};    ///< Будущие

    // Цвета виджета профиля пути (TrainProfileHintWidget), RGBA в диапазоне 0.0 - 1.0
    vsg::vec4 hud_train_profile_grid = {0.353f, 0.353f, 0.353f, 0.588f};               ///< Линии координатной сетки
    vsg::vec4 hud_train_profile_grid_label = {0.745f, 0.745f, 0.745f, 0.863f};         ///< Подписи километража сетки
    vsg::vec4 hud_train_profile_baseline = {0.502f, 0.502f, 0.502f, 1.0f};             ///< Базовая линия профиля (rel=0)
    vsg::vec4 hud_train_profile_curve = {0.0f, 0.4f, 0.8f, 1.0f};                       ///< Кривая профиля пути
    vsg::vec4 hud_train_profile_uncontrolled = {0.251f, 0.502f, 0.0f, 1.0f};           ///< Неуправляемый поезд
    vsg::vec4 hud_train_profile_current = {0.753f, 0.753f, 0.0f, 1.0f};                ///< Текущий поезд
    vsg::vec4 hud_train_profile_controlled = {0.753f, 0.251f, 0.251f, 1.0f};           ///< Управляемый поезд
    vsg::vec4 hud_train_profile_station_text = {0.0f, 0.784f, 1.0f, 1.0f};             ///< Названия станций
    vsg::vec4 hud_train_profile_mast = {0.863f, 0.863f, 0.863f, 1.0f};                  ///< Мачта светофора
    vsg::vec4 hud_train_profile_signal_letter = {1.0f, 1.0f, 1.0f, 1.0f};               ///< Литеры светофора
    vsg::vec4 hud_train_profile_speed_limit_border = {0.353f, 0.353f, 0.353f, 0.588f}; ///< Рамка зоны ограничения скорости
    vsg::vec4 hud_train_profile_speed_limit_fill = {0.353f, 0.353f, 0.353f, 0.157f};    ///< Заливка зоны ограничения скорости
    vsg::vec4 hud_train_profile_speed_limit_text = {1.0f, 0.0f, 0.0f, 1.0f};           ///< Текст ограничения скорости
    vsg::vec4 hud_train_profile_speed_limit_bg = {1.0f, 1.0f, 1.0f, 1.0f};              ///< Подложка текста ограничения скорости
};

#endif // VIEWER_SETTINGS_H
