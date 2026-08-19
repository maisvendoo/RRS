#version 450
#extension GL_ARB_separate_shader_objects : enable
#pragma import_defines (VSG_DIFFUSE_MAP)


#define MATERIAL_DESCRIPTOR_SET 1
#ifdef VSG_DIFFUSE_MAP
layout(set = MATERIAL_DESCRIPTOR_SET, binding = 0) uniform sampler2D diffuseMap;
#endif


layout(location = 0) in vec2 texCoord;


layout(location = 0) out vec4 outColor;


void main()
{
#ifdef VSG_DIFFUSE_MAP
    outColor = texture(diffuseMap, texCoord);
#else
    outColor = vec4(0.0, 0.0, 0.0, 1.0);
#endif
}
