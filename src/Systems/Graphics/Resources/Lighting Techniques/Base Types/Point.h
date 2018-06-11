#pragma once
#ifndef POINT_TECH_H
#define POINT_TECH_H

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
 * A deferred shading lighting technique that manages point lights.
 **/
class Point_Tech : public Light_Tech {
public:
	// (de)Constructors
	/** Destructor. */
	~Point_Tech();
	/** Constructor. */
	Point_Tech(EnginePackage * enginePackage, Light_Buffers * lightBuffers);

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
	 * @param	type		the type of shadow to clear (dynamic or static (0-1))
	 * @param	layer		the layer to begin clearing at */
	void clearShadow(const unsigned int & type, const int & layer);


	// Interface Implementations
	virtual const char * getName() const { return "Point_Tech"; }
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


	// Private Attributes
	EnginePackage * m_enginePackage;
	Shared_Asset_Shader m_shader_Lighting, m_shader_CullDynamic, m_shader_CullStatic, m_shader_ShadowDynamic, m_shader_ShadowStatic;
	Shared_Asset_Primitive m_shapeSphere;
	GLuint m_sphereVAO;
	bool m_sphereVAOLoaded;
	VectorBuffer<Point_Struct> * m_lightSSBO;
	DynamicBuffer m_visShapes;
	StaticBuffer m_indirectShape;
	vector<Lighting_Component*> m_lightList, m_queue;
	size_t m_size;


	// Shadows
	vec2 m_shadowSize;
	GLuint m_shadowFBO[2], m_shadowDepth[2], m_shadowDistance[2], m_shadowWNormal[2], m_shadowRFlux[2];
	GLuint m_shadowCount;
	deque<unsigned int>	m_freedShadowSpots;
	bool m_regenSShadows;
};

#endif // POINT_TECH_H