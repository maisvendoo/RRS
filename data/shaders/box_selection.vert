#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inPosition;

out gl_PerVertex {
    vec4 gl_Position;
};

layout(binding = 0) uniform Transform
{
    vec2 translation;
    vec2 scale;
} transform;

void main() {
    gl_Position = vec4(inPosition * scale + translation, 0.0, 1.0);
}
