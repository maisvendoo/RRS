//------------------------------------------------------------------------------
//
//      Graphics settings core (масштабируемая графика, ТЗ "Графика")
//
//      Одна сцена и одни ресурсы; качество масштабируется пресетами
//      Legacy / Low / High / Ultra / Extreme (+ Custom). Ядро хранит
//      пресеты, определяет возможности GPU, выбирает пресет
//      автоматически, управляет LOD-тирами объектов
//      (0-50/50-200/200-500/500+) и динамическим качеством при
//      падении FPS. Рендерер (VSG) берёт параметры отсюда; физика от
//      пресета не зависит.
//
//      Модуль без Qt: только стандартная библиотека C++17.
//
//------------------------------------------------------------------------------

#ifndef     GRAPHICS_SETTINGS_H
#define     GRAPHICS_SETTINGS_H

#include    <cstddef>
#include    <string>

namespace gfx
{

/// Пресет графики
enum class Preset
{
    Legacy = 0,     ///< Очень старые GPU: diffuse, без PBR/теней/постэффектов
    Low = 1,        ///< Базовый PBR, простые тени
    High = 2,       ///< Полный PBR, CSM, GTAO, TAA, SSR, HDR
    Ultra = 3,      ///< Максимум: volumetrics, probes, upscaling
    Extreme = 4,    ///< UE-подобный тир: всё от Ultra + пост-процесс (Bloom/SSAO/туман)
    Custom = 5      ///< Ручные настройки
};

/// Имя пресета для конфигов/настроек ("Legacy", "Low", ...)
std::string presetToString(Preset preset);

/// Пресет по имени; при неизвестном имени возвращается default_preset
Preset presetFromString(const std::string& name, Preset default_preset = Preset::Low);

/// Возможности GPU (заполняются рендерером при старте, ТЗ)
struct GpuCapabilities
{
    bool supports_compute = false;
    bool supports_instancing = false;
    bool supports_shadow_maps = false;
    bool supports_hdr = false;
    bool supports_msaa = false;
    bool supports_geometry_shader = false;
    bool supports_upscaling = false;    ///< DLSS/FSR/XeSS
    int max_texture_size = 4096;
    std::size_t vram_mb = 1024;
};

/// Параметры качества (потребитель — рендерер)
struct QualityParams
{
    bool pbr = true;
    bool shadows = true;
    int shadow_cascades = 1;        ///< 1 - простая карта, 4 - CSM
    int shadow_resolution = 1024;
    int msaa_samples = 1;           ///< Сглаживание: 1/2/4/8 сэмплов
    bool ssao = false;
    bool ssr = false;
    bool volumetric_fog = false;
    bool taa = false;
    bool hdr = false;
    bool decals = false;
    bool weather_effects = false;
    bool wet_surfaces = false;
    int texture_budget_mb = 512;
    double draw_distance_m = 1000.0;
    double lod_bias = 1.0;          ///< >1 - LOD дальше (агрессивнее)
    double vegetation_density = 1.0;
    double internal_resolution = 1.0;///< Для динамического качества

    //------------------------------------------------------------------
    // Новый тиры качества High/Ultra (ТЗ "Графика", дух TSW/UE5).
    //
    // Все поля ниже по умолчанию ВЫКЛЮЧЕНЫ: пресеты Legacy/Low и
    // Custom-по-умолчанию идут по прежнему пути рендера без единого
    // изменения поведения. Рендерер включает новые ветки только при
    // явной установке флагов пресетами High/Ultra.
    //------------------------------------------------------------------

    /// High/Ultra: новый PBR-тир. Рендерер строит шейдер-сеты от
    /// vsg::createPhysicsBasedRenderingShaderSet() и включает HDR-цепочку
    /// (см. use_aces_tonemap/sun_intensity). glTF-модели получают
    /// metallic/roughness-потоки автоматически: загрузчик vsgXchange::gltf
    /// берёт шейдерсет из options->shaderSets["pbr"] (зарегистрирован
    /// рендерером) и пишет PBR-дескрипторы при его наличии.
    bool use_pbr = false;

