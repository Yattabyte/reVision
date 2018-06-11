#version 460
#package "camera"

layout(location = 0) in vec4 vertex;

struct Light_Struct {
	mat4 mMatrix;
	mat4 lightV;
	mat4 lightPV;
	mat4 InverseLightPV;
	vec4 LightColor;
	vec4 LightPosition;
	vec4 LightVector;
	float ShadowSize_Recip;
	float LightIntensity;
	float LightRadius;
	float LightCutoff;
	int Shadow_Spot;
};

layout (std430, binding = 3) readonly buffer Visibility_Buffer {
	uint indexes[];
};

layout (std430, binding = 6) readonly buffer Light_Buffer {
	Light_Struct buffers[];
};

layout (location = 0) flat out uint BufferIndex;

void main()
{		
	BufferIndex = gl_InstanceID;
	gl_Position = pMatrix * vMatrix * buffers[indexes[BufferIndex]].mMatrix * vertex;
}

