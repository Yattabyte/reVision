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

layout (location = 0) in vec3 vertex;
layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out float Index;

layout (location = 0) uniform int Score = 0;


const int NUM_CHARS = 8;

void main()
{	
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	const mat4 scoreTransMat = mat4(
		vec4(3.0f / 8.0f, 0.0, 0.0, 0.0),
		vec4(0.0, 0.5, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),
		vec4((2.0f * (gl_InstanceID / (NUM_CHARS - 1.0f)) - 1.0f) * 2.625f, 5.5, 0.0, 1.0)
	);
	gl_Position = pMatrix * vMatrix * scoreTransMat * vec4(vertex.xy, -10, 1);
	
	int firstMostDigit = NUM_CHARS - 1;
	for (int x = 0; x < NUM_CHARS - 1; ++x) 
		if (int(mod(Score / pow(10, (NUM_CHARS-1)-x), 10.0f)) != 0) {
			firstMostDigit = x;
			break;
		}
	if (gl_InstanceID >= firstMostDigit)
		Index = float(int(mod(Score / pow(10, (NUM_CHARS - 1) - gl_InstanceID), 10.0f)));
	else
		Index = -1.0f;
}