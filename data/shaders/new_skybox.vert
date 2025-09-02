#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants {
    mat4 projection;
    mat4 modelview;
} pc;

layout(location = 0) in vec3 vsg_Vertex;
layout(location = 1) in vec2 vsg_TexCoord0;

layout(location = 0) out vec2 fragTexCoord;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = (pc.projection * pc.modelview) * vec4(vsg_Vertex, 1.0);
    fragTexCoord = vsg_TexCoord0;
}
