/* Matched Tiles Shader. */
#version 460
#package "Game\GameBuffer"

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in int Type;
layout (location = 0) out vec4 FragColor;
layout (binding = 0) uniform sampler2D piecesTexture;


void main()
{	
	FragColor = texture(piecesTexture, TexCoord);
	FragColor.xyz *= colorScheme * calcPulseAmount(gl_FragCoord.y);
	
	if (FragColor.x + FragColor.y + FragColor.z < 0.1f) 
		FragColor.xyz += colorScheme * calcPulseAmount(gl_FragCoord.y) * 0.125f;
}
