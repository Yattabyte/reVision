#version 460
#package "camera"

layout(location = 0) in vec3 vertex;

struct Light_Struct {
	mat4 mMatrix;
	vec4 LightColor;
	vec4 LightPosition;
	float LightIntensity;
	float LightRadius;
};

layout (std430, binding = 3) readonly buffer Visibility_Buffer {
	uint indexes[];
};

layout (std430, binding = 6) readonly buffer Light_Buffer {
	Light_Struct buffers[];
};

flat out uint BufferIndex;

void main()
{		
	BufferIndex = gl_InstanceID;
	gl_Position = pMatrix * vMatrix * buffers[indexes[BufferIndex]].mMatrix * vec4(vertex, 1.0); 
}

