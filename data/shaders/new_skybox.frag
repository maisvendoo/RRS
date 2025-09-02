#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D prev_texture;
layout(binding = 1) uniform sampler2D curr_texture;
layout(binding = 2) uniform MixValue
{
    float value;
} mix_value;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = mix(texture(prev_texture, fragTexCoord), texture(curr_texture, fragTexCoord), mix_value.value);
}
