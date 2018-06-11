#version 460
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader5 : require 
#extension GL_ARB_gpu_shader_int64 : require

layout (binding = 3) uniform sampler2DArray map;

layout (std430, binding = 0) readonly buffer Material_Buffer
{		
	uint64_t MaterialMaps[];
};

layout (location = 0) out vec4 fragColor;

in vec2 TexCoord;
void main()
{		
	//fragColor = vec4(texture(sampler2DArray(MaterialMaps[2]), vec3(TexCoord, 0)));	
	fragColor = vec4(texture(map, vec3(TexCoord, 0)));
}
