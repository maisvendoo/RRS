#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants {
    mat4 projection;
    mat4 modelView;
} pc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    // Remove translation
    mat4 modelView = pc.modelView;
    modelView[3] = vec4(0.0, 0.0, 0.0, 1.0);

    vec4 pos = pc.projection * modelView * vec4(inPosition, 1.0);
    gl_Position = vec4(pos.xy, 0.0, pos.w);

    fragTexCoord = inTexCoord;
}
