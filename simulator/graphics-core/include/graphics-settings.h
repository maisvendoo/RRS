//------------------------------------------------------------------------------
//
//      Graphics settings core (масштабируемая графика, ТЗ "Графика")
//
//      Одна сцена и одни ресурсы; качество масштабируется пресетами
//      Legacy / Low / High / Ultra (+ Custom). Ядро хранит пресеты,
//      определяет возможности GPU, выбирает пресет автоматически,
//      управляет LOD-тирами объектов (0-50/50-200/200-500/500+) и
//      динамическим качеством при падении FPS. Рендерер (VSG) берёт
//      параметры отсюда; физика от пресета не зависит.
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
    Custom = 4      ///< Ручные настройки
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
};

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
