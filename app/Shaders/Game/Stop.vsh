/* Stop-Timer Shader. */
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
layout(location = 1) flat out int CharToRender;

void main()
{	
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	bool isSemiColon = false;
	bool isLeftOfColon = false;
	if (gl_InstanceID < 2) {
		CharToRender = 0;
		isLeftOfColon = true;
	}
	else if (gl_InstanceID == 2) {
		CharToRender = 10;
		isSemiColon = true;
	}
	else
		CharToRender = stopTimer;

	const mat4 mat = mat4(
		vec4(0.25f, 0.0, 0.0, 0.0),
		vec4(0.0, 0.3, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),
		vec4((2.0f * (gl_InstanceID / 4.0f) - 1.0f) + (isLeftOfColon ? 0.25 : !isSemiColon ? -0.25 : 0), -7.5, 0.0, 1.0)
	);
	gl_Position = pMatrix * vMatrix * mat * vec4(vertex.xy, -10, 1);
}
