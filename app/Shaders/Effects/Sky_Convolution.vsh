/* Reflector - cubemap gaussian blurring shader. */
#version 460
#extension GL_ARB_shader_viewport_layer_array : enable

// Inputs
layout (location = 0) in vec3 vertex;
layout (location = 0) out vec2 TexCoord;

// Outputs
layout (location = 1) flat out int cubeFace;

void main(void)
{	
	TexCoord = (vertex.xy);
	gl_Position = vec4(vertex, 1.0f);	
	gl_Layer = gl_InstanceID;
	cubeFace = gl_InstanceID;
}

