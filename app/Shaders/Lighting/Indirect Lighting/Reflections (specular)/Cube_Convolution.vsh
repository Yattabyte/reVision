#version 460
#extension GL_ARB_shader_viewport_layer_array : require

// Uniform Inputs
layout (location = 0) uniform int cubeIndex = 0;

// Inputs
layout (location = 0) in vec3 vertex;
layout (location = 0) out vec2 TexCoord;

// Outputs
layout (location = 1) flat out int cubeFace;

void main(void)
{	
	TexCoord = (vertex.xy);
	gl_Position = vec4(vertex, 1.0f);	
	gl_Layer = (cubeIndex * 6) + gl_InstanceID;
	cubeFace = gl_InstanceID;
}

