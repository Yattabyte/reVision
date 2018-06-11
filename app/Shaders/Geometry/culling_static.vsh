#version 460
#package "camera"

struct Geometry_Struct {
	uint materialID;
	mat4 mMatrix;
	mat4 bBoxMatrix;
};

layout (std430, binding = 3) readonly buffer Visibility_Buffer {
	uint indexes[];
};
layout (std430, binding = 5) readonly buffer Geometry_Buffer {
	Geometry_Struct buffers[];
};

layout (location = 0) in vec3 vertex;
layout (location = 0) flat out int id;

void main()
{	
	gl_Position = pMatrix * vMatrix * buffers[indexes[gl_DrawID]].bBoxMatrix * vec4(vertex,1.0);		
	id = gl_DrawID;
}