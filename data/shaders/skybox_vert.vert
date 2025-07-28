#version 450
#extension GL_ARB_separate_shader_objects : enable
#pragma import_defines (VSG_INSTANCE_TRANSLATION, VSG_INSTANCE_ROTATION, VSG_INSTANCE_SCALE)


layout(push_constant) uniform PushConstants {
    mat4 projection;
    mat4 modelView;
} pc;


layout(location = 0) in vec3 vsg_Vertex;
layout(location = 2) in vec2 vsg_TexCoord0;

#if defined(VSG_INSTANCE_TRANSLATION)
layout(location = 7) in vec3 vsg_Translation;
#endif

#if defined(VSG_INSTANCE_ROTATION)
layout(location = 8) in vec4 vsg_Rotation;

vec3 rotate(vec4 q, vec3 v)
{
    vec3 uv, uuv;
    vec3 qvec = vec3(q[0], q[1], q[2]);
    uv = cross(qvec, v);
    uuv = cross(qvec, uv);
    uv *= (2.0 * q[3]);
    uuv *= 2.0;
    return v + uv + uuv;
}
#endif

#if defined(VSG_INSTANCE_SCALE)
layout(location = 9) in vec3 vsg_Scale;
#endif


layout(location = 0) out vec2 texCoord;

out gl_PerVertex{
    vec4 gl_Position;
};


void main()
{
    vec4 vertex = vec4(vsg_Vertex, 1.0);

#ifdef VSG_INSTANCE_SCALE
    vertex.xyz = vertex.xyz * vsg_Scale;
#endif

#ifdef VSG_INSTANCE_ROTATION
    vertex.xyz = rotate(vsg_Rotation, vertex.xyz);
#endif

#ifdef VSG_INSTANCE_TRANSLATION
    vertex.xyz = vertex.xyz + vsg_Translation;
#endif

    mat4 mv = pc.modelView;

    // Remove translation
    mv[3] = vec4(0.0, 0.0, 0.0, 1.0);

    gl_Position = (pc.projection * mv) * vertex;

    texCoord = vsg_TexCoord0;
}
