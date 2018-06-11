#version 460 
#package "camera"

layout (triangles) in;
layout (triangle_strip, max_vertices = 108) out;

layout (location = 2) uniform mat4 View[6];
layout (location = 8) uniform mat4 Proj;

layout (location = 0) in vec3 vinput[];
layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out int Read_Index;

void main()
{				
	// Strip the translation from the matrix
	mat4 InvRotVMatrix = vMatrix_Inverse;
	InvRotVMatrix[3][0] = 0;
	InvRotVMatrix[3][1] = 0;
	InvRotVMatrix[3][2] = 0;
	
	const mat4 InvMat = InvRotVMatrix * pMatrix_Inverse;
	
	// Cycle through each texture-read level
	for (int x = 0; x < 6; ++x) {
		Read_Index = x;		
		// Cycle through each viewing direction
		for (int y = 0; y < 6; ++y) {
			const mat4 ProjView = Proj * View[y];
			// Cycle though each of the 3 vertices
			for (int z = 0; z < 3; ++z) {
				const vec3 vertex = vinput[z];
				const vec4 WorldPos = (InvMat * vec4(vertex.xyz, 1));
				gl_Position = ProjView * WorldPos;
				gl_Layer = (x * 6) + y; // has to be done each vertex
				TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
				EmitVertex();  
			}	
			EndPrimitive();			
		}
	}
}

