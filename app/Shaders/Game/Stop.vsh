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


const float NUM_CHARS = 8.0f;

void main()
{	
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	const float tileSize = (3.0f / NUM_CHARS);
	bool isSemiColon = false;
	bool isLeftOfColon = false;
	if (gl_InstanceID < 2) {
		isLeftOfColon = true;	
		CharToRender = int(mod( stopTimer / pow(10.0f, 3.0F - gl_InstanceID), 10.0F ));
	}
	else if (gl_InstanceID == 2) {
		CharToRender = 10;
		isSemiColon = true;
	}
	else 
		CharToRender = int(mod( stopTimer / pow(10.0f, 3.0F - (gl_InstanceID-1.0f)), 10.0F ));
	
	// This matrix stretches the unit row of blocks to the scale of 3
	const mat4 scaleMat = mat4(
		vec4(tileSize, 0.0, 0.0, 0.0),
		vec4(0.0, tileSize, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),
		vec4(0.0, -6.75, 0.0, 1.0)
	);
	// This matrix centers the posiotion of the tiles withom the row
	const mat4 transMat = mat4(
		vec4(1.0, 0.0, 0.0, 0.0),
		vec4(0.0, 1.0, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),
		vec4(vec2((1.0F + ((gl_InstanceID % 5) * 2.0F) - 5.0F) + (isLeftOfColon ? 0.725f : isSemiColon ? 0 : -0.725f), 0.0), 0.0, 1.0)
	);
	gl_Position = pMatrix * vMatrix * scaleMat * transMat * vec4(vertex.xy, -10, 1);
}
