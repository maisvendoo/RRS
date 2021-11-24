#version 130 
         
in vec4 color; 
in vec2 texCoord;
in float diffuse;
out vec4 fragData;

uniform sampler2D tex;
uniform float lightAmbient;
 
void main() 
{ 
	vec4 texColor = texture2D(tex, texCoord);
    
    vec3 diff = texColor.rgb * diffuse;
    vec3 amb = texColor.rgb * lightAmbient;

    fragData = clamp(vec4(diff + amb, texColor.a), 0.0, 1.0);
}