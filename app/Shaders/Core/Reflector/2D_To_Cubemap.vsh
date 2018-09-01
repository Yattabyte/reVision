/* Reflector - 2D-image to cubemap-face shader. */
#version 460
#extension GL_ARB_shader_viewport_layer_array : require

layout (location = 0) in vec3 vertex;
layout (location = 0) out vec2 TexCoord;

layout (location = 0) uniform int LayerFace = 0;

void main()
{	
	TexCoord = (vertex.xy) * 0.5f + 0.5f;
	gl_Position = vec4(vertex, 1.0);	
	gl_Layer = LayerFace;
}