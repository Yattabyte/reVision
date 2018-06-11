#version 460
#package "camera"
#define MAX_BONES 100 

struct Geometry_Struct {
	bool useBones;
	uint materialID;
	mat4 mMatrix;
	mat4 bBoxMatrix;
	mat4 Bones[MAX_BONES];
};

layout (std430, binding = 3) readonly buffer Visibility_Buffer {
	uint indexes[];
};
layout (std430, binding = 4) readonly buffer Geometry_Buffer {
	Geometry_Struct buffers[];
};

layout (location = 0) in vec3 vertex;
layout (location = 0) flat out int id;

void main()
{	
	gl_Position = pMatrix * vMatrix * buffers[indexes[gl_DrawID]].bBoxMatrix * vec4(vertex,1.0);		
	id = gl_DrawID;
}