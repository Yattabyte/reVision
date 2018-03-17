#pragma once
#ifndef SYSTEM_GRAPHICS
#define SYSTEM_GRAPHICS
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define MAX_KERNEL_SIZE 128 // Don't manipulate this, set the usable to a different value < than this

#include "Systems\System_Interface.h"
#include "Systems\Graphics\Frame Buffers\Geometry_Buffer.h"
#include "Systems\Graphics\Frame Buffers\Lighting_Buffer.h"
#include "Systems\Graphics\Frame Buffers\Shadow_Buffer.h"
#include "Systems\Graphics\Frame Buffers\Reflection_Buffer.h"
#include "Systems\Graphics\Lighting Techniques\Lighting_Technique.h"
#include "Systems\Graphics\FX Techniques\FX_Technique.h"
#include "Systems\Graphics\VisualFX.h"
#include "Systems\World\Visibility_Token.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Assets\Asset_Cubemap.h"
#include <vector>

class EnginePackage;
class Camera;


/**
 * An engine system responsible for rendering. Creates Geometry_Buffer, Lighting_Buffer, Shadow_Buffer, and VisualFX
 * @note	performs physically based rendering techniques.
 **/
class DT_ENGINE_API System_Graphics : public System
{
public: 
	// (de)Constructors
	/** Destroy the rendering system. */
	~System_Graphics();
	/** Construct the rendering system. */
	System_Graphics();
	

	// Interface Implementations
	virtual void initialize(EnginePackage * enginePackage);
	virtual void update(const float & deltaTime);
	virtual void updateThreaded(const float & deltaTime) {};


	// Public Methods
	/** Enable or Disable screen-space ambient occlusion.
	 * @param	ssao		true or false */
	void setSSAO(const bool & ssao);
	/** Change the number of samples used in screen-space ambient occlusion. 
	 * @param	samples		the number of samples to use */
	void setSSAOSamples(const int & samples);
	/** Change the strength of the screen-space ambient occlusion. 
	 * @param	strength	the strength to use */
	void setSSAOStrength(const int & strength);
	/** Change the radius of the screen-space ambient occlusion.
	 * @param	radius		the radius to use*/
	void setSSAORadius(const float & radius);
	/** Change the size of framebuffers used.
	 * @param	size		the new size to use */
	void resize(const ivec2 & size);	
	/** Change the shadow update quality  used.
	* @param	size		the new size to use */
	void setShadowUpdateQuality(const float & quality);
	/** Retrieve the shadow buffer. 
	 * @return				the shadow map buffer */
	Shadow_Buffer & getShadowBuffer();

	
private:
	/** Nested buffer class. */
	struct Renderer_Attribs {
		vec4 kernel[MAX_KERNEL_SIZE];
		float m_ssao_radius;
		int m_ssao_strength, m_aa_samples;
		int m_ssao;
	};


	// Public Methods
	/** Regenerate the noise kernel. */
	void generateKernal();
	/** Regenerates shadowmap's. 
	 * @param	vis_token	the visible objects in this frame */
	void shadowPass(const Visibility_Token & vis_token);
	/** Fills the gBuffer by rendering all visible geometric objects.
	 * @param	vis_token	the visible objects in this frame */
	void geometryPass(const Visibility_Token & vis_token);
	/** Renders the sky to the lighting buffer. */
	void skyPass();		


	// Public Attributes
	Renderer_Attribs m_attribs;
	GLuint m_attribID;
	ivec2 m_renderSize;
	VisualFX m_visualFX;
	Geometry_Buffer m_gBuffer;
	Lighting_Buffer m_lBuffer;
	Shadow_Buffer m_shadowBuffer;
	Reflection_Buffer m_refBuffer;
	Shared_Asset_Shader	m_shaderDirectional_Shadow, m_shaderPoint_Shadow, m_shaderSpot_Shadow,						
						m_shaderGeometry, m_shaderSky;
	Shared_Asset_Primitive m_shapeQuad;
	GLuint m_quadVAO;
	Shared_Asset_Cubemap m_textureSky;
	int m_updateQuality;

	// Rendering Techniques
	vector<Lighting_Technique*> m_lightingTechs;
	vector<FX_Technique*> m_fxTechs;
};

#endif // SYSTEM_GRAPHICS