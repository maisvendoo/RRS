//------------------------------------------------------------------------------
//
//      Graphics settings core
//
//------------------------------------------------------------------------------

#include    "graphics-settings.h"

#include    <algorithm>
#include <cmath>

namespace gfx
{

/// Минимальный LOD-bias: при меньших (и отрицательных) значениях
/// LOD-тиры схлопываются в один
constexpr double MIN_LOD_BIAS = 0.1;

/// Кадров подряд с запасом времени, после которого восстанавливаем качество
constexpr int RECOVERY_STABLE_THRESHOLD = 30;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void GraphicsSettings::setPreset(Preset preset)
{
    preset_ = preset;

    if (preset != Preset::Custom)
    {
        applyPreset();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Preset GraphicsSettings::autoDetect(const GpuCapabilities& caps)
{
    // Ultra: всё современное + много VRAM + апскейлинг
    if (caps.supports_compute && caps.supports_hdr && caps.supports_msaa &&
        caps.supports_upscaling && caps.vram_mb >= 6144 &&
        caps.max_texture_size >= 8192)
    {
        setPreset(Preset::Ultra);
    }
    // High: PBR-возможности, тени, средняя VRAM
    else if (caps.supports_compute && caps.supports_shadow_maps &&
             caps.vram_mb >= 2048)
    {
        setPreset(Preset::High);
    }
    // Low: базовые возможности
    else if (caps.supports_instancing && caps.vram_mb >= 1024)
    {
        setPreset(Preset::Low);
    }
    // Legacy: очень старые GPU
    else
    {
        setPreset(Preset::Legacy);
    }

    return preset_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Preset GraphicsSettings::getPreset() const
{
    return preset_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const QualityParams& GraphicsSettings::getParams() const
{
    return params_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const QualityParams& GraphicsSettings::getBaseParams() const
{
    return base_params_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void GraphicsSettings::applyPreset(Preset preset)
{
    preset_ = preset;

    // Полное применение, включая Custom: ручные параметры фиксируются
    // как рабочая база, к которой вернётся динамическое качество
    applyPreset();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool GraphicsSettings::setParam(const std::string& name, double value)
{
    if (name == "draw_distance_m")
    {
        params_.draw_distance_m = std::max(0.0, value);
    }
    else if (name == "lod_bias")
    {
        // Не даём тирам схлопнуться
        params_.lod_bias = std::max(value, MIN_LOD_BIAS);
    }
    else if (name == "vegetation_density")
    {
        params_.vegetation_density = std::max(0.0, value);
    }
    else if (name == "internal_resolution")
    {
        params_.internal_resolution = std::clamp(value, 0.25, 2.0);
    }
    else
    {
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool GraphicsSettings::setParam(const std::string& name, int value)
{
    if (name == "shadow_cascades")
    {
        params_.shadow_cascades = std::clamp(value, 1, 4);
    }
    else if (name == "shadow_resolution")
    {
        params_.shadow_resolution = std::clamp(value, 256, 16384);
    }
    else if (name == "msaa_samples")
    {
        // Только степени двойки, поддерживаемые Vulkan
        switch (value)
        {
        case 1:
        case 2:
        case 4:
        case 8:
            params_.msaa_samples = value;
            break;
        default:
            return false;
        }
    }
    else if (name == "texture_budget_mb")
    {
        params_.texture_budget_mb = std::max(1, value);
    }
    else if (name == "pbr")
    {
        params_.pbr = (value != 0);
    }
    else if (name == "shadows")
    {
        params_.shadows = (value != 0);
    }
    else if (name == "ssao")
    {
        params_.ssao = (value != 0);
    }
    else if (name == "ssr")
    {
        params_.ssr = (value != 0);
    }
    else if (name == "volumetric_fog")
    {
        params_.volumetric_fog = (value != 0);
    }
    else if (name == "taa")
    {
        params_.taa = (value != 0);
    }
    else if (name == "hdr")
    {
        params_.hdr = (value != 0);
    }
    else if (name == "decals")
    {
        params_.decals = (value != 0);
    }
    else if (name == "weather_effects")
    {
        params_.weather_effects = (value != 0);
    }
    else if (name == "wet_surfaces")
    {
        params_.wet_surfaces = (value != 0);
    }
    else
    {
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void GraphicsSettings::setTargetFrameMs(double frame_ms)
{
    if (frame_ms > 0.0)
    {
        target_frame_ms_ = frame_ms;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int GraphicsSettings::lodTier(double distance_m) const
{
    // Bias < 1 сжимает пороги (агрессивнее LOD у Legacy);
    // снизу клампим, чтобы тиры не схлопнулись в один
    const double b = std::max(params_.lod_bias, MIN_LOD_BIAS);

    if (distance_m < 50.0 * b)
        return 0;   // Высокая геометрия, 2K/4K, PBR, отражения
    if (distance_m < 200.0 * b)
        return 1;   // Средняя, 1K/2K, PBR, упрощённые эффекты
    if (distance_m < 500.0 * b)
        return 2;   // Низкая, упрощённые материалы

    return 3;       // Billboard/impostor
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void GraphicsSettings::adaptFrameTime(double frame_ms)
{
    if (frame_ms <= 0.0)
        return;

    // Перегрузка: снижаем разрешение и дальность быстро (ТЗ)
    if (frame_ms > target_frame_ms_ * 1.3)
    {
        dynamic_resolution_ = std::max(0.6, dynamic_resolution_ - 0.02);
        dynamic_distance_ = std::max(0.5, dynamic_distance_ - 0.02);

        // Намёка на запас нет — счётчик устойчивости сбрасываем
        recovery_stable_frames_ = 0;
    }
    // Запас: восстанавливаем медленно. Порог 0.95 достижим и при vsync
    // (кадр не быстрее 0.8 целевого там не бывает никогда), зона между
    // 0.95 и 1.3 — гистерезис: в ней качество не меняем
    else if (frame_ms < target_frame_ms_ * 0.95)
    {
        // Требуем устойчивый запас несколько кадров подряд,
        // иначе качество "пилит" туда-сюда на границе порога
        if (++recovery_stable_frames_ >= RECOVERY_STABLE_THRESHOLD)
        {
            dynamic_resolution_ = std::min(1.0, dynamic_resolution_ + 0.005);
            dynamic_distance_ = std::min(1.0, dynamic_distance_ + 0.005);
        }
    }
    else
    {
        // Зона гистерезиса — держим текущее качество
        recovery_stable_frames_ = 0;
    }

    // Динамические множители применяем к базе пресета,
    // чтобы восстановление возвращало исходное качество
    params_.internal_resolution = dynamic_resolution_;
    params_.draw_distance_m = base_params_.draw_distance_m * dynamic_distance_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void GraphicsSettings::resetDynamic()
{
    dynamic_resolution_ = 1.0;
    dynamic_distance_ = 1.0;
    recovery_stable_frames_ = 0;

    // Полный сброс к базе пресета (для Custom — к зафиксированной
    // базе ручных параметров), включая internal_resolution
    params_ = base_params_;
    params_.internal_resolution = 1.0;
    params_.draw_distance_m = base_params_.draw_distance_m;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void GraphicsSettings::applyPreset()
{
    switch (preset_)
    {
    case Preset::Legacy:
        params_ = QualityParams();
        params_.pbr = false;
        params_.shadows = false;
        params_.msaa_samples = 1;
        params_.shadow_cascades = 1;
        params_.shadow_resolution = 1024;
        params_.texture_budget_mb = 128;
        params_.draw_distance_m = 1500.0;
        params_.lod_bias = 0.5;          // Агрессивный LOD
        params_.vegetation_density = 0.2;
        break;

    case Preset::Low:
        params_ = QualityParams();
        params_.pbr = true;
        params_.shadows = true;
        params_.shadow_cascades = 2;
        params_.shadow_resolution = 1024;
        params_.msaa_samples = 4;
        params_.texture_budget_mb = 256;
        params_.draw_distance_m = 3000.0;
        params_.lod_bias = 0.8;
        params_.vegetation_density = 0.5;
        break;

    case Preset::High:
        params_ = QualityParams();
        params_.shadow_cascades = 3;
        params_.shadow_resolution = 2048;
        params_.msaa_samples = 4;
        params_.ssao = true;
        params_.ssr = true;
        params_.taa = true;
        params_.hdr = true;
        params_.decals = true;
        params_.weather_effects = true;
        params_.wet_surfaces = true;
        params_.texture_budget_mb = 768;
        params_.draw_distance_m = 5000.0;
        params_.vegetation_density = 1.0;
        break;

    case Preset::Ultra:
        params_ = QualityParams();
        params_.shadow_cascades = 4;
        params_.shadow_resolution = 4096;
        params_.msaa_samples = 8;
        params_.ssao = true;
        params_.ssr = true;
        params_.volumetric_fog = true;
        params_.taa = true;
        params_.hdr = true;
        params_.decals = true;
        params_.weather_effects = true;
        params_.wet_surfaces = true;
        params_.texture_budget_mb = 1536;
        params_.draw_distance_m = 8000.0;
        params_.vegetation_density = 1.5;
        break;

    case Preset::Custom:
        // Ручные параметры не трогаем — фиксируются как база ниже
        break;
    }

    // LOD-bias клампим: при <= 0 тиры LOD схлопываются
    params_.lod_bias = std::max(params_.lod_bias, MIN_LOD_BIAS);

    // Фиксируем базу: к ней возвращается динамическое качество
    base_params_ = params_;

    // Поверх базы — текущие динамические множители
    params_.internal_resolution = dynamic_resolution_;
    params_.draw_distance_m = base_params_.draw_distance_m * dynamic_distance_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string presetToString(Preset preset)
{
    switch (preset)
    {
    case Preset::Legacy:    return "Legacy";
    case Preset::Low:       return "Low";
    case Preset::High:      return "High";
    case Preset::Ultra:     return "Ultra";
    case Preset::Custom:    return "Custom";
    }

    return "Low";
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Preset presetFromString(const std::string& name, Preset default_preset)
{
    if (name == "Legacy")       return Preset::Legacy;
    if (name == "Low")          return Preset::Low;
    if (name == "High")         return Preset::High;
    if (name == "Ultra")        return Preset::Ultra;
    if (name == "Custom")       return Preset::Custom;

    return default_preset;
}

} // namespace gfx
