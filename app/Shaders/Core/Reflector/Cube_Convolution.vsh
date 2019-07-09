/* Reflector - cubemap gaussian blurring shader. */
#version 460
#extension GL_ARB_shader_viewport_layer_array : enable
#package "CameraBuffer"

// Inputs
layout (location = 0) in vec3 vertex;

// Outputs
layout (location = 0) out vec3 TexCoord;
layout (location = 1) flat out int cubeOffset;

void main(void)
{	
	const vec2 UV = (vertex.xy);
	gl_Position = vec4(vertex, 1.0f);	
	gl_Layer = camIndexes[gl_InstanceID].y;
	const vec3 faces[6] = vec3[]( 
		vec3( 1, -UV.y, -UV.x),
		vec3( -1, -UV.y, UV.x),
		vec3( UV.x, 1, UV.y),
		vec3( UV.x, -1, -UV.y),
		vec3( UV.x, -UV.y, 1),
		vec3(-UV.x, -UV.y, -1) 
	);
	TexCoord = normalize(faces[gl_InstanceID % 6]);
	cubeOffset = int(gl_InstanceID / 6) * 6;
}

