#version 450
#VSG_JIT_SHADER

in vec2 out_uv;
in float out_alpha;

layout(set = 0, binding = 0) uniform sampler2D particleTexture;

out vec4 out_color;

void main()
{
    vec4 tex = texture(particleTexture, out_uv);
    out_color = vec4(tex.rgb * out_alpha, tex.a * out_alpha);
}