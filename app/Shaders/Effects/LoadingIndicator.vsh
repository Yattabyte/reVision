/* Displays a spinning circle indicating that something is loading. */
#version 460 

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 texcoord;

layout (location = 1) uniform mat4 projMat;
layout (location = 2) uniform mat4 modelMat;
layout (location = 3) uniform float time;

layout (location = 0) out vec2 TexCoord;


void main()
{
	const float angle = (time + ((0.5 * sin(time) + 0.5) * 1.0f)) * 10.0f;
	mat2 rotMat = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
	TexCoord = texcoord;
	gl_Position = projMat * modelMat * vec4(rotMat*vertex.xy, vertex.z, 1.0);
}

