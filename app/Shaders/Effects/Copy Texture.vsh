/* Copies an image from binding 0, to location 0. */
#version 460
#extension GL_ARB_shader_viewport_layer_array : enable
#package "CameraBuffer"

layout(location = 0) in vec3 vertex;

layout (location = 0) out vec2 TexCoord;

void main()
{		
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	gl_Position = vec4(vertex.xyz, 1);	
	gl_Layer = camIndexes[gl_InstanceID].y;
}