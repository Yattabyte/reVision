/* Board Shader. */
#version 460

layout (std430, binding = 2) readonly coherent buffer Camera_Buffer {		
	mat4 pMatrix;
	mat4 vMatrix;
	mat4 pMatrix_Inverse;
	mat4 vMatrix_Inverse;
	vec3 EyePosition;
	vec2 CameraDimensions;
};
layout (std430, binding = 8) readonly buffer BoardBuffer {		
	mat4 tileMats[12*6];
	uint types[12*6];
	mat4 boardMat;
	uint tick;
};

layout (location = 0) in vec3 vertex;
layout (location = 0) out vec2 TexCoord;

void main()
{	
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	gl_Position = pMatrix * vMatrix * boardMat * vec4(vertex.xy, 0, 1);
}