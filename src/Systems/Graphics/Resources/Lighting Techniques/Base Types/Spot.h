#pragma once
#ifndef SPOT_TECH
#define SPOT_TECH
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\Graphics\Resources\Lighting Techniques\Base Types\Light_Tech.h"
#include "Systems\Graphics\Resources\Light_Buffers.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\DynamicBuffer.h"
#include "Utilities\GL\VectorBuffer.h"
#include "Utilities\GL\StaticBuffer.h"
#include <deque>


class EnginePackage;

/**
* An interface for specific deferred shading lighting techniques.
* To be used only by the Graphics class and its supporting techniques.
**/
class DT_ENGINE_API Spot_Tech : public Light_Tech {
public:
	// (de)Constructors
	/** Destructor. */
	~Spot_Tech();
	/** Constructor. */
	Spot_Tech(EnginePackage * enginePackage, Light_Buffers * lightBuffers);


	/** Get the size of the shadows used by this light type.
	 * @return				the shadowmap size */
	vec2 getSize() const;
	/** Register a new shadow caster to this category of lights. 
	 * @param	array_spot	updated index into the shadowmap array */
	void registerShadowCaster(int & array_spot);
	/** Remove a shadow caster from this category of lights.
	 * @param	array_spot	the index to remove from the shadowmap array */
	void unregisterShadowCaster(int & array_spot);
	/** Clear the shadows from this shadowmap starting at the layer specified.
	 * @param	layer		the layer to begin clearing at */
	void clearShadow(const int & layer);


	// Interface Implementations
	virtual const char * getName() const { return "Spot_Tech"; }
	virtual void updateData(const Visibility_Token & vis_token, const int & updateQuality, const vec3 & camPos);
	virtual void updateDataGI(const Visibility_Token & vis_token, const unsigned int & bounceResolution);
	virtual void renderOcclusionCulling();
	virtual void renderShadows();
	virtual void renderLightBounce();
	virtual void renderLighting();


private:
	// Private Functions
	/** Set the shadowmap size.
	 * @param	size	the size to set the shadowmaps for this category of lights */
	void setSize(const float & size);
	/** Bind textures required for Global Illumination.
	 * @param	ShaderTextureUnit	the texture unit to begin binding at */
	void BindForReading_GI(const GLuint & ShaderTextureUnit);


	// Private Attributes
	EnginePackage * m_enginePackage;
	Shared_Asset_Shader m_shader_Lighting, m_shader_Cull, m_shader_Shadow, m_shader_Bounce;
	Shared_Asset_Primitive m_shapeCone;
	GLuint m_coneVAO;
	bool m_coneVAOLoaded;
	VectorBuffer<Spot_Struct> * m_lightSSBO;
	DynamicBuffer m_visShapes;
	StaticBuffer m_indirectShape;
	vector<Lighting_Component*> m_queue;
	size_t m_size;


	// Shadows
	vec2 m_shadowSize;
	GLuint m_shadowFBO, m_shadowDepth, m_shadowWNormal, m_shadowRFlux;
	GLuint m_shadowCount;
	deque<unsigned int>	m_freedShadowSpots;


	// Bounces
	size_t m_sizeGI;
	StaticBuffer m_indirectBounce;
	DynamicBuffer m_visSpots;
};

#endif // SPOT_TECH