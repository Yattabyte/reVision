/* Point light - (indirect) light bounce shader. */
#version 460

layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 6) out;

// X = Layer count, Y = Light Count
layout (location = 0) uniform int LightCount = 0;

layout (location = 0) in int In_InstanceID[];
layout (location = 0) flat out int BufferIndex;

void main()
{
    const int Slice = In_InstanceID[0] / LightCount;
	BufferIndex = In_InstanceID[0] % LightCount;
    vec4 v1 = vec4(1,-1, 0, 1);
    vec4 v2 = vec4(-1,-1, 0, 1);
    vec4 v3 = vec4(-1,1, 0, 1);
    vec4 v4 = vec4(1,1, 0, 1);    
    
    gl_Position = v1;
    gl_Layer = Slice;
    EmitVertex();  
    
    gl_Position = v2;
    gl_Layer = Slice;
    EmitVertex();    
    
    gl_Position = v3;
    gl_Layer = Slice;
    EmitVertex();    
    
    gl_Position = v1;
    gl_Layer = Slice;
    EmitVertex();  
    
    gl_Position = v4;
    gl_Layer = Slice;
    EmitVertex();  
    
    gl_Position = v3;
    gl_Layer = Slice;
    EmitVertex();  
    EndPrimitive();	
}