    /// High/Ultra: ACES-тонмаппинг (аппроксимация Narkowicz, см.
    /// aces_tonemap_shader_fragment()) в конце фрагментных шейдеров —
    /// сжимает HDR-диапазон в LDR. Встроенного тонмаппера в VSG нет,
    /// поэтому рендерер инжектит GLSL-обёртку в исходники шейдеров.
    /// Используется вместе с use_pbr (единый HDR-тир).
    bool use_aces_tonemap = false;

    /// High/Ultra: мягкие тени vsg::SoftShadows (PCF-подобное смягчение
    /// границ) вместо vsg::HardShadows.
    bool soft_shadows = false;

    /// High/Ultra: HD-варианты текстур неба — файлы "<имя>_hd.<ext>".
    /// Если HD-файла нет, скайбокс молча использует обычную текстуру.
    bool skybox_hd = false;

    /// SSAO (только Ultra).
    ///
    /// ЧЕСТНОЕ ЗАМЕЧАНИЕ: готового SSAO-пасса в VSG 1.1.x НЕТ — в
    /// заголовках C:\rrs-deps (vsg/utils, vsg/app, vsg/state) не
    /// найдено ни одного SSAO/AmbientOcclusion-узла. Поэтому флаг
    /// пока только декларирует намерение: рендер его не потребляет,
    /// чекбокс в GUI помечен «в разработке». Полноценный пасс
    /// (G-buffer + вычисления) отложен до появления готовых узлов
    /// в VSG или собственной реализации.
    bool use_ssao = false;

    //------------------------------------------------------------------
    // Пост-процессинговый тир Extreme (UE-подобный, ТЗ "Графика").
    //
    // Реальный пост-процесс на полноэкранных проходах чистого VSG
    // (graphics::PostProcessChain в модуле graphics): сцена рендерится
    // в offscreen HDR-буфер, затем Bloom/SSAO/объёмный туман и
    // финальная композиция в swapchain до отрисовки GUI. Все поля
    // по умолчанию ВЫКЛЮЧЕНЫ: Legacy/Low/High/Ultra/Custom идут по
    // прежнему пути рендера байт-в-байт.
    //------------------------------------------------------------------

    /// Extreme: включить пост-процессинговую цепочку. Рендерер в ветке
    /// use_postprocess строит альтернативный командный граф:
    /// offscreen-сцена -> проходы эффектов -> финальный квад + GUI.
    /// Требует перезапуска при смене (шейдеры/буферы собираются при
    /// старте).
    bool use_postprocess = false;

    /// Extreme: Bloom (bright-pass + разделяемый гауссов блюр,
    /// аддитивная композиция). Настраивается чекбоксом в GUI.
    bool use_bloom = false;

    /// Extreme: SSAO-проход (half-res, реконструкция позиций/нормалей
    /// из depth, спиральное ядро 16 сэмплов, билатеральный блюр,
    /// multiply-композиция). В отличие от декларативного use_ssao
    /// (Ultra) — реально влияющий на картинку пасс.
    bool use_ssao_pass = false;

    /// Extreme: объёмный туман (quarter-res raymarch, анизотропное
    /// рассеяние к солнцу, аддитивная композиция).
    bool use_volumetric_fog = false;

    /// Extreme: SSR — Screen Space Reflections (quarter-res raymarch
    /// 16 шагов в screen-space, толщина слоя 0.3 м, бинарный поиск
    /// уточнения 4 итерации, roughness-based билатеральный блюр,
    /// аддитивная композиция в финальном кваде; лучи за пределами
    /// экрана гаснут edge-fade'ом — поверхность остаётся на своём
    /// цвете). В отличие от декларативного ssr (High/Ultra) — реально
    /// влияющий на картинку пасс PostProcessChain. Настраивается
    /// чекбоксом в GUI, сохраняется в settings.xml (ключ Ssr).
    bool use_ssr = false;

