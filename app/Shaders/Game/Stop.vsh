/* Stop-Timer Shader. */
#version 460
#package "Game\GameBuffer"

layout (location = 0) in vec3 vertex;
layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out int CharToRender;
layout (location = 0) uniform mat4 orthoProj;


const float NUM_CHARS = 8.0f;

void main()
{	
	// Constants
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	const float tileSize = (3.0f / NUM_CHARS) * 75.0f;
	
	// Render Text
	if (gl_InstanceID == 0) {
		TexCoord.x *= 0.5f;
		if (stopTimer >= 0)
			TexCoord.x += 0.5f;
		CharToRender = -1;
		// This matrix centers the position of the text within the row
		const mat4 transMat = mat4(
			vec4(1.0, 0.0, 0.0, 0.0),
			vec4(0.0, 1.0, 0.0, 0.0),
			vec4(0.0, 0.0, 1.0, 0.0),
			vec4(-1.0, 0.0, 0.0, 1.0)
		);
		// This matrix stretches the unit row of blocks to the footer-width
		const mat4 scaleMat = mat4(
			vec4(tileSize * 4.0f, 0.0, 0.0, 0.0),
			vec4(0.0, tileSize, 0.0, 0.0),
			vec4(0.0, 0.0, 1.0, 0.0),
			vec4(0.0, 0.0, 0.0, 1.0)
		);
		gl_Position = scaleMat * transMat * vec4(vertex.xy, 0, 1);	
	}
	
	// Render Numbers
	else {
		const int timeToUse = stopTimer < 0 ? gameTimer : stopTimer;
		const int timeInMinutes = timeToUse / 60;
		const int timeInSeconds = timeToUse % 60;
		bool isSemiColon = false;
		bool isLeftOfColon = false;
		if (gl_InstanceID < 3) {
			isLeftOfColon = true;	
			CharToRender = int(mod( timeInMinutes / pow(10.0f, 2 - gl_InstanceID), 10.0F ));
		}
		else if (gl_InstanceID == 3) {
			CharToRender = 10;
			isSemiColon = true;
		}
		else 
			CharToRender = int(mod( timeInSeconds / pow(10.0f, 5 - gl_InstanceID), 10.0F ));		
		
		// This matrix centers the position of the tiles withim the row
		const mat4 transMat = mat4(
			vec4(1.0, 0.0, 0.0, 0.0),
			vec4(0.0, 1.0, 0.0, 0.0),
			vec4(0.0, 0.0, 1.0, 0.0),
			vec4(vec2((((gl_InstanceID % 6) * 2.0F)) + (isLeftOfColon ? 0.725f : isSemiColon ? 0 : -0.725f) - 1, 0.0), 0.0, 1.0)
		);	
		// This matrix stretches the unit row of blocks to the footer-width
		const mat4 scaleMat = mat4(
			vec4(tileSize, 0.0, 0.0, 0.0),
			vec4(0.0, tileSize, 0.0, 0.0),
			vec4(0.0, 0.0, 1.0, 0.0),
			vec4(0.0, 0.0, 0.0, 1.0)
		);
		gl_Position = scaleMat * transMat * vec4(vertex.xy, 0, 1);
	}
	
	// Project
	gl_Position = orthoProj * gl_Position;
}
