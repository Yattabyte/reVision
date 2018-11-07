/* Score Shader. */
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
	float lifeTick[12*6];
	mat4 boardMat;
	float heightOffset;
	float excitement;
	int score;
	int highlightIndex;
	int stopTimer;
	mat4 playerMat;
};

layout (location = 0) in vec3 vertex;
layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out float NumberToRender;
layout (location = 2) flat out float HighlightAmount;
layout (location = 3) flat out uint UseBackdrop;

layout (location = 0) uniform uint scoreLength;


const float NUM_CHARS = 8.0f;

void main()
{
	const float tileSize = 3.0f / NUM_CHARS;
	UseBackdrop = 1u - uint(gl_InstanceID/scoreLength);
	const vec2 offsetMatrix = vec2(0.3, -0.1) * 0.65f * UseBackdrop;
	const uint modInstance = gl_InstanceID % scoreLength;
	// This matrix stretches the unit row of blocks to the scale of 3
	const mat4 scoreScaleMat = mat4(
		vec4(tileSize, 0.0, 0.0, 0.0),
		vec4(0.0, 1.0, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),
		vec4(0.0, 0.0, 0.0, 1.0)
	);
	// This matrix positions the tiles within the top row, centered.
	const mat4 scoreTransMat = mat4(
		vec4(1.0, 0.0, 0.0, 0.0),
		vec4(0.0, tileSize, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),
		vec4(vec2(1.0F + (modInstance * 2.0F) - scoreLength, 5.5) + offsetMatrix, 0.0, 1.0)
	);
	gl_Position = pMatrix * vMatrix * scoreScaleMat * scoreTransMat * vec4(vertex.xy, -10, 1);
	
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	NumberToRender = float(int(mod(score / pow(10, (NUM_CHARS - 1.0f) - (modInstance + (8u - scoreLength))), 10.0f)));
	
	if ((modInstance >= highlightIndex))
		HighlightAmount = 1.0f;
	else
		HighlightAmount = 0.0f;
}