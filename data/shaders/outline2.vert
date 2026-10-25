#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants {
    mat4 projection;
    mat4 modelView;
} pc;

layout(location = 0) in vec3 vsg_Vertex;
layout(location = 1) in vec3 vsg_Normal;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = pc.projection * pc.modelView * vec4(vsg_Vertex + vsg_Normal * 0.025, 1.0);
}
