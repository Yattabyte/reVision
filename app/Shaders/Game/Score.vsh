/* Score Shader. */
#version 460
#package "Game\GameBuffer"
#define M_PI 3.1415926535897932384626433832795

layout (location = 0) in vec3 vertex;
layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out float NumberToRender;
layout (location = 2) flat out float HighlightAmount;
layout (location = 3) flat out uint UseBackdrop;

layout (location = 0) uniform mat4 orthoProj;
layout (location = 4) uniform uint scoreLength;


const float NUM_CHARS = 8.0f;
const float SCORE_ROTATE_TICK = 750.0F;

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
	const float tileSize = ((3.0f / NUM_CHARS) + (HighlightAmount * 0.025f)) * 128.0f;
	const vec2 offsetMatrix = vec2(0.3, -0.1) * 0.65f * UseBackdrop;
	const mat4 scoreScaleMat = mat4(
		vec4(tileSize, 0.0, 0.0, 0.0),
		vec4(0.0, tileSize, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),
		vec4(0.0, 0.0, 0.0, 1.0)
	);
	// This matrix centers the posiotion of the tiles withom the row
	const mat4 scoreTransMat = mat4(
		vec4(1.0, 0.0, 0.0, 0.0),
		vec4(0.0, 1.0, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),
		vec4(vec2(1.0F + (modInstance * 2.0F) - scoreLength, 0.0) + offsetMatrix, 0.0, 1.0)
	);
	const float angle = sin((2.0f * (float(scoreTick) / SCORE_ROTATE_TICK) - 1.0f) * M_PI) * 0.0625F;
	const vec3 axis = vec3(0,0,1);
	const float s = sin(angle);
    const float c = cos(angle);
    const float oc = 1.0 - c;    
    const mat4 scoreRotMat = mat4(
		oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
		oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
        oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
        0.0,        0.0,                                0.0,                                1.0);	
	gl_Position = orthoProj * scoreScaleMat * scoreRotMat * scoreTransMat * vec4(vertex.xy, 0, 1);
}