#version 130 
         
in vec4 color; 
in vec2 texCoord;
out vec4 fragData;

uniform sampler2D tex;
 
void main() 
{ 
	vec4 texColor = texture2D(tex, texCoord);
    fragData = vec4(color.r * texColor.r, color.g * texColor.g, color.b * texColor.b, color.a * texColor.a);
};