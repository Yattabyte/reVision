/* Intro Shader. */
#version 460
#package "Game\GameBuffer"

layout (location = 0) in vec3 vertex;
layout (location = 0) out vec2 TexCoord;


void main()
{	
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	float scl = 0.75f;
	float AR = 6.0f / 12.0f;
	if (intro.countDown < 3)
		scl /= 3.0f;	
	else if (intro.countDown == 3) {
		TexCoord.x *= 3.0f;
		AR /= 3.0f;
	}
	else 
		scl = 0.0f;
	const mat4 matrix = mat4(
		vec4(scl, 0.0, 0.0, 0.0),
		vec4(0.0, scl * AR, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),
		vec4(0.0, 0.0, 0.0, 1.0)
	);
	gl_Position = matrix * vec4(vertex.xy, 0, 1);
}