#version 460
#package "camera"

layout (location = 0) in vec3 vertex;

layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out uint BufferIndex;

void main()
{	
	BufferIndex = gl_InstanceID;	
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	gl_Position = vec4(vertex, 1);
}