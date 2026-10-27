#ifndef GRAPHICS_POSTPROCESS_H
#define GRAPHICS_POSTPROCESS_H

#include <vsg/core/ref_ptr.h>

#include <vulkan/vulkan_core.h>

#include <cstddef>
#include <vector>

namespace vsg
{

class Camera;
class DirectionalLight;
class Device;
class ImageView;
class Node;
class RenderGraph;
class View;
class Window;

}

//------------------------------------------------------------------------------
// Пост-процессинговый тир Extreme (ТЗ "Графика", UE-подобный).
//
// Чистый VSG 1.1.x (все вызовы сверены с заголовками C:\rrs-deps):
// готовых пост-процесс-узлов в VSG нет, поэтому цепочка собирается из
// базовых примитивов: offscreen RenderGraph с собственными
// vsg::Image/vsg::ImageView/vsg::Framebuffer/vsg::RenderPass и
// полноэкранные проходы на собственном vsg::GraphicsPipeline
// (big-triangle из gl_VertexIndex, вершинный буфер не нужен).
//
// Схема командного графа (ветка use_postprocess в RouteViewer):
//
//   CommandGraph(окно)
//   |- RenderGraph #1: сцена -> offscreen HDR-цвет + depth (createSceneRenderGraph)
//   |- RenderGraph #2..N: проходы эффектов (createEffectPasses)
//   |    Bloom: bright-pass (half-res) -> blur H -> blur V
//   |    SSAO:  ядро 16 сэмплов (half-res) -> билатеральный блюр
//   |    Туман: raymarch 24 шага (quarter-res)
//   |    SSR:   raymarch 16 шагов + бинарный поиск (quarter-res)
//   |          -> roughness-билатеральный блюр (quarter-res)
//   |- RenderGraph(окно): финальный квад (createFinalQuad) + RenderImGui
//
// Композиция (multiply AO / additive bloom / additive fog / additive
// SSR) свёрнута в финальный квад — полноэкранных full-res
// промежуточных буферов нет, все эффекты считаются на half/quarter-res
// (ТЗ: производительность).
//
// Динамические данные (inverse projection камеры, направление на
// солнце в пространстве вида) передаются push-константами из
// собственного StateCommand'а, читающего vsg::Camera/vsg::DirectionalLight
// В МОМЕНТ записи кадра — отдельный механизм обновления буферов не нужен.
//------------------------------------------------------------------------------
namespace graphics
{

/// Конфигурация цепочки (заполняется рендерером из QualityParams/настроек)
struct PostProcessConfig
{
    bool bloom = true;              ///< Bloom-проход
    bool ssao = true;               ///< SSAO-проход
    bool volumetric_fog = true;     ///< Объёмный туман
    bool ssr = true;                ///< SSR (Screen Space Reflections)

    double scale = 0.75;            ///< Масштаб offscreen-буфера сцены (0.4-1.0)

    double bloom_threshold = 1.0;   ///< Порог яркости bright-pass
    double bloom_intensity = 0.3;   ///< Интенсивность аддитивного блума

    double ssao_radius_m = 0.5;     ///< Радиус ядра AO, м
    int ssao_samples = 16;          ///< Сэмплов спирального ядра
    double ssao_bias = 0.025;       ///< Смещение от нормали (против self-occlusion)
    double ssao_strength = 1.0;     ///< Сила затенения

    double fog_distance_m = 200.0;  ///< Дистанция raymarch, м
    int fog_steps = 24;             ///< Шагов raymarch
    double fog_density = 0.012;     ///< Плотность тумана, 1/м
    double fog_scattering = 0.5;    ///< Сила рассеяния к солнцу

    double ssr_thickness_m = 0.3;   ///< Толщина слоя глубины (reject тонких
                                    ///< объектов/сколов глубины), м
    int ssr_steps = 16;             ///< Шагов raymarch в screen-space
    int ssr_refine_steps = 4;       ///< Итераций бинарного поиска уточнения
    double ssr_max_distance_m = 60.0; ///< Дистанция луча, м
    double ssr_intensity = 0.8;     ///< Сила отражений в композиции
};

/// Цепочка пост-обработки пресета Extreme
class PostProcessChain
{
public:
    using RenderGraphs = std::vector<vsg::ref_ptr<vsg::RenderGraph>>;

    explicit PostProcessChain(const PostProcessConfig& config);

    /// Деструктор и копирование — только в .cpp (vsg-типы в членах
    /// объявлены forward-declare)
    ~PostProcessChain();
    PostProcessChain(const PostProcessChain&) = delete;
    PostProcessChain& operator=(const PostProcessChain&) = delete;

    /// Проход сцены в offscreen: цвет HDR VK_FORMAT_R16G16B16A16_SFLOAT
    /// + глубина VK_FORMAT_D32_SFLOAT (обе текстуры сэмплится дальше).
    /// Подгоняет viewportState камеры вида под extent offscreen-буфера.
    vsg::ref_ptr<vsg::RenderGraph> createSceneRenderGraph(
        vsg::View* view,
        vsg::Device* device,
        const VkExtent2D& extent);

    /// Полноэкранные проходы эффектов (bloom/SSAO/туман). camera и light —
    /// источник динамических матриц/направления солнца на каждый кадр
    /// (могут быть nullptr — проходы с матрицами не создаются).
    void createEffectPasses(
        vsg::Device* device,
        vsg::Camera* camera,
        vsg::DirectionalLight* light,
        const VkExtent2D& scene_extent);

    /// Финальный квад: композиция сцены и эффектов -> текущий
    /// framebuffer окна. Встраивается в ГЛАВНЫЙ RenderGraph окна
    /// (созданный по окну) ПЕРЕД RenderImGui.
    vsg::ref_ptr<vsg::Node> createFinalQuad(vsg::Window* window);

    /// Промежуточные проходы эффектов — добавить в CommandGraph после
    /// прохода сцены (возвращённом createSceneRenderGraph)
    const RenderGraphs& effectPasses() const { return effect_passes_; }

    /// Статусы проходов (для диагностики/GUI)
    bool hasBloom() const { return bloom_built_; }
    bool hasSsao() const { return ssao_built_; }
    bool hasFog() const { return fog_built_; }
    bool hasSsr() const { return ssr_built_; }

private:
    class Impl;
    Impl* impl_ = nullptr;          ///< Pimpl: vsg-объекты не тащим в заголовок

    PostProcessConfig config_;
    RenderGraphs effect_passes_;
    bool bloom_built_ = false;
    bool ssao_built_ = false;
    bool fog_built_ = false;
    bool ssr_built_ = false;
};

} // namespace graphics

#endif // GRAPHICS_POSTPROCESS_H
