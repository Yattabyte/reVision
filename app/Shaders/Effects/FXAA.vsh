/* FXAA shader - applies FXAA to the input image. */
#version 460
#extension GL_ARB_shader_viewport_layer_array : enable
#package "CameraBuffer"

layout(location = 0) in vec3 vertex;

layout (location = 0) out vec2 TexOffset;
layout (location = 1) out vec2 TexCoord;


void main()
{		
	const int CamIndex = camIndexes[gl_InstanceID].x;
	TexOffset = 1.0f / camBuffer[CamIndex].CameraDimensions;
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	gl_Position = vec4(vertex.xyz, 1);	
	gl_Layer = camIndexes[gl_InstanceID].y;
}