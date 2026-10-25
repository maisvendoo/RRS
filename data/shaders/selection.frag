#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform Color
{
    vec3 value;
} color;

void main() {
    outColor = vec4(color.value, 1.0);
}
