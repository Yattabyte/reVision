/* Screen space reflection shader - part 1 - UV Lookup Creation. */
#version 460

layout (std430, binding = 2) readonly coherent buffer Camera_Buffer {		
	mat4 pMatrix;
	mat4 vMatrix;
	vec3 EyePosition;
	vec2 CameraDimensions;
};

layout (location = 0) in vec3 vertex;

layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out mat4 CamPMatrix;
layout (location = 5) flat out mat4 CamVMatrix;
layout (location = 9) flat out mat4 CamPInverse;
layout (location = 13) flat out mat4 CamVInverse;
layout (location = 17) flat out vec3 CamEyePosition;
layout (location = 18) flat out vec2 CamDimensions;

void main(void)
{	
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;	
	CamPMatrix = pMatrix;
	CamVMatrix = vMatrix;
	CamPInverse = inverse(pMatrix);
	CamVInverse = inverse(vMatrix);
	CamEyePosition = EyePosition;
	CamDimensions = CameraDimensions;
	gl_Position = vec4(vertex.xyz, 1);
}

