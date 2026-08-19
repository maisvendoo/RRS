#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D prev_texture;
layout(binding = 1) uniform sampler2D curr_texture;
layout(binding = 2) uniform MixValue
{
    float value;
} mix_value;

// Туман (ТЗ "Видимость и погода"): rgb - цвет, a - плотность, 1/м
layout(binding = 3) uniform FogValue
{
    vec4 value;
} fog_value;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = mix(texture(prev_texture, fragTexCoord), texture(curr_texture, fragTexCoord), mix_value.value);

    // Затухание градиента неба в цвет тумана: экспоненциальная муть
    // от плотности, усиленная у горизонта. UV сферы: полюса по краям
    // координаты v, экватор (горизонт) - в середине, поэтому градиент
    // не зависит от ориентации текстуры
    float density = max(fog_value.value.a, 0.0);
    float haze = 1.0 - exp(-density * density * 4.0e6);
    haze = clamp(haze, 0.0, 0.9);

    float horizon = 1.0 - abs(fragTexCoord.y * 2.0 - 1.0);
    float fog_amount = haze * (0.35 + 0.65 * horizon);

    outColor.rgb = mix(outColor.rgb, fog_value.value.rgb, fog_amount);
    outColor.a = 1.0;
}
