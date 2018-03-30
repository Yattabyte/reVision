#pragma once
#ifndef REFLECTIONS
#define REFLECTIONS
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define BRDF_SIZE 512

#include "Systems\Graphics\Resources\Lighting Techniques\Lighting_Technique.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Assets\Asset_Texture.h"
#include "Assets\Asset_Cubemap.h"
#include "Utilities\GL\MappedBuffer.h"

class Geometry_FBO;
class Lighting_FBO;
class Reflection_FBO;
class Reflection_UBO;


/**
 * Performs a reflection pass using the screen as input - using the screen space reflection technique.
 * Also caches the viewport over time into a persistent cubemap, used as a fallback environment map.
 * Lastly, supports parallax corrected local cubemaps.
 * Responsible for indirect specular lighting.
 * Supports physically based shaders.
 **/
class DT_ENGINE_API Reflections : public Lighting_Technique {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~Reflections();
	/** Constructor. */
	Reflections(EnginePackage * enginePackage, Geometry_FBO * geometryFBO, Lighting_FBO * lightingFBO, Reflection_FBO * reflectionFBO);


	// Interface Implementations.
	virtual void updateLighting(const Visibility_Token & vis_token);
	virtual void applyLighting(const Visibility_Token & vis_token);	


	// Public Methods
	/** Resize the frame buffer by the amount specified.
	 * @param	size	the amount to resize by */
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
	void reflectLight(const Visibility_Token & vis_token);


	// Private Attributes
	Geometry_FBO * m_geometryFBO;
	Lighting_FBO * m_lightingFBO;
	Reflection_FBO * m_reflectionFBO;
	Reflection_UBO * m_reflectionUBO;
	EnginePackage * m_enginePackage;
	Shared_Asset_Shader m_shaderCopy, m_shaderBlur, m_shaderSSR, m_shaderCubemap, m_shaderCubeProj, m_shaderFinal, m_shaderParallax;
	Shared_Asset_Primitive m_shapeQuad, m_shapeCube;
	Shared_Asset_Texture m_brdfMap;
	GLuint m_quadVAO, m_cubeVAO;
	GLuint m_fbo, m_texture;
	vec2 m_renderSize;
	GLuint m_cube_fbo, m_cube_tex;
	MappedBuffer m_cubeIndirectBuffer, m_ssrBuffer;
};

#endif // REFLECTIONS