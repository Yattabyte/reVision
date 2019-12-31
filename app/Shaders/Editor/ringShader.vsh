/* Ring shader for the rotation gizmo. */
#version 460

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 texcoord;
layout (location = 2) in uint meshID;
layout (location = 3) in vec3 normal;

layout (location = 0) uniform mat4 pvmMat_3;
layout (location = 4) uniform mat4 pvmMat_Single;

layout (location = 0) out vec2 UV;
layout (location = 1) flat out uint axisID;
layout (location = 2) out vec3 N;
layout (location = 3) flat out mat3 usedMat;


void main()
{	
	UV = texcoord;
	axisID = meshID;
	N = normal;
	mat4 desiredMat = pvmMat_3;
	if (meshID == 3u)
		desiredMat = pvmMat_Single;
	usedMat = mat3(desiredMat);
	gl_Position = desiredMat * vec4(vertex, 1.0);
}