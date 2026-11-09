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
    else if (name == "sun_intensity")
    {
        // HDR-интенсивность солнца пресета; <=0 - использовать
        // пользовательское значение из настроек
        params_.sun_intensity = std::max(0.0, value);
    }
    else if (name == "shadow_distance_m")
    {
        // Дистанция теней пресета; <=0 - пользовательское значение
        params_.shadow_distance_m = std::max(0.0, value);
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
    else if (name == "use_pbr")
    {
        params_.use_pbr = (value != 0);
    }
    else if (name == "use_aces_tonemap")
    {
        params_.use_aces_tonemap = (value != 0);
    }
    else if (name == "use_ssao")
    {
        params_.use_ssao = (value != 0);
    }
    else if (name == "soft_shadows")
    {
        params_.soft_shadows = (value != 0);
    }
    else if (name == "skybox_hd")
    {
        params_.skybox_hd = (value != 0);
    }
    else if (name == "use_postprocess")
    {
        params_.use_postprocess = (value != 0);
    }
    else if (name == "use_bloom")
    {
        params_.use_bloom = (value != 0);
    }
    else if (name == "use_ssao_pass")
    {
        params_.use_ssao_pass = (value != 0);
    }
    else if (name == "use_volumetric_fog")
    {
        params_.use_volumetric_fog = (value != 0);
    }
    else if (name == "use_ssr")
    {
        params_.use_ssr = (value != 0);
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
        // Новый тиры (ТЗ "Графика", TSW/UE5): PBR + ACES + HDR-солнце,
        // мягкие тени, HD-небо. Тени: 2 каскада 2048, дистанция 150 м.
        params_.use_pbr = true;
        params_.use_aces_tonemap = true;
        params_.soft_shadows = true;
        params_.skybox_hd = true;
        params_.sun_intensity = 10.0;
        params_.shadow_distance_m = 150.0;
        params_.shadow_cascades = 2;
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
        // Максимум: PBR + ACES + HDR-солнце 12, 4 каскада 4096,
        // дистанция теней 300 м, HD-небо + декларативный SSAO-флаг
        // (сам пасс в разработке - см. комментарий к use_ssao)
        params_.use_pbr = true;
        params_.use_aces_tonemap = true;
        params_.use_ssao = true;
        params_.soft_shadows = true;
        params_.skybox_hd = true;
        params_.sun_intensity = 12.0;
        params_.shadow_distance_m = 300.0;
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

    case Preset::Extreme:
        params_ = QualityParams();
        // UE-подобный тир: всё от Ultra + РЕАЛЬНЫЙ пост-процесс
        // (graphics::PostProcessChain в рендере): offscreen-сцена в HDR,
        // Bloom/SSAO/SSR/объёмный туман полноэкранными проходами
        // half/quarter res, финальная композиция в swapchain до GUI.
        // Тени: 4 каскада 4096, дистанция 300 м, HD-небо, HDR-солнце 12
        params_.use_pbr = true;
        params_.use_aces_tonemap = true;
        params_.use_ssao = true;
        params_.use_postprocess = true;
        params_.use_bloom = true;
        params_.use_ssao_pass = true;
        params_.use_volumetric_fog = true;
        params_.use_ssr = true;
        params_.soft_shadows = true;
        params_.skybox_hd = true;
        params_.sun_intensity = 12.0;
        params_.shadow_distance_m = 300.0;
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
        params_.texture_budget_mb = 2048;
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
    case Preset::Extreme:   return "Extreme";
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
    if (name == "Extreme")      return Preset::Extreme;
    if (name == "Custom")       return Preset::Custom;

    return default_preset;
}

//------------------------------------------------------------------------------
// GLSL-фрагмент ACES-тонмаппинга (пресеты High/Ultra). Строка
// дописывается в конец GLSL-исходника фрагментного шейдера после того,
// как рендерер переименовал исходный void main() в
// rrs_tonemap_original_main() (см. wrap_fragment_shader_with_tonemap
// в graphics/shader_funcs). Формула - публичная аппроксимация ACES
// filmic curve (Narkowicz 2015):
//     x * (2.51*x + 0.03) / (x * (2.43*x + 0.59) + 0.14)
// Только базовые операции vec3/float - совместимо с 420/450 core,
// как шейдеры проекта в data/shaders.
//------------------------------------------------------------------------------
std::string aces_tonemap_shader_fragment()
{
    return
        "// ------------------------------------------------------------------\n"
        "// RRS: ACES filmic tonemapping, аппроксимация Narkowicz 2015.\n"
        "// Инжектится в конец шейдера пресетами High/Ultra; исходный main\n"
        "// переименован в rrs_tonemap_original_main().\n"
        "// ------------------------------------------------------------------\n"
        "vec3 rrs_aces_tonemap(vec3 x)\n"
        "{\n"
        "    return clamp((x * (2.51 * x + 0.03)) /\n"
        "                 (x * (2.43 * x + 0.59) + 0.14),\n"
        "                 vec3(0.0), vec3(1.0));\n"
        "}\n"
        "\n"
        "void main()\n"
        "{\n"
        "    rrs_tonemap_original_main();\n"
        "    outColor = vec4(rrs_aces_tonemap(outColor.rgb), outColor.a);\n"
        "}\n";
}

} // namespace gfx
