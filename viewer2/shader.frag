#version 450
layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;
layout(binding = 0) uniform sampler2D texSampler;
layout(push_constant) uniform PushColor {
    layout(offset = 128) vec4 color; // Offset matches MVP size (2 mat4 = 128 bytes)
} uColor;
void main()
{
    outColor = texture(texSampler, fragTexCoord);
}
