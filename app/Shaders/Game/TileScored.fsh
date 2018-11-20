/* Tile-Scored Shader. */
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

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in int RenderNumber;
layout (location = 2) flat in int NumberToRender;
layout (location = 3) flat in float TileLife;
layout (location = 0) out vec4 FragColor;
layout (binding = 0) uniform sampler2D TileTexture;
layout (binding = 1) uniform sampler2D Numbers;


void main()
{	
	if (RenderNumber > 0) {
		const ivec2 Size = textureSize(Numbers, 0);
		const float AtlasWidth = float(Size.x);
		const float ElementWidth = float(Size.y);
		const float ElementCount = 11.0f;	
		const vec2 DigitIndex = vec2((TexCoord.x / ElementCount) + ((NumberToRender * ElementWidth) / AtlasWidth), TexCoord.y);
		FragColor = texture(Numbers, DigitIndex);
	}
	else {
		const float waveAmt = 0.5f * sin((-length(gl_FragCoord.y / CameraDimensions.y) * (2.0f + (excitement * 8.0))  ) + (2.0f * (float(gameTick) / 750.0) - 1.0f) * 3.1415f * (2.0f + (excitement * 8.0))) + 0.5f;
		const float pulseAmount = 1.0f - (0.75 * (1.0f - ((1.0f - waveAmt) * (1.0f - waveAmt))));		
		const vec3 boardColor = mix(vec3(0,0.5,1), vec3(1,0,0.5), excitement);
		FragColor = texture(TileTexture, TexCoord) * vec4( boardColor * pulseAmount, 1 );
	}
}
