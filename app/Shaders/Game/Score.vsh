/* Score Shader. */
#version 460
#package "Game\GameBuffer"

layout (location = 0) in vec3 vertex;
layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out float NumberToRender;
layout (location = 2) flat out float HighlightAmount;
layout (location = 3) flat out uint UseBackdrop;

layout (location = 0) uniform mat4 orthoProj;
layout (location = 4) uniform uint scoreLength;


const float NUM_CHARS = 8.0f;

void main()
{
	// Starting Variables
	const uint modInstance = gl_InstanceID % scoreLength;
	UseBackdrop = 1u - uint(gl_InstanceID / scoreLength);	
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	NumberToRender = float(int(mod(score / pow(10, (NUM_CHARS - 1.0f) - (modInstance + (8u - scoreLength))), 10.0f)));
	
	// Amount of color to add to the highlighted digit (when score is added)
	HighlightAmount = 0.0f;
	if ((modInstance >= highlightIndex))
		HighlightAmount = 1.0f;		
		
	// This matrix stretches the unit row of blocks to the scale of 3
	const float tileSize = ((3.0f / NUM_CHARS) + (HighlightAmount * 0.025f)) * (128.0f - (10.0f * multiplier * scoreAnimLinear));
	const vec2 offsetMatrix = vec2(0.3, -0.1) * 0.65f * UseBackdrop;
	const mat4 scoreScaleMat = mat4(
		vec4(tileSize, 0.0, 0.0, 0.0),
		vec4(0.0, tileSize, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),
		vec4((vec2(50, -10) * (multiplier/10.0f)) + (vec2(125, -50) * scoreAnimLinear), 0.0, 1.0)
	);
	// This matrix centers the posiotion of the tiles withom the row
	const mat4 scoreTransMat = mat4(
		vec4(1.0, 0.0, 0.0, 0.0),
		vec4(0.0, 1.0, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),
		vec4(vec2(1.0F + (modInstance * 2.0F) - scoreLength, 0.0) + offsetMatrix, 0.0, 1.0)
	);
	gl_Position = orthoProj * scoreScaleMat * scoreTransMat * vec4(vertex.xy, 0, 1);
}