#version 130 

uniform mat4 osg_ModelViewProjectionMatrix; 
uniform mat3 osg_NormalMatrix; 
uniform vec3 ecLightDir; 
 
in vec4 osg_Vertex; 
in vec3 osg_Normal; 
in vec4 osg_MultiTexCoord0;
out vec4 color; 
out vec2 texCoord;
out float diffuse;
 
void main() 
{ 
    vec3 ecNormal = normalize( osg_NormalMatrix * osg_Normal ); 
    diffuse = max( dot( ecLightDir, osg_Normal ), 0.0 ); 
    color = vec4( vec3( diffuse ), 1.0 ); 

    texCoord = osg_MultiTexCoord0.st;
 
    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex; 
};