/* Wireframe shader. */
#version 460
#extension GL_ARB_shader_viewport_layer_array : enable

layout (location = 0) in vec3 vertex;

layout (std430, binding = 3) readonly buffer Matrix_Buffer {
	mat4 matrices[];
};


void main()
{	
	gl_Position = matrices[gl_BaseInstance + gl_InstanceID] * vec4(vertex, 1.0);	
}