    /// >0 — интенсивность солнца пресета (HDR-диапазон ~10-12 под
    /// ACES-тонмаппинг; ACES сожмёт яркие значения в LDR). <=0 —
    /// используется пользовательское значение из настроек.
    double sun_intensity = 0.0;

    /// >0 — дистанция отрисовки теней пресета, м (High ~150,
    /// Ultra ~300). <=0 — пользовательское значение из настроек.
    double shadow_distance_m = 0.0;
};

/// GLSL-фрагмент ACES-тонмаппинга для инжекта в конец исходника
/// фрагментного шейдера (пресеты High/Ultra).
///
/// Публичная аппроксимация ACES filmic (Narkowicz 2015):
///     x * (2.51*x + 0.03) / (x * (2.43*x + 0.59) + 0.14)
///
/// Возвращаемая строка самодостаточна (не требует внешних объявлений,
/// совместима с #version 420/450 core, как шейдеры проекта в
/// data/shaders) и содержит:
///  - функцию rrs_aces_tonemap(vec3) -> vec3;
///  - новый void main(), вызывающий исходный main шейдера (рендерер
///    переименовывает его в rrs_tonemap_original_main) и применяющий
///    тонмаппинг к outColor.rgb (альфа не меняется).
std::string aces_tonemap_shader_fragment();

//------------------------------------------------------------------------------
/// Ядро графических настроек
//------------------------------------------------------------------------------
class GraphicsSettings
{
public:

    GraphicsSettings() = default;

    /// Установить пресет (Custom - ручные параметры,
    /// задаются через setParam и фиксируются applyPreset(Custom))
    void setPreset(Preset preset);

    /// Авто-выбор пресета по возможностям GPU (ТЗ)
    Preset autoDetect(const GpuCapabilities& caps);

    Preset getPreset() const;
    const QualityParams& getParams() const;

    /// База пресета (без динамических изменений) — для восстановления
    const QualityParams& getBaseParams() const;

    /// Применить пресет. Для Custom фиксирует текущие параметры
    /// (заданные setParam) как рабочую базу
    void applyPreset(Preset preset);

    /// Установка числового параметра по имени поля QualityParams
    /// (draw_distance_m, lod_bias, vegetation_density, internal_resolution)
    bool setParam(const std::string& name, double value);

    /// Установка целочисленного/флагового параметра по имени поля
    /// QualityParams (shadow_cascades, shadow_resolution, msaa_samples,
    /// texture_budget_mb, pbr, shadows, ssao, ...)
    bool setParam(const std::string& name, int value);

    /// Целевое время кадра для динамического качества, мс
    /// (рендерер знает vsync/лимит FPS и уточняет цель)
    void setTargetFrameMs(double frame_ms);

    /// LOD-тир объекта по дистанции (ТЗ "Графика"): 0 - high, 1 - med,
    /// 2 - low, 3 - billboard/impostor
    int lodTier(double distance_m) const;

    /// Динамическое качество (ТЗ): при падении FPS снижает разрешение/
    /// дальность/эффекты плавно; recovery_rate возвращает
    void adaptFrameTime(double frame_ms);

    /// Сброс динамических изменений к пресету
    void resetDynamic();

private:

    void applyPreset();

    Preset preset_ = Preset::Low;
    QualityParams params_;

    /// База пресета: к ней возвращается качество после динамических
    /// снижений (для Custom — зафиксированные ручные параметры)
    QualityParams base_params_;

    /// Динамическое качество: целевое время кадра, мс
    double target_frame_ms_ = 16.6;

    /// Текущие динамические множители
    double dynamic_resolution_ = 1.0;
    double dynamic_distance_ = 1.0;

    /// Счётчик кадров подряд с устойчивым запасом — только после него
    /// восстанавливаем качество (гистерезис против "пилы")
    int recovery_stable_frames_ = 0;
};

} // namespace gfx

#endif // GRAPHICS_SETTINGS_H
