#include "graphics/postprocess.h"

#include "Journal.h"

#include <vsg/app/Camera.h>
#include <vsg/app/ProjectionMatrix.h>
#include <vsg/app/RenderGraph.h>
#include <vsg/app/View.h>
#include <vsg/app/ViewMatrix.h>
#include <vsg/app/Window.h>
#include <vsg/commands/Command.h>
#include <vsg/commands/Draw.h>
#include <vsg/core/Value.h>
#include <vsg/lighting/DirectionalLight.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/vec4.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/Descriptor.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/DescriptorSet.h>
#include <vsg/state/DescriptorSetLayout.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/Image.h>
#include <vsg/state/ImageInfo.h>
#include <vsg/state/ImageView.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/PipelineLayout.h>
#include <vsg/state/PushConstants.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/ShaderStage.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/state/ViewportState.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Framebuffer.h>
#include <vsg/vk/RenderPass.h>

#include <algorithm>
#include <functional>
#include <string>
#include <utility>

namespace
{

//------------------------------------------------------------------------------
// Форматы цепочки
//------------------------------------------------------------------------------
constexpr VkFormat HDR_COLOR_FORMAT = VK_FORMAT_R16G16B16A16_SFLOAT;
constexpr VkFormat SCENE_DEPTH_FORMAT = VK_FORMAT_D32_SFLOAT;

//------------------------------------------------------------------------------
// GLSL: общий вершинный шейдер полноэкранного прохода.
// Квад — big-triangle из gl_VertexIndex: вершинный буфер не нужен,
// вершины (0,0), (2,0), (0,2) покрывают uv-диапазон [0..2]
//------------------------------------------------------------------------------
const char* fullscreen_vertex_source()
{
    return
        "#version 450\n"
        "\n"
        "layout(location = 0) out vec2 in_uv;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    vec2 position = vec2(float((gl_VertexIndex << 1) & 2),\n"
        "                         float(gl_VertexIndex & 2));\n"
        "    in_uv = position;\n"
        "    gl_Position = vec4(position * 2.0 - 1.0, 0.0, 1.0);\n"
        "}\n";
}

//------------------------------------------------------------------------------
// GLSL: Bloom bright-pass — выделение ярких участков (порог 1.0 в HDR)
//------------------------------------------------------------------------------
std::string bloom_bright_fragment_source()
{
    return
        std::string("#version 450\n") +
        "\n"
        "layout(binding = 0) uniform sampler2D in_color;\n"
        "\n"
        "layout(push_constant) uniform Params\n"
        "{\n"
        "    vec4 params;    // x: порог яркости, y/z/w: зарезервировано\n"
        "} pc;\n"
        "\n"
        "layout(location = 0) in vec2 in_uv;\n"
        "layout(location = 0) out vec4 outColor;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    vec3 color = texture(in_color, in_uv).rgb;\n"
        "\n"
        "    float luma = max(color.r, max(color.g, color.b));\n"
        "\n"
        "    // Мягкий порог: всё ниже порога гасится к нулю\n"
        "    float response = clamp((luma - pc.params.x)\n"
        "                           / max(luma, 1.0e-4), 0.0, 1.0);\n"
        "\n"
        "    outColor = vec4(color * response, 1.0);\n"
        "}\n";
}

//------------------------------------------------------------------------------
// GLSL: разделяемый 9-tap гауссов блюр (направление — push-константой)
//------------------------------------------------------------------------------
std::string blur_fragment_source()
{
    return
        std::string("#version 450\n") +
        "\n"
        "layout(binding = 0) uniform sampler2D in_source;\n"
        "\n"
        "layout(push_constant) uniform Params\n"
        "{\n"
        "    vec4 params;    // xy: направление * размер текселя\n"
        "} pc;\n"
        "\n"
        "layout(location = 0) in vec2 in_uv;\n"
        "layout(location = 0) out vec4 outColor;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    // Linear-sampled 9-tap гауссиан (пары отводов свёрнуты)\n"
        "    vec3 sum = texture(in_source, in_uv).rgb * 0.227027;\n"
        "\n"
        "    vec2 offset_1 = pc.params.xy * 1.3846153846;\n"
        "    vec2 offset_2 = pc.params.xy * 3.2307692308;\n"
        "\n"
        "    sum += texture(in_source, in_uv + offset_1).rgb * 0.3162162162;\n"
        "    sum += texture(in_source, in_uv - offset_1).rgb * 0.3162162162;\n"
        "    sum += texture(in_source, in_uv + offset_2).rgb * 0.0702702703;\n"
        "    sum += texture(in_source, in_uv - offset_2).rgb * 0.0702702703;\n"
        "\n"
        "    outColor = vec4(sum, 1.0);\n"
        "}\n";
}

//------------------------------------------------------------------------------
// GLSL: SSAO — реконструкция позиций/нормалей из depth (inverse projection
// из камеры), спиральное ядро 16 сэмплов
//------------------------------------------------------------------------------
std::string ssao_fragment_source()
{
    return
        std::string("#version 450\n") +
        "\n"
        "layout(binding = 0) uniform sampler2D in_depth;\n"
        "\n"
        "layout(push_constant) uniform Params\n"
        "{\n"
        "    mat4 inverse_projection;\n"
        "    vec4 params;    // x: радиус, м; y: сэмплов; z: bias; w: сила\n"
        "} pc;\n"
        "\n"
        "layout(location = 0) in vec2 in_uv;\n"
        "layout(location = 0) out vec4 outColor;\n"
        "\n"
        "// Реконструкция позиции в пространстве вида по глубине.\n"
        "// Матрица берётся из камеры, поэтому соглашение глубины\n"
        "// (VSG использует reversed-Z) не имеет значения\n"
        "vec3 view_position(vec2 uv)\n"
        "{\n"
        "    float depth = texture(in_depth, uv).r;\n"
        "    vec4 clip = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);\n"
        "    vec4 view = pc.inverse_projection * clip;\n"
        "    return view.xyz / view.w;\n"
        "}\n"
        "\n"
        "float hash_1(vec2 seed)\n"
        "{\n"
        "    return fract(sin(dot(seed, vec2(12.9898, 78.233))) * 43758.5453);\n"
        "}\n"
        "\n"
        "void main()\n"
        "{\n"
        "    const float PI_2 = 6.28318530718;\n"
        "\n"
        "    vec3 position = view_position(in_uv);\n"
        "\n"
        "    // Нормаль из производных реконструированной позиции\n"
        "    vec3 normal = normalize(cross(dFdy(position), dFdx(position)));\n"
        "\n"
        "    // Тангенсный базис + случайный поворот ядра по пикселю\n"
        "    vec3 arbitrary = (abs(normal.z) < 0.999)\n"
        "                    ? vec3(0.0, 0.0, 1.0)\n"
        "                    : vec3(1.0, 0.0, 0.0);\n"
        "    vec3 tangent = normalize(cross(arbitrary, normal));\n"
        "    vec3 bitangent = cross(normal, tangent);\n"
        "\n"
        "    float rotation = hash_1(gl_FragCoord.xy) * PI_2;\n"
        "\n"
        "    // Прямая проекция = обратная к inverse projection\n"
        "    mat4 projection = inverse(pc.inverse_projection);\n"
        "\n"
        "    float occlusion = 0.0;\n"
        "    int samples = int(pc.params.y);\n"
        "\n"
        "    for (int i = 0; i < 16; ++i)\n"
        "    {\n"
        "        if (i >= samples)\n"
        "        {\n"
        "            break;\n"
        "        }\n"
        "\n"
        "        // Спиральное ядро: угол и радиус растут с индексом\n"
        "        float sample_angle = rotation + float(i) * PI_2 / 16.0;\n"
        "        float sample_scale = (float(i) + 0.5) / 16.0;\n"
        "        vec2 disk = vec2(cos(sample_angle), sin(sample_angle))\n"
        "                    * sample_scale;\n"
        "\n"
        "        vec3 offset = tangent * disk.x\n"
        "                    + bitangent * disk.y\n"
        "                    + normal * 0.1;\n"
        "        vec3 sample_pos = position + offset * pc.params.x;\n"
        "\n"
        "        // Проекция сэмпла на экран\n"
        "        vec4 projected = projection * vec4(sample_pos, 1.0);\n"
        "        vec2 sample_uv = (projected.xy / projected.w) * 0.5 + 0.5;\n"
        "\n"
        "        if ((sample_uv.x < 0.0) || (sample_uv.x > 1.0) ||\n"
        "            (sample_uv.y < 0.0) || (sample_uv.y > 1.0))\n"
        "        {\n"
        "            continue;\n"
        "        }\n"
        "\n"
        "        // Глубина сцены в точке проекции сэмпла. Вид смотрит\n"
        "        // в -Z: сэмпл глубже поверхности перекрыт ею\n"
        "        float scene_z = view_position(sample_uv).z;\n"
        "        float deeper = scene_z - sample_pos.z;\n"
        "\n"
        "        // Ограничение дистанции против «гало» от дальних\n"
        "        // перекрывателей (небо/фон)\n"
        "        if ((deeper > pc.params.z) &&\n"
        "            (deeper < pc.params.x * 4.0))\n"
        "        {\n"
        "            occlusion += 1.0;\n"
        "        }\n"
        "    }\n"
        "\n"
        "    float ao = clamp(1.0 - pc.params.w\n"
        "                     * occlusion / max(float(samples), 1.0),\n"
        "                     0.0, 1.0);\n"
        "\n"
        "    outColor = vec4(vec3(ao), 1.0);\n"
        "}\n";
}

//------------------------------------------------------------------------------
// GLSL: билатеральный блюр SSAO (AO + глубина для сохранения граней)
//------------------------------------------------------------------------------
std::string ssao_blur_fragment_source()
{
    return
        std::string("#version 450\n") +
        "\n"
        "layout(binding = 0) uniform sampler2D in_ao;\n"
        "layout(binding = 1) uniform sampler2D in_depth;\n"
        "\n"
        "layout(push_constant) uniform Params\n"
        "{\n"
        "    vec4 params;    // xy: тексель буфера, z: сигма глубины\n"
        "} pc;\n"
        "\n"
        "layout(location = 0) in vec2 in_uv;\n"
        "layout(location = 0) out vec4 outColor;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    float ao = texture(in_ao, in_uv).r;\n"
        "    float depth = texture(in_depth, in_uv).r;\n"
        "\n"
        "    float sum = ao;\n"
        "    float weight_sum = 1.0;\n"
        "\n"
        "    // Крест 5-tap: вес по дистанции + близости глубины\n"
        "    for (int i = 1; i <= 2; ++i)\n"
        "    {\n"
        "        float weight = 1.0 / (1.0 + float(i));\n"
        "\n"
        "        vec2 offset_x = vec2(pc.params.x * float(i), 0.0);\n"
        "        float ao_p = texture(in_ao, in_uv + offset_x).r;\n"
        "        float depth_p = texture(in_depth, in_uv + offset_x).r;\n"
        "        float depth_w = exp(-abs(depth_p - depth)\n"
        "                            / max(pc.params.z, 1.0e-6));\n"
        "        sum += ao_p * weight * depth_w;\n"
        "        weight_sum += weight * depth_w;\n"
        "\n"
        "        float ao_m = texture(in_ao, in_uv - offset_x).r;\n"
        "        float depth_m = texture(in_depth, in_uv - offset_x).r;\n"
        "        depth_w = exp(-abs(depth_m - depth)\n"
        "                      / max(pc.params.z, 1.0e-6));\n"
        "        sum += ao_m * weight * depth_w;\n"
        "        weight_sum += weight * depth_w;\n"
        "\n"
        "        vec2 offset_y = vec2(0.0, pc.params.y * float(i));\n"
        "        ao_p = texture(in_ao, in_uv + offset_y).r;\n"
        "        depth_p = texture(in_depth, in_uv + offset_y).r;\n"
        "        depth_w = exp(-abs(depth_p - depth)\n"
        "                      / max(pc.params.z, 1.0e-6));\n"
        "        sum += ao_p * weight * depth_w;\n"
        "        weight_sum += weight * depth_w;\n"
        "\n"
        "        ao_m = texture(in_ao, in_uv - offset_y).r;\n"
        "        depth_m = texture(in_depth, in_uv - offset_y).r;\n"
        "        depth_w = exp(-abs(depth_m - depth)\n"
        "                      / max(pc.params.z, 1.0e-6));\n"
        "        sum += ao_m * weight * depth_w;\n"
        "        weight_sum += weight * depth_w;\n"
        "    }\n"
        "\n"
        "    outColor = vec4(vec3(clamp(sum / weight_sum, 0.0, 1.0)), 1.0);\n"
        "}\n";
}

//------------------------------------------------------------------------------
// GLSL: объёмный туман — quarter-res raymarch от камеры до 200 м,
// анизотропное рассеяние к солнцу (Хеньей-Гринштейн)
//------------------------------------------------------------------------------
std::string fog_fragment_source()
{
    return
        std::string("#version 450\n") +
        "\n"
        "layout(binding = 0) uniform sampler2D in_depth;\n"
        "\n"
        "layout(push_constant) uniform Params\n"
        "{\n"
        "    mat4 inverse_projection;\n"
        "    vec4 sun;    // xyz: направление НА солнце (вид-пространство),\n"
        "                // w: сила рассеяния\n"
        "    vec4 params; // x: дистанция, м; y: шагов; z: плотность, 1/м;\n"
        "                // w: анизотропия (g)\n"
        "} pc;\n"
        "\n"
        "layout(location = 0) in vec2 in_uv;\n"
        "layout(location = 0) out vec4 outColor;\n"
        "\n"
        "vec3 view_position(vec2 uv)\n"
        "{\n"
        "    float depth = texture(in_depth, uv).r;\n"
        "    vec4 clip = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);\n"
        "    vec4 view = pc.inverse_projection * clip;\n"
        "    return view.xyz / view.w;\n"
        "}\n"
        "\n"
        "// Фаза рассеяния Хеньей-Гринштейна (анизотропия к солнцу)\n"
        "float henvey_greenstein(float cosine, float g)\n"
        "{\n"
        "    const float PI = 3.14159265359;\n"
        "    float gg = g * g;\n"
        "    return (1.0 - gg)\n"
        "           / (4.0 * PI * pow(1.0 + gg - 2.0 * g * cosine, 1.5));\n"
        "}\n"
        "\n"
        "void main()\n"
        "{\n"
        "    vec3 position = view_position(in_uv);\n"
        "\n"
        "    // Камера — начало координат пространства вида\n"
        "    float scene_distance = length(position);\n"
        "    vec3 direction = position / max(scene_distance, 1.0e-4);\n"
        "\n"
        "    // Марш не дальше сцены и предельной дистанции тумана\n"
        "    float distance = min(pc.params.x, scene_distance);\n"
        "    int steps = int(pc.params.y);\n"
        "    float step_len = distance / max(float(steps), 1.0);\n"
        "\n"
        "    vec3 sun_dir = normalize(pc.sun.xyz);\n"
        "    float phase = henvey_greenstein(dot(direction, sun_dir),\n"
        "                                    pc.params.w);\n"
        "\n"
        "    float transmittance = 1.0;\n"
        "    vec3 scatter = vec3(0.0);\n"
        "\n"
        "    for (int i = 0; i < 24; ++i)\n"
        "    {\n"
        "        if (i >= steps)\n"
        "        {\n"
        "            break;\n"
        "        }\n"
        "\n"
        "        float step_density = pc.params.z * step_len;\n"
        "        transmittance *= exp(-step_density);\n"
        "\n"
        "        // Изотропная часть 1/4pi + анизотропная фаза к солнцу\n"
        "        scatter += transmittance * step_density\n"
        "                    * (0.0795775 + phase);\n"
        "    }\n"
        "\n"
        "    outColor = vec4(scatter * pc.sun.w, transmittance);\n"
        "}\n";
}

//------------------------------------------------------------------------------
// GLSL: SSR (Screen Space Reflections) — quarter-res raymarch в
// screen-space: 16 шагов, толщина слоя 0.3 м, затухание к краям экрана.
// Позиции реконструируются из depth + inverse projection (как SSAO),
// уточнение попадания — бинарным поиском (4 итерации).
//
// Выход: RGB — цвет отражения, premultiplied на вес (edge fade у краёв
// экрана -> вклад нулевой, поверхность остаётся на своём prelit-цвете);
// A — прокси «шероховатости» для блюра (дальность попадания луча)
//------------------------------------------------------------------------------
std::string ssr_raymarch_fragment_source()
{
    return
        std::string("#version 450\n") +
        "\n"
        "layout(binding = 0) uniform sampler2D in_scene_color;\n"
        "layout(binding = 1) uniform sampler2D in_scene_depth;\n"
        "\n"
        "layout(push_constant) uniform Params\n"
        "{\n"
        "    mat4 inverse_projection;\n"
        "    vec4 params;    // x: толщина, м; y: шагов; z: дистанция луча,\n"
        "                    // м; w: итераций бинарного поиска\n"
        "} pc;\n"
        "\n"
        "layout(location = 0) in vec2 in_uv;\n"
        "layout(location = 0) out vec4 outColor;\n"
        "\n"
        "// Реконструкция позиции в пространстве вида по глубине (как SSAO:\n"
        "// матрица из камеры учитывает соглашение глубины VSG, reversed-Z)\n"
        "vec3 view_position(vec2 uv)\n"
        "{\n"
        "    float depth = texture(in_scene_depth, uv).r;\n"
        "    vec4 clip = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);\n"
        "    vec4 view = pc.inverse_projection * clip;\n"
        "    return view.xyz / view.w;\n"
        "}\n"
        "\n"
        "void main()\n"
        "{\n"
        "    vec3 position = view_position(in_uv);\n"
        "\n"
        "    // Нормаль из производных реконструированной позиции\n"
        "    vec3 normal = normalize(cross(dFdy(position), dFdx(position)));\n"
        "    vec3 view_dir = normalize(-position);\n"
        "\n"
        "    // Поверхность отвёрнута от камеры — отражения нет\n"
        "    if (dot(normal, view_dir) <= 0.0)\n"
        "    {\n"
        "        outColor = vec4(0.0);\n"
        "        return;\n"
        "    }\n"
        "\n"
        "    // Прямая проекция = обратная к inverse projection (как в SSAO)\n"
        "    mat4 projection = inverse(pc.inverse_projection);\n"
        "\n"
        "    int steps = int(pc.params.y);\n"
        "    int refine_steps = int(pc.params.w);\n"
        "    float step_len = pc.params.z / max(float(steps), 1.0);\n"
        "\n"
        "    // Луч в пространстве вида от поверхности (incident — из\n"
        "    // камеры в точку, reflect ждёт именно его)\n"
        "    vec3 reflected = normalize(reflect(normalize(position),\n"
        "                                       normal));\n"
        "\n"
        "    vec3 sample_pos = position;\n"
        "    vec3 hit_point = position;\n"
        "    vec2 hit_uv = vec2(0.0);\n"
        "    bool hit = false;\n"
        "\n"
        "    for (int i = 0; i < 16; ++i)\n"
        "    {\n"
        "        if (i >= steps)\n"
        "        {\n"
        "            break;\n"
        "        }\n"
        "\n"
        "        sample_pos += reflected * step_len;\n"
        "\n"
        "        vec4 projected = projection * vec4(sample_pos, 1.0);\n"
        "\n"
        "        // За ближней плоскостью луч ушёл в камеру\n"
        "        if (projected.w <= 0.0)\n"
        "        {\n"
        "            break;\n"
        "        }\n"
        "\n"
        "        vec2 uv = (projected.xy / projected.w) * 0.5 + 0.5;\n"
        "\n"
        "        // Вышли за экран — дальше искать бессмысленно\n"
        "        if ((uv.x < 0.0) || (uv.x > 1.0) ||\n"
        "            (uv.y < 0.0) || (uv.y > 1.0))\n"
        "        {\n"
        "            break;\n"
        "        }\n"
        "\n"
        "        // Глубина сцены в точке проекции шага. Вид смотрит в -Z:\n"
        "        // поверхность ближе шага на толщину -> пересечение\n"
        "        vec3 scene_pos = view_position(uv);\n"
        "\n"
        "        if ((scene_pos.z - sample_pos.z) > pc.params.x)\n"
        "        {\n"
        "            hit = true;\n"
        "\n"
        "            // Бинарный поиск: отрезок [до пересечения; за ним]\n"
        "            vec3 near_point = sample_pos - reflected * step_len;\n"
        "            vec3 far_point = sample_pos;\n"
        "\n"
        "            for (int j = 0; j < 4; ++j)\n"
        "            {\n"
        "                if (j >= refine_steps)\n"
        "                {\n"
        "                    break;\n"
        "                }\n"
        "\n"
        "                vec3 mid_point = mix(near_point, far_point, 0.5);\n"
        "                vec4 mid_projected = projection * vec4(mid_point,\n"
        "                                                       1.0);\n"
        "\n"
        "                if (mid_projected.w <= 0.0)\n"
        "                {\n"
        "                    break;\n"
        "                }\n"
        "\n"
        "                vec2 mid_uv = (mid_projected.xy / mid_projected.w)\n"
        "                              * 0.5 + 0.5;\n"
        "\n"
        "                if ((mid_uv.x < 0.0) || (mid_uv.x > 1.0) ||\n"
        "                    (mid_uv.y < 0.0) || (mid_uv.y > 1.0))\n"
        "                {\n"
        "                    break;\n"
        "                }\n"
        "\n"
        "                vec3 mid_scene = view_position(mid_uv);\n"
        "\n"
        "                if ((mid_scene.z - mid_point.z) > pc.params.x)\n"
        "                {\n"
        "                    far_point = mid_point;\n"
        "                }\n"
        "                else\n"
        "                {\n"
        "                    near_point = mid_point;\n"
        "                }\n"
        "            }\n"
        "\n"
        "            hit_point = far_point;\n"
        "            vec4 hit_projected = projection * vec4(hit_point, 1.0);\n"
        "            hit_uv = (hit_projected.xy / hit_projected.w)\n"
        "                     * 0.5 + 0.5;\n"
        "            break;\n"
        "        }\n"
        "    }\n"
        "\n"
        "    if (!hit)\n"
        "    {\n"
        "        outColor = vec4(0.0);\n"
        "        return;\n"
        "    }\n"
        "\n"
        "    // Затухание к краям экрана: у границы вклад отражения\n"
        "    // гасится, поверхность остаётся со своим (prelit) цветом —\n"
        "    // fallback для лучей, покидающих экран\n"
        "    vec2 border = min(hit_uv, 1.0 - hit_uv);\n"
        "    float edge = clamp(min(border.x, border.y) / 0.05, 0.0, 1.0);\n"
        "    edge *= edge;\n"
        "\n"
        "    vec3 reflection = texture(in_scene_color,\n"
        "                              clamp(hit_uv, 0.0, 1.0)).rgb;\n"
        "\n"
        "    // Дальность попадания — прокси шероховатости: дальние\n"
        "    // попадания шумнее (в альфу — радиус блюра)\n"
        "    float travel = length(hit_point - position);\n"
        "    float blur_factor = clamp(travel / max(pc.params.z, 1.0e-4),\n"
        "                              0.0, 1.0);\n"
        "\n"
        "    // Достоверность: ближние попадания точнее; френель-подобное\n"
        "    // усиление на скользящих углах\n"
        "    float grazing = 1.0 - clamp(dot(normal, view_dir), 0.0, 1.0);\n"
        "    float confidence = (1.0 - 0.5 * blur_factor)\n"
        "                        * (0.3 + 0.7 * grazing * grazing);\n"
        "\n"
        "    float weight = edge * confidence;\n"
        "\n"
        "    outColor = vec4(reflection * weight, blur_factor * weight);\n"
        "}\n";
}

//------------------------------------------------------------------------------
// GLSL: roughness-based билатеральный блюр SSR (один проход): радиус
// ядра масштабируется «шероховатостью» из альфы, вес по близости
// глубины сцены сохраняет границы объектов
//------------------------------------------------------------------------------
std::string ssr_blur_fragment_source()
{
    return
        std::string("#version 450\n") +
        "\n"
        "layout(binding = 0) uniform sampler2D in_ssr;\n"
        "layout(binding = 1) uniform sampler2D in_scene_depth;\n"
        "\n"
        "layout(push_constant) uniform Params\n"
        "{\n"
        "    vec4 params;    // xy: тексель буфера, z: сигма глубины,\n"
        "                    // w: макс. радиус блюра (текселей)\n"
        "} pc;\n"
        "\n"
        "layout(location = 0) in vec2 in_uv;\n"
        "layout(location = 0) out vec4 outColor;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    vec4 base = texture(in_ssr, in_uv);\n"
        "    float depth = texture(in_scene_depth, in_uv).r;\n"
        "\n"
        "    // «Шероховатость» из альфы: дальние попадания — шире ядро\n"
        "    float radius = mix(0.5, max(pc.params.w, 1.0), base.a);\n"
        "\n"
        "    vec4 sum = base;\n"
        "    float weight_sum = 1.0;\n"
        "\n"
        "    // Крест 5-tap (как у блюра SSAO): вес по дистанции,\n"
        "    // масштабируемой шероховатостью, + близости глубины\n"
        "    for (int i = 1; i <= 2; ++i)\n"
        "    {\n"
        "        float scale = radius * float(i);\n"
        "        float weight = 1.0 / (1.0 + float(i) * radius);\n"
        "\n"
        "        vec2 offset_x = vec2(pc.params.x * scale, 0.0);\n"
        "        vec4 ssr_p = texture(in_ssr, in_uv + offset_x);\n"
        "        float depth_p = texture(in_scene_depth,\n"
        "                                in_uv + offset_x).r;\n"
        "        float depth_w = exp(-abs(depth_p - depth)\n"
        "                            / max(pc.params.z, 1.0e-6));\n"
        "        sum += ssr_p * weight * depth_w;\n"
        "        weight_sum += weight * depth_w;\n"
        "\n"
        "        vec4 ssr_m = texture(in_ssr, in_uv - offset_x);\n"
        "        float depth_m = texture(in_scene_depth,\n"
        "                                in_uv - offset_x).r;\n"
        "        depth_w = exp(-abs(depth_m - depth)\n"
        "                      / max(pc.params.z, 1.0e-6));\n"
        "        sum += ssr_m * weight * depth_w;\n"
        "        weight_sum += weight * depth_w;\n"
        "\n"
        "        vec2 offset_y = vec2(0.0, pc.params.y * scale);\n"
        "        ssr_p = texture(in_ssr, in_uv + offset_y);\n"
        "        depth_p = texture(in_scene_depth, in_uv + offset_y).r;\n"
        "        depth_w = exp(-abs(depth_p - depth)\n"
        "                      / max(pc.params.z, 1.0e-6));\n"
        "        sum += ssr_p * weight * depth_w;\n"
        "        weight_sum += weight * depth_w;\n"
        "\n"
        "        ssr_m = texture(in_ssr, in_uv - offset_y);\n"
        "        depth_m = texture(in_scene_depth, in_uv - offset_y).r;\n"
        "        depth_w = exp(-abs(depth_m - depth)\n"
        "                      / max(pc.params.z, 1.0e-6));\n"
        "        sum += ssr_m * weight * depth_w;\n"
        "        weight_sum += weight * depth_w;\n"
        "    }\n"
        "\n"
        "    outColor = sum / weight_sum;\n"
        "}\n";
}

//------------------------------------------------------------------------------
// GLSL: финальный квад — композиция результата и вывод в swapchain.
// Битовая маска эффектов генерируется под конфигурацию цепочки,
// привязки объявляются только для собранных проходов
//------------------------------------------------------------------------------
std::string final_quad_fragment_source(bool bloom, bool ssao, bool fog,
                                       bool ssr)
{
    std::string source =
        std::string("#version 450\n") +
        "\n"
        "layout(binding = 0) uniform sampler2D in_scene_color;\n";

    if (bloom)
    {
        source += "layout(binding = 1) uniform sampler2D in_bloom;\n";
    }

    if (ssao)
    {
        source += "layout(binding = 2) uniform sampler2D in_ssao;\n";
    }

    if (fog)
    {
        source += "layout(binding = 3) uniform sampler2D in_fog;\n";
    }

    if (ssr)
    {
        source += "layout(binding = 4) uniform sampler2D in_ssr;\n";
    }

    source +=
        "\n"
        "layout(push_constant) uniform Params\n"
        "{\n"
        "    vec4 params;    // x: bloom, y: AO, z: туман, w: битовая маска\n"
        "    vec4 params2;   // x: интенсивность SSR, yzw: резерв\n"
        "} pc;\n"
        "\n"
        "layout(location = 0) in vec2 in_uv;\n"
        "layout(location = 0) out vec4 outColor;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    vec3 color = texture(in_scene_color, in_uv).rgb;\n"
        "\n"
        "    int flags = int(pc.params.w + 0.5);\n"
        "\n";

    if (ssao)
    {
        source +=
            "    // SSAO: multiply-композиция\n"
            "    if ((flags & 2) != 0)\n"
            "    {\n"
            "        color *= mix(1.0, texture(in_ssao, in_uv).r,\n"
            "                     pc.params.y);\n"
            "    }\n"
            "\n";
    }

    if (ssr)
    {
        source +=
            "    // SSR: аддитивная композиция (цвет отражения уже\n"
            "    // premultiplied на вес — edge fade гасит лучи за экраном,\n"
            "    // поверхность остаётся на своём prelit-цвете)\n"
            "    if ((flags & 8) != 0)\n"
            "    {\n"
            "        color += texture(in_ssr, in_uv).rgb * pc.params2.x;\n"
            "    }\n"
            "\n";
    }

    if (bloom)
    {
        source +=
            "    // Bloom: аддитивная композиция\n"
            "    if ((flags & 1) != 0)\n"
            "    {\n"
            "        color += texture(in_bloom, in_uv).rgb * pc.params.x;\n"
            "    }\n"
            "\n";
    }

    if (fog)
    {
        source +=
            "    // Туман: аддитивная композиция (затухание по глубине\n"
            "    // сцены учтено в самом raymarch-проходе)\n"
            "    if ((flags & 4) != 0)\n"
            "    {\n"
            "        color += texture(in_fog, in_uv).rgb * pc.params.z;\n"
            "    }\n"
            "\n";
    }

    source +=
        "    outColor = vec4(color, 1.0);\n"
        "}\n";

    return source;
}

//------------------------------------------------------------------------------
// Offscreen-таргет: vsg::Image + vsg::ImageView (память выделяет
// vsg::createImageView — сверено с vsg/state/ImageView.h)
//------------------------------------------------------------------------------
struct RenderTarget
{
    vsg::ref_ptr<vsg::Image> image;
    vsg::ref_ptr<vsg::ImageView> view;
};

RenderTarget create_color_target(vsg::Device* device, const VkExtent2D& extent)
{
    RenderTarget target;

    target.image = vsg::Image::create();
    target.image->imageType = VK_IMAGE_TYPE_2D;
    target.image->format = HDR_COLOR_FORMAT;
    target.image->extent = VkExtent3D{extent.width, extent.height, 1};
    target.image->mipLevels = 1;
    target.image->arrayLayers = 1;
    target.image->samples = VK_SAMPLE_COUNT_1_BIT;
    target.image->usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                          VK_IMAGE_USAGE_SAMPLED_BIT;

    target.view = vsg::createImageView(device, target.image,
                                       VK_IMAGE_ASPECT_COLOR_BIT);

    return target;
}

RenderTarget create_depth_target(vsg::Device* device, const VkExtent2D& extent)
{
    RenderTarget target;

    target.image = vsg::Image::create();
    target.image->imageType = VK_IMAGE_TYPE_2D;
    target.image->format = SCENE_DEPTH_FORMAT;
    target.image->extent = VkExtent3D{extent.width, extent.height, 1};
    target.image->mipLevels = 1;
    target.image->arrayLayers = 1;
    target.image->samples = VK_SAMPLE_COUNT_1_BIT;
    // Глубина читается SSAO/туманом -> SAMPLED обязателен
    target.image->usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                          VK_IMAGE_USAGE_SAMPLED_BIT;

    target.view = vsg::createImageView(device, target.image,
                                       VK_IMAGE_ASPECT_DEPTH_BIT);

    return target;
}

//------------------------------------------------------------------------------
// RenderPass с одним цветевым аттачментом, читаемым далее как текстура:
// finalLayout = SHADER_READ_ONLY, зависимости пишут-затем-читают
//------------------------------------------------------------------------------
vsg::ref_ptr<vsg::RenderPass> create_color_sampled_renderpass(
    vsg::Device* device)
{
    const vsg::RenderPass::Attachments attachments{
        {
            0,                                          // flags
            HDR_COLOR_FORMAT,                           // format
            VK_SAMPLE_COUNT_1_BIT,                      // samples
            VK_ATTACHMENT_LOAD_OP_CLEAR,                // loadOp
            VK_ATTACHMENT_STORE_OP_STORE,               // storeOp
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,            // stencilLoadOp
            VK_ATTACHMENT_STORE_OP_DONT_CARE,           // stencilStoreOp
            VK_IMAGE_LAYOUT_UNDEFINED,                  // initialLayout
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL    // finalLayout
        }
    };

    const vsg::RenderPass::Subpasses subpasses{
        {
            0,                                              // flags
            VK_PIPELINE_BIND_POINT_GRAPHICS,                // bindPoint
            {},                                             // inputAttachments
            { {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL} },
            {},                                             // resolveAttachments
            {},                                             // depthStencil
            {}                                              // preserve
        }
    };

    const vsg::RenderPass::Dependencies dependencies{
        {
            VK_SUBPASS_EXTERNAL,                           // srcSubpass
            0,                                             // dstSubpass
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,             // srcStageMask
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // dstStageMask
            0,                                             // srcAccessMask
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,          // dstAccessMask
            VK_DEPENDENCY_BY_REGION_BIT                    // dependencyFlags
        },
        {
            0,                                              // srcSubpass
            VK_SUBPASS_EXTERNAL,                            // dstSubpass
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // srcStageMask
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,          // dstStageMask
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,           // srcAccessMask
            VK_ACCESS_SHADER_READ_BIT,                      // dstAccessMask
            VK_DEPENDENCY_BY_REGION_BIT                     // dependencyFlags
        }
    };

    return vsg::RenderPass::create(device, attachments, subpasses,
                                   dependencies);
}

//------------------------------------------------------------------------------
// RenderPass прохода сцены: HDR-цвет + D32, оба читаются дальше по цепочке
//------------------------------------------------------------------------------
vsg::ref_ptr<vsg::RenderPass> create_scene_renderpass(vsg::Device* device)
{
    const vsg::RenderPass::Attachments attachments{
        {
            0,                                          // flags
            HDR_COLOR_FORMAT,                           // format
            VK_SAMPLE_COUNT_1_BIT,                      // samples
            VK_ATTACHMENT_LOAD_OP_CLEAR,                // loadOp
            VK_ATTACHMENT_STORE_OP_STORE,               // storeOp
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,            // stencilLoadOp
            VK_ATTACHMENT_STORE_OP_DONT_CARE,           // stencilStoreOp
            VK_IMAGE_LAYOUT_UNDEFINED,                  // initialLayout
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL    // finalLayout
        },
        {
            0,                                          // flags
            SCENE_DEPTH_FORMAT,                         // format
            VK_SAMPLE_COUNT_1_BIT,                      // samples
            VK_ATTACHMENT_LOAD_OP_CLEAR,                // loadOp
            VK_ATTACHMENT_STORE_OP_STORE,               // storeOp
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,            // stencilLoadOp
            VK_ATTACHMENT_STORE_OP_DONT_CARE,           // stencilStoreOp
            VK_IMAGE_LAYOUT_UNDEFINED,                  // initialLayout
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL // finalLayout
        }
    };

    const vsg::RenderPass::Subpasses subpasses{
        {
            0,                                              // flags
            VK_PIPELINE_BIND_POINT_GRAPHICS,                // bindPoint
            {},                                             // inputAttachments
            { {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL} },
            {},                                             // resolveAttachments
            { {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL} },
            {}                                              // preserve
        }
    };

    const vsg::RenderPass::Dependencies dependencies{
        {
            VK_SUBPASS_EXTERNAL,                            // srcSubpass
            0,                                              // dstSubpass
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,              // srcStageMask
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, // dstStageMask
            0,                                              // srcAccessMask
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_DEPENDENCY_BY_REGION_BIT                     // dependencyFlags
        },
        {
            0,                                              // srcSubpass
            VK_SUBPASS_EXTERNAL,                            // dstSubpass
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,  // srcStageMask
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,          // dstStageMask
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT,                      // dstAccessMask
            VK_DEPENDENCY_BY_REGION_BIT                     // dependencyFlags
        }
    };

    return vsg::RenderPass::create(device, attachments, subpasses,
                                   dependencies);
}

//------------------------------------------------------------------------------
// Сэмплеры: линейный для цветов, NEAREST для глубины (линейная фильтрация
// D32_SFLOAT опциональна у драйверов — не полагаемся на неё)
//------------------------------------------------------------------------------
vsg::ref_ptr<vsg::Sampler> create_linear_clamp_sampler()
{
    auto sampler = vsg::Sampler::create();
    sampler->minFilter = VK_FILTER_LINEAR;
    sampler->magFilter = VK_FILTER_LINEAR;
    sampler->addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler->addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler->addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    return sampler;
}

vsg::ref_ptr<vsg::Sampler> create_nearest_clamp_sampler()
{
    auto sampler = vsg::Sampler::create();
    sampler->minFilter = VK_FILTER_NEAREST;
    sampler->magFilter = VK_FILTER_NEAREST;
    sampler->addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler->addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler->addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    return sampler;
}

//------------------------------------------------------------------------------
// Дескриптор сэмплер-текстуры из offscreen-таргета
// (vsg::ImageInfo{sampler, imageView, layout} -> vsg::DescriptorImage)
//------------------------------------------------------------------------------
vsg::ref_ptr<vsg::DescriptorImage> create_sampled_descriptor(
    vsg::Sampler* sampler,
    vsg::ImageView* view,
    uint32_t binding)
{
    // ref_ptr из сырых указателей строится явно (конструктор explicit)
    auto image_info = vsg::ImageInfo::create(
        vsg::ref_ptr<vsg::Sampler>(sampler),
        vsg::ref_ptr<vsg::ImageView>(view),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    return vsg::DescriptorImage::create(
        vsg::ImageInfoList{image_info}, binding);
}

//------------------------------------------------------------------------------
// Динамические push-константы прохода: читают камеру/солнце В МОМЕНТ
// записи кадра (record), поэтому матрицы всегда актуальны и отдельный
// механизм обновления буферов не нужен.
//
// Раскладка данных (push-константы следуют правилам std430):
//  - Kind::Ssao: mat4 inverse_projection + vec4 params            (80 Б)
//  - Kind::Ssr:  mat4 inverse_projection + vec4 params            (80 Б)
//    (толщина/шаги/дистанция/итерации поиска — та же раскладка)
//  - Kind::Fog:  mat4 inverse_projection + vec4 sun + vec4 params  (96 Б)
//    sun.xyz — направление НА солнце в пространстве вида (источник
//    вращается в мире — поворачиваем матрицей вида)
//------------------------------------------------------------------------------
class DynamicPushConstants final : public vsg::Inherit<vsg::Command,
                                                       DynamicPushConstants>
{
public:
    enum class Kind
    {
        Ssao,
        Fog,
        Ssr
    };

    DynamicPushConstants(Kind kind,
                         vsg::ref_ptr<vsg::PipelineLayout> layout,
                         vsg::ref_ptr<vsg::Camera> camera,
                         vsg::ref_ptr<vsg::DirectionalLight> light,
                         const vsg::vec4& static_params_a,
                         const vsg::vec4& static_params_b) :
        kind_(kind),
        layout_(std::move(layout)),
        camera_(std::move(camera)),
        light_(std::move(light)),
        static_params_a_(static_params_a),
        static_params_b_(static_params_b)
    {
    }

    void record(vsg::CommandBuffer& commandBuffer) const override
    {
        if (!layout_ || !camera_ || !camera_->projectionMatrix)
        {
            return;
        }

        // Обратная проекция (double -> float для шейдера)
        const vsg::dmat4 inverse_projection_d =
                camera_->projectionMatrix->inverse();
        const vsg::mat4 inverse_projection(inverse_projection_d);

        if (kind_ == Kind::Fog)
        {
            // DirectionalLight::direction — куда светит; рассеяние — к
            // источнику, поэтому в шейдер уходит противоположный вектор
            vsg::vec3 sun_dir(0.0f, 0.0f, 1.0f);

            if (light_ && camera_->viewMatrix)
            {
                const vsg::dmat4 view = camera_->viewMatrix->transform();
                const vsg::dvec3 to_sun = -light_->direction;
                const vsg::dvec4 rotated = view * vsg::dvec4(to_sun, 0.0);
                const vsg::dvec3 normalized = vsg::normalize(
                    vsg::dvec3(rotated.x, rotated.y, rotated.z));
                sun_dir = vsg::vec3(static_cast<float>(normalized.x),
                                    static_cast<float>(normalized.y),
                                    static_cast<float>(normalized.z));
            }

            struct
            {
                vsg::mat4 inverse_projection;
                vsg::vec4 sun;
                vsg::vec4 params;
            } data{inverse_projection,
                   vsg::vec4(sun_dir, static_params_a_.w),
                   static_params_b_};

            vkCmdPushConstants(commandBuffer,
                               layout_->vk(commandBuffer.deviceID),
                               VK_SHADER_STAGE_FRAGMENT_BIT,
                               0, sizeof(data), &data);
        }
        else
        {
            // Kind::Ssao и Kind::Ssr: одна раскладка mat4 + vec4 (80 Б)
            struct
            {
                vsg::mat4 inverse_projection;
                vsg::vec4 params;
            } data{inverse_projection, static_params_a_};

            vkCmdPushConstants(commandBuffer,
                               layout_->vk(commandBuffer.deviceID),
                               VK_SHADER_STAGE_FRAGMENT_BIT,
                               0, sizeof(data), &data);
        }
    }

private:
    Kind kind_ = Kind::Ssao;
    vsg::ref_ptr<vsg::PipelineLayout> layout_;
    vsg::ref_ptr<vsg::Camera> camera_;
    vsg::ref_ptr<vsg::DirectionalLight> light_;
    vsg::vec4 static_params_a_;
    vsg::vec4 static_params_b_;
};

//------------------------------------------------------------------------------
// Полноэкранный проход: собственный GraphicsPipeline + big-triangle,
// рендер в собственный offscreen-таргет. push_factory (если задан)
// получает PipelineLayout созданного прохода и возвращает Command с
// push-константами (статичными или динамическими от камеры/солнца)
//------------------------------------------------------------------------------
using PushCommandFactory =
    std::function<vsg::ref_ptr<vsg::Command>(vsg::ref_ptr<vsg::PipelineLayout>)>;

struct FullscreenPass
{
    vsg::ref_ptr<vsg::RenderGraph> renderGraph;
    vsg::ref_ptr<vsg::PipelineLayout> pipelineLayout;
    RenderTarget target;
};

FullscreenPass createFullscreenPass(
    vsg::Device* device,
    const VkExtent2D& extent,
    const std::string& fragment_source,
    const vsg::DescriptorSetLayoutBindings& layout_bindings,
    const vsg::Descriptors& descriptors,
    uint32_t push_constant_size,
    VkSampleCountFlagBits samples,
    PushCommandFactory push_factory = {})
{
    FullscreenPass pass;

    // Шейдеры из GLSL-строк: компилируются glslang'ом при компиляции
    // пайплайна (как шейдеры проекта из data/shaders — options проекта
    // регистрируют vsgXchange::all)
    auto vertexShader = vsg::ShaderStage::create(
        VK_SHADER_STAGE_VERTEX_BIT, "main", fullscreen_vertex_source());
    auto fragmentShader = vsg::ShaderStage::create(
        VK_SHADER_STAGE_FRAGMENT_BIT, "main", fragment_source);

    vsg::PushConstantRanges push_ranges;
    if (push_constant_size > 0)
    {
        push_ranges.push_back(VkPushConstantRange{
            VK_SHADER_STAGE_FRAGMENT_BIT, 0, push_constant_size});
    }

    const auto descriptor_set_layout =
            vsg::DescriptorSetLayout::create(layout_bindings);

    const auto pipeline_layout = vsg::PipelineLayout::create(
        vsg::DescriptorSetLayouts{descriptor_set_layout}, push_ranges);

    // Пайплайн-состояние: без глубины (у RenderPass прохода только
    // цветовой аттачмент), без блендинга, без cull (один треугольник)
    auto rasterizationState = vsg::RasterizationState::create();
    rasterizationState->cullMode = VK_CULL_MODE_NONE;

    auto depthStencilState = vsg::DepthStencilState::create();
    depthStencilState->depthTestEnable = VK_FALSE;
    depthStencilState->depthWriteEnable = VK_FALSE;

    const vsg::GraphicsPipelineStates pipeline_states{
        vsg::VertexInputState::create(),    // без привязок: gl_VertexIndex
        vsg::InputAssemblyState::create(),  // TRIANGLE_LIST по умолчанию
        rasterizationState,
        vsg::MultisampleState::create(samples),
        vsg::ColorBlendState::create(),     // блендинг отключён
        depthStencilState
    };

    const auto graphics_pipeline = vsg::GraphicsPipeline::create(
        pipeline_layout,
        vsg::ShaderStages{vertexShader, fragmentShader},
        pipeline_states);

    const auto descriptor_set = vsg::DescriptorSet::create(
        descriptor_set_layout, descriptors);
    const auto bind_descriptor_set = vsg::BindDescriptorSet::create(
        VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0,
        descriptor_set);

    auto state_group = vsg::StateGroup::create();
    state_group->add(vsg::BindGraphicsPipeline::create(graphics_pipeline));
    state_group->add(bind_descriptor_set);

    // Push-константы до отрисовки (Command-ребёнок — записывается в
    // порядке обхода, состояние пайплайна уже установлено)
    if (push_factory)
    {
        if (auto push_command = push_factory(pipeline_layout))
        {
            state_group->addChild(push_command);
        }
    }

    // Big-triangle: 3 вершины из gl_VertexIndex, буферов нет
    state_group->addChild(vsg::Draw::create(3, 1, 0, 0));

    // Таргет и RenderGraph
    pass.target = create_color_target(device, extent);

    const auto render_pass = create_color_sampled_renderpass(device);
    const auto framebuffer = vsg::Framebuffer::create(
        render_pass, vsg::ImageViews{pass.target.view},
        extent.width, extent.height, 1);

    auto render_graph = vsg::RenderGraph::create();
    render_graph->framebuffer = framebuffer;
    render_graph->renderArea = VkRect2D{VkOffset2D{0, 0}, extent};
    render_graph->viewportState = vsg::ViewportState::create(extent);
    render_graph->setClearValues({{0.0f, 0.0f, 0.0f, 1.0f}}, {0.0f, 0});
    render_graph->addChild(state_group);

    pass.renderGraph = render_graph;
    pass.pipelineLayout = pipeline_layout;

    return pass;
}

/// Фабрика статичных push-констант (порог блума, направление блюра и т.п.)
PushCommandFactory static_push_factory(const vsg::vec4& value)
{
    return [value](vsg::ref_ptr<vsg::PipelineLayout>)
    {
        return vsg::PushConstants::create(
            VK_SHADER_STAGE_FRAGMENT_BIT, 0, vsg::vec4Value::create(value));
    };
}

VkExtent2D scaled_extent(const VkExtent2D& extent, double divisor)
{
    return VkExtent2D{
        std::max(1u, static_cast<std::uint32_t>(
                         static_cast<double>(extent.width) / divisor)),
        std::max(1u, static_cast<std::uint32_t>(
                         static_cast<double>(extent.height) / divisor))};
}

/// Описание биндинга сэмплер-текстуры фрагментного шейдера
VkDescriptorSetLayoutBinding sampled_binding(uint32_t binding)
{
    return VkDescriptorSetLayoutBinding{
        binding,                                       // binding
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,     // descriptorType
        1,                                             // descriptorCount
        VK_SHADER_STAGE_FRAGMENT_BIT,                  // stageFlags
        nullptr};                                      // pImmutableSamplers
}

} // namespace

//------------------------------------------------------------------------------
// Pimpl: offscreen-таргеты и сэмплеры цепочки
//------------------------------------------------------------------------------
class graphics::PostProcessChain::Impl
{
public:
    RenderTarget scene_color;    ///< HDR-цвет сцены (масштаб chain scale)
    RenderTarget scene_depth;    ///< D32 сцены (масштаб chain scale)
    RenderTarget bloom_bright;   ///< bright-pass (half-res)
    RenderTarget bloom_blur_a;   ///< гаусс H (half-res)
    RenderTarget bloom_blur_b;   ///< гаусс V (half-res, результат)
    RenderTarget ssao_raw;       ///< ядро AO (half-res)
    RenderTarget ssao_blur;      ///< билатеральный блюр (half-res)
    RenderTarget fog_color;      ///< raymarch тумана (quarter-res)
    RenderTarget ssr_raw;        ///< raymarch отражений (quarter-res)
    RenderTarget ssr_blur;       ///< roughness-блюр отражений (quarter-res)

