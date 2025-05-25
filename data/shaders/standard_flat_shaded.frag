#version 450
#extension GL_ARB_separate_shader_objects : enable
#pragma import_defines (VSG_POINT_SPRITE, VSG_DIFFUSE_MAP, VSG_GREYSCALE_DIFFUSE_MAP, VSG_DETAIL_MAP, VSG_ALPHA_TEST)

#define VIEW_DESCRIPTOR_SET 0
#define MATERIAL_DESCRIPTOR_SET 1

#ifdef VSG_DIFFUSE_MAP
layout(set = MATERIAL_DESCRIPTOR_SET, binding = 0) uniform sampler2D diffuseMap;
#endif

#ifdef VSG_DETAIL_MAP
layout(set = MATERIAL_DESCRIPTOR_SET, binding = 1) uniform sampler2D detailMap;
#endif

layout(set = MATERIAL_DESCRIPTOR_SET, binding = 10) uniform MaterialData
{
    vec4 ambientColor;
    vec4 diffuseColor;
    vec4 specularColor;
    vec4 emissiveColor;
    float shininess;
    float alphaMask;
    float alphaMaskCutoff;
} material;

layout(location = 2) in vec4 vertexColor;
#ifndef VSG_POINT_SPRITE
layout(location = 3) in vec2 texCoord0;
#endif

layout(location = 0) out vec4 outColor;

void main()
{
#ifdef VSG_POINT_SPRITE
    vec2 texCoord0 = gl_PointCoord.xy;
#endif

    vec4 diffuseColor = vertexColor * material.diffuseColor;

#ifdef VSG_DIFFUSE_MAP
    #ifdef VSG_GREYSCALE_DIFFUSE_MAP
        float v = texture(diffuseMap, texCoord0.st).s;
        diffuseColor *= vec4(v, v, v, 1.0);
    #else
        diffuseColor *= texture(diffuseMap, texCoord0.st);
    #endif
#endif

#ifdef VSG_DETAIL_MAP
    vec4 detailColor = texture(detailMap, texCoord0.st);
    diffuseColor.rgb = mix(diffuseColor.rgb, detailColor.rgb, detailColor.a);
#endif


#ifdef VSG_ALPHA_TEST
    if (material.alphaMask == 1.0f && diffuseColor.a < material.alphaMaskCutoff) discard;
#endif

    outColor = diffuseColor;
}
