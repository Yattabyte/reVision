/* Background Shader. */
#version 460
#package "Game\GameBuffer"

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in float LinearTop;
layout (location = 2) flat in float Time;
layout (location = 3) flat in float Speed;
layout (location = 4) flat in float Count;
layout (location = 5) flat in vec4 Offsets;
layout (location = 0) out vec3 FragColor;


vec2 rotateUV(vec2 uv, float rotation)
{
    return vec2(
        cos(rotation) * uv.x - sin(rotation) * uv.y,
        cos(rotation) * uv.y + sin(rotation) * uv.x
    );
}

void main()
{
	// Spin the uv around the center, based on the time
	const vec2 uv = rotateUV(TexCoord, (sysTime / 2.0f));
	const float uvLength =  length(uv);
	const float x = pow(uvLength, Speed), y = atan(uv.x, uv.y) / 3.14f;

	// Generate the cells
	float c = 1.0f;
	for (float i = 0.0f; i < Count; ++i)    
		c = min(c, length(fract(vec2(x - Time * i * 0.015f , fract(y + i * 0.15) * 0.15) * 40.0f) * 2.0f - 1.0f));

	// Modify the color
	vec3 f = (vec3(
		uvLength + Offsets[0] * c * uvLength * (1 + uvLength), 
		uvLength + Offsets[1] * c * uvLength * (1 - uvLength), 
		uvLength + Offsets[2] * c * uvLength * (1 + uvLength)
	) * uvLength + Offsets[3] * c * uvLength * (1 - uvLength)) * colorScheme;
	f = f * f * f * f * colorScheme;
	FragColor = ((f / 20.0f) * (10.0f * LinearTop));
}