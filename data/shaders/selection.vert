#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;

out gl_PerVertex {
    vec4 gl_Position;
};

layout(push_constant) uniform VertexPushConstants {
    mat4 projection;
    mat4 modelView;
} pc_vert;

void main() {
    gl_Position = pc_vert.projection * pc_vert.ModelView * vec4(inPosition,
        1.0);
}
