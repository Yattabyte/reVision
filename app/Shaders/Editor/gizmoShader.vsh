/* Gizmo shader. */
#version 460

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 texcoord;
layout (location = 0) uniform mat4 projMat;
layout (location = 4) uniform mat4 modelMat;

layout (location = 0) out vec2 UV;

void main()
{	
	UV = texcoord;
	gl_Position = projMat * modelMat * vec4(vertex, 1.0);	
}

