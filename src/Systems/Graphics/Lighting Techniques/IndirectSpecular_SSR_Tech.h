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

class EnginePackage;
class Geometry_Buffer;
class Lighting_Buffer;
class Callback_Container;
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
	IndirectSpecular_SSR_Tech(EnginePackage * enginePackage, Geometry_Buffer * gBuffer, Lighting_Buffer * lBuffer, VisualFX * visualFX);


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
	/** Binds the framebuffer and its render-targets for writing.
	* @param	bounceSpot		which bounce we are performing */
	void bindForWriting(const GLuint & bounceSpot);
	/** Binds the framebuffer and its render-targets for reading.
	* @param	bounceSpot		which bounce we are performing
	* @param	textureUnit		which texture unit we are going to start with (minimum GL_TEXTURE0) */
	void bindForReading(const GLuint & bounceSpot, const GLuint textureUnit);
	/** Binds the light buffer for reading and convolute's it into several MIPs, representing increasing roughness.*/
	void blurLight();
	/** Applies the SSR effect using the blurred light MIP chain*/
	void reflectLight();


	// Private Attributes
	Geometry_Buffer * m_gBuffer;
	Lighting_Buffer * m_lBuffer;
	EnginePackage * m_enginePackage;
	VisualFX * m_visualFX;
	Shared_Asset_Shader m_shaderCopy, m_shaderBlur, m_shaderSSR;
	Shared_Asset_Primitive m_shapeQuad;
	Shared_Asset_Texture m_brdfMap;
	GLuint m_quadVAO;
	void* m_QuadObserver;
	GLuint m_fbo, m_texture;
	vec2 m_renderSize;
	Callback_Container * m_widthChangeCallback, *m_heightChangeCallback;
	SSR_Buffer m_ssrBuffer;
	GLuint m_ssrUBO;
};

#endif // INDIRECT_LIGHTING_SSR