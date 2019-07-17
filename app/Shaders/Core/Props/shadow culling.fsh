/* Prop - Geometry culling shader for shadow maps. */
#version 460

struct Draw_Struct {
	uint count;
	uint instanceCount;
	uint first;
	uint baseInstance;
};

layout (early_fragment_tests) in;
layout (std430, binding = 8) writeonly coherent buffer Output_DrawBuffer {
	Draw_Struct o_buffers[];
};

layout (location = 0) flat in int id;


void main()									
{				
	o_buffers[id].instanceCount = 1;
}