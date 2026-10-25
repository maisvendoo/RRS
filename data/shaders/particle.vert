#version 450
#VSG_JIT_SHADER

in vec3 vsg_Vertex;
in vec3 vsg_Normal;
in vec2 vsg_TexCoord0;

out vec4 out_position;
out vec3 out_normal;
out vec2 out_uv;
out float out_alpha;

void main()
{
    // vsg_Vertex = position (vec3)
    // vsg_TexCoord0.x = size
    // vsg_TexCoord0.y = alpha
    // vsg_Normal.x = uv_x
    // vsg_Normal.y = uv_y

    vec4 worldPos = sceneData.worldMatrix * vec4(vsg_Vertex, 1.0);
    vec4 viewPos  = viewMatrix * worldPos;

    float px = vsg_TexCoord0.x;
    float py = vsg_TexCoord0.x;

    if (vsg_Normal.x < 0.5) px = -px;
    if (vsg_Normal.y < 0.5) py = -py;

    viewPos.x += px * viewProjection[0][0];
    viewPos.y += py * viewProjection[1][1];

    out_position = invViewMatrix * viewPos;
    out_normal   = vec3(0.0, 1.0, 0.0);
    out_uv       = vec2(vsg_Normal.x < 0.5 ? 0.0 : 1.0, vsg_Normal.y < 0.5 ? 0.0 : 1.0);
    out_alpha    = vsg_TexCoord0.y;
}