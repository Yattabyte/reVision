/* Calculates skybox reflections. */
#version 460

layout (std430, binding = 2) readonly coherent buffer Camera_Buffer {		
	mat4 pMatrix;
	mat4 vMatrix;
	vec3 EyePosition;
	vec2 CameraDimensions;
};

layout (location = 0) in vec3 vertex;

layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out mat4 CamPInverse;
layout (location = 5) flat out mat4 CamVInverse;

void main()
{	
	TexCoord = (vertex.xy) * 0.5f + 0.5f;
	CamPInverse = inverse(pMatrix);
	CamVInverse = inverse(vMatrix);
	gl_Position = vec4(vertex,1.0);	
}