/* Background Shader. */
#version 460
#package "Game\GameBuffer"

layout (location = 0) in vec3 vertex;
layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out float LinearTop;
layout (location = 2) flat out float Time;
layout (location = 3) flat out float Speed;
layout (location = 4) flat out float Count;
layout (location = 5) flat out vec4 Offsets;

void main()
{	
	TexCoord = ((vertex.xy + vec2(1.0)) / 2.0 - 0.5F) * vec2(CameraDimensions.x/CameraDimensions.y, 1.0f) * 1.5f;
	gl_Position = vec4(vertex.xy, 0, 1);

	LinearTop = nearingTop * nearingTop;
	Time = (sysTime / 2.0f) + (1.0f + LinearTop);
	Speed = 0.02f + (LinearTop / 20.0f);
	Count = 2.0f + (8.0f * LinearTop);
	Offsets = vec4(
		5.0f + sin(Time) * 10.0f * (0.25 + LinearTop),
		2.5f + cos(Time*1.5f) * 5.0f,
		(0.5f * sin(Time*2.0f) + 0.5) * 2.0f,
		(0.5f * cos(Time/1.5f) + 0.5) 
	);
}