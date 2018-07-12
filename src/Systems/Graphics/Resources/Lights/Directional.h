#pragma once
#ifndef DIRECTIONAL_TECH_H
#define DIRECTIONAL_TECH_H

#include "Systems\Graphics\Resources\Lights\Light_Tech.h"
#include "Systems\Graphics\Resources\Light_Buffers.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\VectorBuffer.h"
#include "Utilities\GL\StaticBuffer.h"
#include <deque>


class Engine;

/**
 * A deferred shading lighting technique that manages directional lights.
 */
class Directional_Tech : public Light_Tech {
public:
	// (de)Constructors
	/** Destructor. */
	~Directional_Tech();
	/** Constructor. */
	Directional_Tech(Engine * engine, Light_Buffers * lightBuffers);


	/** Get the size of the shadows used by this light type.
	 * @return				the shadowmap size */
	glm::vec2 getSize() const;
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
	virtual const char * getName() const { return "Directional_Tech"; }
	virtual void updateData(const Visibility_Token & vis_token, const int & updateQuality, const glm::vec3 & camPos);
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


	// Private Attributes
	Engine * m_engine;
	Shared_Asset_Shader m_shader_Lighting, m_shader_CullDynamic, m_shader_CullStatic, m_shader_ShadowDynamic, m_shader_ShadowStatic, m_shader_Bounce;
	Shared_Asset_Primitive m_shapeQuad;
	GLuint m_quadVAO;
	bool m_quadVAOLoaded;
	VectorBuffer<Directional_Struct> * m_lightSSBO; 
	StaticBuffer m_indirectShape;
	std::vector<Lighting_Component*> m_queue;
	size_t m_size;


	// Shadows
	glm::vec2 m_shadowSize;
	GLuint m_shadowFBO, m_shadowDepth, m_shadowWNormal, m_shadowRFlux;
	GLuint m_shadowCount;
	std::deque<unsigned int>	m_freedShadowSpots;


	// Bounces
	size_t m_sizeGI;
	StaticBuffer m_indirectBounce;
};

#endif // DIRECTIONAL_TECH_H