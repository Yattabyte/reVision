/* Wireframe shader. */
#version 460

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;

layout (location = 0) uniform float offset;
layout (location = 1) uniform vec3 camPosition;
layout (location = 2) uniform mat4 invPMatrix;

layout (std430, binding = 3) readonly buffer Matrix_Buffer {
	mat4 matrices[];
};


void main()
{	
	vec4 worldPos = invPMatrix * matrices[gl_DrawID] * vec4(vertex, 1.0);
	worldPos /= worldPos.w;
	const float dist = distance(worldPos.xyz, camPosition);	
	gl_Position = matrices[gl_DrawID] * vec4(vertex + (normal * dist * offset), 1.0);	
}