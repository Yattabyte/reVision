#version 460
#extension GL_ARB_shader_viewport_layer_array : require

layout(location = 0) in vec3 vertex;

void main()
{
    gl_Position = vec4(vertex, 1);
	gl_Layer = gl_InstanceID;
}
