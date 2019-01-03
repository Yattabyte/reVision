/* Board Shader. */
#version 460
#package "Game\GameBuffer"

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 4) in vec2 textureCoordinate;
layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out int Index;
layout (location = 2) out float Dot;


vec2 shake() {
	const float amount = sin( 1000.0f * sysTime * M_PI) * shakeLinear;
	const float xAmt = cos( 1000.0f * sysTime * 1.5 * M_PI) * amount;
	const float yAmt = sin( 1000.0f * sysTime * 0.5f * M_PI) * amount;
	return vec2(xAmt, yAmt) / 5.0f * amount;
}

void main()
{	
	TexCoord = textureCoordinate;
	Index = gl_DrawID;
	Dot = dot(vec3(0,0,1), normal);
	gl_Position = pMatrix * vMatrix * vec4(vertex.xyz - vec3(shake(), 10), 1);
}