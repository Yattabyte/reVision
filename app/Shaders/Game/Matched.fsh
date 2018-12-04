/* Matched Tiles Shader. */
#version 460
#package "Game\GameBuffer"

layout (location = 0) in vec2 TexCoord;
layout (location = 0) out vec4 FragColor;
layout (binding = 0) uniform sampler2D piecesTexture;


void main()
{	
	const float pulseAmount = calcPulseAmount(gl_FragCoord.y);
	const vec3 boardColor = (colorScheme * pulseAmount) * (colorScheme * pulseAmount) * (colorScheme / M_PI);
	FragColor = texture(piecesTexture, TexCoord);
	FragColor.xyz *= boardColor;
	
	// Color the background
	if (FragColor.x + FragColor.y + FragColor.z < 0.1f) 
		FragColor.xyz += boardColor * 0.125f;
}
