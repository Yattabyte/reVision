#pragma once
#ifndef INDIRECT_LIGHTING_SSR
#define INDIRECT_LIGHTING_SSR
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define BRDF_SIZE 512

#include "Systems\Graphics\Lighting Techniques\Lighting_Technique.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Assets\Asset_Texture.h"
#include "Assets\Asset_Cubemap.h"

class EnginePackage;
class Geometry_Buffer;
class Lighting_Buffer;
class Reflection_Buffer;
class VisualFX;


/**
 * A lighting technique that calculates indirect specular lighting contribution, aka light reflections off of other objects, using the final lighting buffer. Uses PBR techniques.
 **/
class DT_ENGINE_API IndirectSpecular_SSR_Tech : public Lighting_Technique {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~IndirectSpecular_SSR_Tech();
	/** Constructor. */
	IndirectSpecular_SSR_Tech(EnginePackage * enginePackage, Geometry_Buffer * gBuffer, Lighting_Buffer * lBuffer, Reflection_Buffer * refBuffer, VisualFX * visualFX);


	// Interface Implementations.
	virtual void updateLighting(const Visibility_Token & vis_token);
	virtual void applyLighting(const Visibility_Token & vis_token);	


	// Public Methods
	/** Resize the frame buffer by the amount specified.
	 * @param	size	the amount to resize by*/
	void resize(const vec2 &size);

private:
	/** Nested buffer object struct for sending data to GPU */
	struct SSR_Buffer {
		int maxSteps = 16;
		float maxDistance = 50.0f;
		float numMips = 6;
		float fadeStart = 0.40;
		float fadeEnd = 0.9f;
	};


	// Private Methods	
	/** Binds the light buffer for reading and convolute's it into several MIPs, representing increasing roughness.*/
	void blurLight();
	/** Adds to the global env map using the blurred light. */
	void buildEnvMap();
	/** Applies the SSR effect using the blurred light MIP chain*/
	void reflectLight();


	// Private Attributes
	Geometry_Buffer * m_gBuffer;
	Lighting_Buffer * m_lBuffer;
	Reflection_Buffer * m_refBuffer;
	EnginePackage * m_enginePackage;
	VisualFX * m_visualFX;
	Shared_Asset_Shader m_shaderCopy, m_shaderBlur, m_shaderSSR, TEMP_SHADER, TEMP_CUBE_SHADER;
	Shared_Asset_Primitive m_shapeQuad;
	Shared_Asset_Texture m_brdfMap;
	Shared_Asset_Cubemap TEMP_SKY;
	GLuint m_quadVAO;
	GLuint m_fbo, m_texture;
	vec2 m_renderSize;
	SSR_Buffer m_ssrBuffer;
	GLuint m_ssrUBO;

	mat4 views[6], proj;
	GLuint m_cube_fbo, m_cube_tex;
};

#endif // INDIRECT_LIGHTING_SSR