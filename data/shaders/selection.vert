#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;

out gl_PerVertex {
    vec4 gl_Position;
};

layout(push_constant) uniform PushConstants {
    mat4 projection;
    mat4 modelView;
} pc;

void main() {
    gl_Position = pc.projection * pc.modelView * vec4(inPosition, 1.0);
}
