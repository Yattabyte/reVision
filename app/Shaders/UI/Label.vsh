/* UI Label Shader. */
#version 460

// Inputs
layout (location = 0) in vec3 vertex;

// Uniforms
layout (location = 0) uniform vec3 ElementTransform;
layout (location = 1) uniform vec2 Scale;
layout (location = 2) uniform float TextScale;
layout (location = 3) uniform int Alignment;

// Buffers
layout (std430, binding = 2) readonly coherent buffer ProjectionBuffer { 
	mat4 ScreenProjection;
};
layout (std430, binding = 8) readonly coherent buffer TextBuffer {
	int count;
	int characters[];
};

// Outputs
layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out int Character;


void main()
{
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	const float totalWidth = Scale.x * 2.0f;
	const float textWidth = TextScale * count;
	const float diff = totalWidth - textWidth;
	const mat4 transformMat = mat4(
		vec4(TextScale, 0.0, 0.0, 0.0),
		vec4(0.0, TextScale, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),		
		vec4(ElementTransform.x + (gl_InstanceID * TextScale) - ((TextScale * count) / 2.0f) + TextScale + (Alignment * (diff / 2.0f)), ElementTransform.y, ElementTransform.z, 1.0)
	);
	Character = characters[gl_InstanceID];
	gl_Position = ScreenProjection * transformMat * vec4(vertex.xy, 0, 1);
}