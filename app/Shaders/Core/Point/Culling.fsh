/* Point light - geometry culling shader. */
#version 460
layout (early_fragment_tests) in;
layout (location = 0) flat in int id;

struct Draw_Struct {
	uint count;
	uint instanceCount;
	uint first;
	uint baseInstance;
};

layout (std430, binding = 7) writeonly coherent buffer Output_DrawBuffer {
	Draw_Struct o_buffers[];
};

void main()									
{				
	o_buffers[id].instanceCount = 6; // 6 layer-faces to render
}