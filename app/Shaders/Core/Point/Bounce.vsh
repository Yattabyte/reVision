#version 460

layout(location = 0) in vec4 vertex;

layout (location = 0) out int In_InstanceID;

void main()
{
	In_InstanceID = gl_InstanceID;
}
