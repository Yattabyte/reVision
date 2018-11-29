/* Multiplier Shader. */
#version 460
#package "Game\GameBuffer"

layout (location = 0) in vec3 vertex;
layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out float NumberToRender;
layout (location = 2) flat out uint UseBackdrop;

layout (location = 0) uniform mat4 orthoProj;


void main()
{
	// Starting Variables	
	const uint modInstance = gl_InstanceID % 2;
	UseBackdrop = 1u - uint(gl_InstanceID / 2);	
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	NumberToRender =  modInstance == 0 ? 11 : float(int(mod(multiplier / pow(10, 0), 10.0f)));
	const float tileSize = min(75.0f, 15.0f + (5.0f * multiplier));
	const vec2 offsetMatrix = vec2(0.1, -0.1) * UseBackdrop;
	const mat4 scoreScaleMat = mat4(
		vec4(tileSize, 0.0, 0.0, 0.0),
		vec4(0.0, tileSize, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),
		vec4(0.0, 0.0, 0.0, 1.0)
	);
	const float angle = sin(gameWave * M_PI) * 0.25f;
	const vec3 axis = vec3(0,0,1);
	const float s = sin(angle);
    const float c = cos(angle);
    const float oc = 1.0 - c;    
    const mat4 scoreRotMat = mat4(
		oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
		oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
        oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
        0.0,        0.0,                                0.0,                                1.0);	
	
	gl_Position = orthoProj * scoreScaleMat * (vec4(-1.5,0,0,0) + scoreRotMat * (vec4(vec2(-1 + (modInstance * 2.0F), 0.5) + offsetMatrix, 0.0, 0.0) + vec4(vertex.xy, 0, 1)));
}