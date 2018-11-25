/* Combo Shader. */
#version 460
#package "Game\GameBuffer"

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
layout (location = 1) flat out float NumberToRender;
layout (location = 2) flat out float QuadIndex;
layout (location = 3) flat out float HighlightIndex;


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
	
	NumberToRender = -1.0f;
	const int decimalPlaces[8] = int[](10000000,1000000,100000,10000,1000,100,10,1 );
		for (int x = 0; x < 8; ++x) 
			if (score >= decimalPlaces[x]) {
				if (gl_InstanceID >= x) 
					NumberToRender = float(int(mod(score / pow(10, (NUM_CHARS - 1) - gl_InstanceID), 10.0f)));				
				break;
			}			
	QuadIndex = float(gl_InstanceID);
	HighlightIndex = float(highlightIndex);
}