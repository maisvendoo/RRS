#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform FragmentPushConstants {
    uint r;
    uint g;
    uint b;
} pc_frag;

void main() {
    outColor = vec4(pc_frag.r / 255.0, pc_frag.g / 255.0, pc_frag.b / 255.0, 1.0);
}