    vsg::ref_ptr<vsg::Sampler> linear_sampler;
    vsg::ref_ptr<vsg::Sampler> nearest_sampler;
};

namespace graphics
{

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PostProcessChain::PostProcessChain(const PostProcessConfig& config) :
    impl_(new Impl),
    config_(config)
{
}

PostProcessChain::~PostProcessChain()
{
    delete impl_;
}

//------------------------------------------------------------------------------
// Проход сцены в offscreen: RenderGraph с собственными framebuffer /
// renderPass / clearValues (структура — как у RenderGraph(окно, вид),
// но цель — HDR-буферы цепочки)
//------------------------------------------------------------------------------
vsg::ref_ptr<vsg::RenderGraph> PostProcessChain::createSceneRenderGraph(
    vsg::View* view,
    vsg::Device* device,
    const VkExtent2D& extent)
{
    if (!view || !device)
    {
        Journal::instance()->warning(
            "PostProcess: no view/device for scene render graph");
        return {};
    }

    // Viewport камеры вида = extent offscreen-буфера: камера — общий
    // источник куллинга/проекции, держим его согласованным с целью
    if (view->camera)
    {
        view->camera->viewportState = vsg::ViewportState::create(extent);
    }

    impl_->scene_color = create_color_target(device, extent);
    impl_->scene_depth = create_depth_target(device, extent);

    const auto render_pass = create_scene_renderpass(device);

    const auto framebuffer = vsg::Framebuffer::create(
        render_pass,
        vsg::ImageViews{impl_->scene_color.view, impl_->scene_depth.view},
        extent.width, extent.height, 1);

    auto render_graph = vsg::RenderGraph::create();
    render_graph->framebuffer = framebuffer;
    render_graph->renderArea = VkRect2D{VkOffset2D{0, 0}, extent};
    render_graph->viewportState = vsg::ViewportState::create(extent);

    // Reversed-Z VSG: очистка глубины 0.0 (значение по умолчанию
    // vsg::RenderGraph::setClearValues — сверено с заголовком)
    render_graph->setClearValues({{0.0f, 0.0f, 0.0f, 1.0f}}, {0.0f, 0});

    render_graph->addChild(vsg::ref_ptr<vsg::Node>(view));

    return render_graph;
}

//------------------------------------------------------------------------------
// Полноэкранные проходы эффектов (каждый — свой RenderGraph со своим
// framebuffer). Порядок добавления = порядок исполнения в CommandGraph
//------------------------------------------------------------------------------
void PostProcessChain::createEffectPasses(
    vsg::Device* device,
    vsg::Camera* camera,
    vsg::DirectionalLight* light,
    const VkExtent2D& scene_extent)
{
    if (!device || !impl_->scene_color.view || !impl_->scene_depth.view)
    {
        Journal::instance()->warning(
            "PostProcess: no device/scene targets for effect passes "
            "(createSceneRenderGraph must be called first)");
        return;
    }

    impl_->linear_sampler = create_linear_clamp_sampler();
    impl_->nearest_sampler = create_nearest_clamp_sampler();

    const VkExtent2D half_extent = scaled_extent(scene_extent, 2.0);
    const VkExtent2D quarter_extent = scaled_extent(scene_extent, 4.0);

    //------------------------------------------------------------------
    // BLOOM: bright-pass -> гаусс H -> гаусс V (всё half-res).
    // Аддитивная композиция (интенсивность) — в финальном кваде
    //------------------------------------------------------------------
    if (config_.bloom)
    {
        const float texel_x = 1.0f / static_cast<float>(half_extent.width);
        const float texel_y = 1.0f / static_cast<float>(half_extent.height);

        // 1. Bright-pass: сцена -> яркое
        auto bright = createFullscreenPass(
            device, half_extent,
            bloom_bright_fragment_source(),
            vsg::DescriptorSetLayoutBindings{sampled_binding(0)},
            vsg::Descriptors{create_sampled_descriptor(
                impl_->linear_sampler.get(),
                impl_->scene_color.view.get(), 0)},
            sizeof(vsg::vec4),
            VK_SAMPLE_COUNT_1_BIT,
            static_push_factory(
                vsg::vec4(static_cast<float>(config_.bloom_threshold),
                          0.0f, 0.0f, 0.0f)));

        // 2. Гаусс по горизонтали: яркое -> blur A
        auto blur_h = createFullscreenPass(
            device, half_extent,
            blur_fragment_source(),
            vsg::DescriptorSetLayoutBindings{sampled_binding(0)},
            vsg::Descriptors{create_sampled_descriptor(
                impl_->linear_sampler.get(),
                bright.target.view.get(), 0)},
            sizeof(vsg::vec4),
            VK_SAMPLE_COUNT_1_BIT,
            static_push_factory(
                vsg::vec4(texel_x, 0.0f, 0.0f, 0.0f)));

        // 3. Гаусс по вертикали: blur A -> результат
        auto blur_v = createFullscreenPass(
            device, half_extent,
            blur_fragment_source(),
            vsg::DescriptorSetLayoutBindings{sampled_binding(0)},
            vsg::Descriptors{create_sampled_descriptor(
                impl_->linear_sampler.get(),
                blur_h.target.view.get(), 0)},
            sizeof(vsg::vec4),
            VK_SAMPLE_COUNT_1_BIT,
            static_push_factory(
                vsg::vec4(0.0f, texel_y, 0.0f, 0.0f)));

        impl_->bloom_bright = bright.target;
        impl_->bloom_blur_a = blur_h.target;
        impl_->bloom_blur_b = blur_v.target;

        effect_passes_.push_back(bright.renderGraph);
        effect_passes_.push_back(blur_h.renderGraph);
        effect_passes_.push_back(blur_v.renderGraph);

        bloom_built_ = true;
    }

    //------------------------------------------------------------------
    // SSAO: ядро 16 сэмплов (half-res) -> билатеральный блюр (half-res).
    // Реконструкция позиций/нормалей из depth через inverse projection
    // камеры (push-константы на каждый кадр). Multiply-композиция —
    // в финальном кваде
    //------------------------------------------------------------------
    if (config_.ssao && camera)
    {
        const float texel_x = 1.0f / static_cast<float>(half_extent.width);
        const float texel_y = 1.0f / static_cast<float>(half_extent.height);

        // 1. Ядро AO: глубина сцены (NEAREST — линейная фильтрация
        //    D32_SFLOAT опциональна у драйверов). Матрицы камеры —
        //    push-константами на каждый кадр
        auto ao = createFullscreenPass(
            device, half_extent,
            ssao_fragment_source(),
            vsg::DescriptorSetLayoutBindings{sampled_binding(0)},
            vsg::Descriptors{create_sampled_descriptor(
                impl_->nearest_sampler.get(),
                impl_->scene_depth.view.get(), 0)},
            80,    // mat4 inverse_projection + vec4 params
            VK_SAMPLE_COUNT_1_BIT,
            [camera, light, this](vsg::ref_ptr<vsg::PipelineLayout> layout)
            {
                return DynamicPushConstants::create(
                    DynamicPushConstants::Kind::Ssao,
                    layout,
                    vsg::ref_ptr<vsg::Camera>(camera),
                    vsg::ref_ptr<vsg::DirectionalLight>(light),
                    vsg::vec4(
                        static_cast<float>(config_.ssao_radius_m),
                        static_cast<float>(config_.ssao_samples),
                        static_cast<float>(config_.ssao_bias),
                        static_cast<float>(config_.ssao_strength)),
                    vsg::vec4());
            });

        // 2. Билатеральный блюр: AO + глубина
        auto blur = createFullscreenPass(
            device, half_extent,
            ssao_blur_fragment_source(),
            vsg::DescriptorSetLayoutBindings{sampled_binding(0),
                                             sampled_binding(1)},
            vsg::Descriptors{
                create_sampled_descriptor(impl_->linear_sampler.get(),
                                          ao.target.view.get(), 0),
                create_sampled_descriptor(impl_->nearest_sampler.get(),
                                          impl_->scene_depth.view.get(), 1)},
            sizeof(vsg::vec4),
            VK_SAMPLE_COUNT_1_BIT,
            static_push_factory(
                vsg::vec4(texel_x, texel_y, 0.002f, 0.0f)));

        impl_->ssao_raw = ao.target;
        impl_->ssao_blur = blur.target;

        effect_passes_.push_back(ao.renderGraph);
        effect_passes_.push_back(blur.renderGraph);

        ssao_built_ = true;
    }
    else if (config_.ssao)
    {
        Journal::instance()->warning(
            "PostProcess: SSAO pass skipped - no camera");
    }

    //------------------------------------------------------------------
    // VOLUMETRIC FOG: quarter-res raymarch 24 шага до 200 м,
    // анизотропное рассеяние к солнцу. Аддитивная композиция —
    // в финальном кваде
    //------------------------------------------------------------------
    if (config_.volumetric_fog && camera)
    {
        auto fog = createFullscreenPass(
            device, quarter_extent,
            fog_fragment_source(),
            vsg::DescriptorSetLayoutBindings{sampled_binding(0)},
            vsg::Descriptors{create_sampled_descriptor(
                impl_->nearest_sampler.get(),
                impl_->scene_depth.view.get(), 0)},
            96,    // mat4 inverse_projection + vec4 sun + vec4 params
            VK_SAMPLE_COUNT_1_BIT,
            [camera, light, this](vsg::ref_ptr<vsg::PipelineLayout> layout)
            {
                return DynamicPushConstants::create(
                    DynamicPushConstants::Kind::Fog,
                    layout,
                    vsg::ref_ptr<vsg::Camera>(camera),
                    vsg::ref_ptr<vsg::DirectionalLight>(light),
                    vsg::vec4(0.0f, 0.0f, 0.0f,
                              static_cast<float>(config_.fog_scattering)),
                    vsg::vec4(
                        static_cast<float>(config_.fog_distance_m),
                        static_cast<float>(config_.fog_steps),
                        static_cast<float>(config_.fog_density),
                        0.6f));
            });

        impl_->fog_color = fog.target;

        effect_passes_.push_back(fog.renderGraph);

        fog_built_ = true;
    }
    else if (config_.volumetric_fog)
    {
        Journal::instance()->warning(
            "PostProcess: volumetric fog pass skipped - no camera");
    }

    //------------------------------------------------------------------
    // SSR: quarter-res raymarch 16 шагов в screen-space (толщина слоя
    // 0.3 м, реконструкция позиций из depth + inverse projection как
    // SSAO, бинарный поиск уточнения 4 итерации, затухание к краям
    // экрана) -> roughness-based билатеральный блюр (один проход).
    // Аддитивная композиция (интенсивность) — в финальном кваде
    //------------------------------------------------------------------
    if (config_.ssr && camera)
    {
        const float texel_x = 1.0f / static_cast<float>(quarter_extent.width);
        const float texel_y = 1.0f / static_cast<float>(quarter_extent.height);

        // 1. Raymarch: HDR-цвет сцены (linear) + глубина (NEAREST —
        //    линейная фильтрация D32_SFLOAT опциональна у драйверов).
        //    Матрица камеры — push-константами на каждый кадр
        auto ssr = createFullscreenPass(
            device, quarter_extent,
            ssr_raymarch_fragment_source(),
            vsg::DescriptorSetLayoutBindings{sampled_binding(0),
                                             sampled_binding(1)},
            vsg::Descriptors{
                create_sampled_descriptor(impl_->linear_sampler.get(),
                                          impl_->scene_color.view.get(), 0),
                create_sampled_descriptor(impl_->nearest_sampler.get(),
                                          impl_->scene_depth.view.get(), 1)},
            80,    // mat4 inverse_projection + vec4 params
            VK_SAMPLE_COUNT_1_BIT,
            [camera, this](vsg::ref_ptr<vsg::PipelineLayout> layout)
            {
                return DynamicPushConstants::create(
                    DynamicPushConstants::Kind::Ssr,
                    layout,
                    vsg::ref_ptr<vsg::Camera>(camera),
                    vsg::ref_ptr<vsg::DirectionalLight>(),
                    vsg::vec4(
                        static_cast<float>(config_.ssr_thickness_m),
                        static_cast<float>(config_.ssr_steps),
                        static_cast<float>(config_.ssr_max_distance_m),
                        static_cast<float>(config_.ssr_refine_steps)),
                    vsg::vec4());
            });

        // 2. Roughness-билатеральный блюр: отражения + глубина сцены
        //    (радиус ядра из альфы, границы — по близости глубины)
        auto blur = createFullscreenPass(
            device, quarter_extent,
            ssr_blur_fragment_source(),
            vsg::DescriptorSetLayoutBindings{sampled_binding(0),
                                             sampled_binding(1)},
            vsg::Descriptors{
                create_sampled_descriptor(impl_->linear_sampler.get(),
                                          ssr.target.view.get(), 0),
                create_sampled_descriptor(impl_->nearest_sampler.get(),
                                          impl_->scene_depth.view.get(), 1)},
            sizeof(vsg::vec4),
            VK_SAMPLE_COUNT_1_BIT,
            static_push_factory(
                vsg::vec4(texel_x, texel_y, 0.002f, 3.0f)));

        impl_->ssr_raw = ssr.target;
        impl_->ssr_blur = blur.target;

        effect_passes_.push_back(ssr.renderGraph);
        effect_passes_.push_back(blur.renderGraph);

        ssr_built_ = true;
    }
    else if (config_.ssr)
    {
        Journal::instance()->warning(
            "PostProcess: SSR pass skipped - no camera");
    }
}

//------------------------------------------------------------------------------
// Финальный квад: композиция сцены и эффектов -> текущий framebuffer
// окна. Возвращается УЗЕЛ (не RenderGraph): встраивается в главный
// RenderGraph окна ПЕРЕД RenderImGui, пайплайн компилируется под
// render pass окна
//------------------------------------------------------------------------------
vsg::ref_ptr<vsg::Node> PostProcessChain::createFinalQuad(vsg::Window* window)
{
    if (!window || !impl_->scene_color.view || !impl_->linear_sampler)
    {
        Journal::instance()->warning(
            "PostProcess: no window/chain targets for final quad");
        return {};
    }

    vsg::DescriptorSetLayoutBindings layout_bindings{
        sampled_binding(0)};
    vsg::Descriptors descriptors{create_sampled_descriptor(
        impl_->linear_sampler.get(), impl_->scene_color.view.get(), 0)};

    // Битовая маска эффектов финального шейдера: 1 - bloom, 2 - SSAO,
    // 4 - туман, 8 - SSR (шейдер сгенерирован под собранные проходы)
    float effect_flags = 0.0f;

    vsg::vec4 composite_params(
        static_cast<float>(config_.bloom_intensity),
        1.0f,
        1.0f,
        0.0f);

    // Второй блок push-констант (смещение 16): интенсивность SSR.
    // Первый vec4 (bloom/AO/туман/маска) уже занят целиком
    vsg::vec4 composite_params2(
        static_cast<float>(config_.ssr_intensity),
        0.0f,
        0.0f,
        0.0f);

    if (bloom_built_)
    {
        layout_bindings.push_back(sampled_binding(1));
        descriptors.push_back(create_sampled_descriptor(
            impl_->linear_sampler.get(), impl_->bloom_blur_b.view.get(), 1));
        effect_flags += 1.0f;
    }

    if (ssao_built_)
    {
        layout_bindings.push_back(sampled_binding(2));
        descriptors.push_back(create_sampled_descriptor(
            impl_->linear_sampler.get(), impl_->ssao_blur.view.get(), 2));
        effect_flags += 2.0f;
    }

    if (fog_built_)
    {
        layout_bindings.push_back(sampled_binding(3));
        descriptors.push_back(create_sampled_descriptor(
            impl_->linear_sampler.get(), impl_->fog_color.view.get(), 3));
        effect_flags += 4.0f;
    }

    if (ssr_built_)
    {
        layout_bindings.push_back(sampled_binding(4));
        descriptors.push_back(create_sampled_descriptor(
            impl_->linear_sampler.get(), impl_->ssr_blur.view.get(), 4));
        effect_flags += 8.0f;
    }

    composite_params.w = effect_flags;

    auto vertexShader = vsg::ShaderStage::create(
        VK_SHADER_STAGE_VERTEX_BIT, "main", fullscreen_vertex_source());
    auto fragmentShader = vsg::ShaderStage::create(
        VK_SHADER_STAGE_FRAGMENT_BIT, "main",
        final_quad_fragment_source(bloom_built_, ssao_built_, fog_built_,
                                   ssr_built_));

    const vsg::PushConstantRanges push_ranges{
        VkPushConstantRange{VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                            2 * sizeof(vsg::vec4)}
    };

    const auto descriptor_set_layout =
            vsg::DescriptorSetLayout::create(layout_bindings);
    const auto pipeline_layout = vsg::PipelineLayout::create(
        vsg::DescriptorSetLayouts{descriptor_set_layout}, push_ranges);

    // Пайплайн: глубина окна не используется, cull выключен.
    // rasterizationSamples = сэмплам фреймбуфера окна (окно создано
    // с MSAA пресета Extreme)
    auto rasterizationState = vsg::RasterizationState::create();
    rasterizationState->cullMode = VK_CULL_MODE_NONE;

    auto depthStencilState = vsg::DepthStencilState::create();
    depthStencilState->depthTestEnable = VK_FALSE;
    depthStencilState->depthWriteEnable = VK_FALSE;

    const vsg::GraphicsPipelineStates pipeline_states{
        vsg::VertexInputState::create(),
        vsg::InputAssemblyState::create(),
        rasterizationState,
        vsg::MultisampleState::create(window->framebufferSamples()),
        vsg::ColorBlendState::create(),
        depthStencilState
    };

    const auto graphics_pipeline = vsg::GraphicsPipeline::create(
        pipeline_layout,
        vsg::ShaderStages{vertexShader, fragmentShader},
        pipeline_states);

    const auto descriptor_set = vsg::DescriptorSet::create(
        descriptor_set_layout, descriptors);
    const auto bind_descriptor_set = vsg::BindDescriptorSet::create(
        VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0,
        descriptor_set);

    auto state_group = vsg::StateGroup::create();
    state_group->add(vsg::BindGraphicsPipeline::create(graphics_pipeline));
    state_group->add(bind_descriptor_set);
    state_group->addChild(vsg::PushConstants::create(
        VK_SHADER_STAGE_FRAGMENT_BIT, 0,
        vsg::vec4Value::create(composite_params)));
    // Второй vec4 блока push-констант — со смещением 16
    state_group->addChild(vsg::PushConstants::create(
        VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(vsg::vec4),
        vsg::vec4Value::create(composite_params2)));
    state_group->addChild(vsg::Draw::create(3, 1, 0, 0));

    return state_group;
}

} // namespace graphics
