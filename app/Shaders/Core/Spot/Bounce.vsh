/* Spot light - (indirect) light bounce shader. */
#version 460
#extension GL_ARB_shader_viewport_layer_array : enable

layout(location = 0) in vec3 vertex;

layout (location = 0) flat out int BufferIndex;

layout (location = 0) uniform int LightCount = 0;

void main()
{
	// Draw order arranged like: (layer 0)[light 0, light 1, light 2], (layer 1)[light 0, light 1, light 2], etc
    gl_Position = vec4(vertex, 1);
    gl_Layer = gl_InstanceID / LightCount;
	BufferIndex = gl_InstanceID % LightCount;
}
