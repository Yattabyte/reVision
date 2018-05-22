#pragma once
#ifndef POINT_TECH
#define POINT_TECH
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
* To be used only by the DS_Lighting class.
**/
class DT_ENGINE_API Point_Tech : public Light_Tech {
public:
	// (de)Constructors
	/** Destructor. */
	~Point_Tech();
	/** Constructor. */
	Point_Tech(EnginePackage * enginePackage, Light_Buffers * lightBuffers);

	/***/
	vec2 getSize() const;
	/***/
	void registerShadowCaster(int & array_spot);
	/***/
	void unregisterShadowCaster(int & array_spot);
	/***/
	void clearShadow(const int & layer);


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
	/***/
	void setSize(const float & size);


	// Private Attributes
	EnginePackage * m_enginePackage;
	Shared_Asset_Shader m_shader_Lighting, m_shader_Cull, m_shader_Shadow;
	Shared_Asset_Primitive m_shapeSphere;
	GLuint m_sphereVAO;
	bool m_sphereVAOLoaded;
	VectorBuffer<Point_Struct> * m_lightSSBO;
	DynamicBuffer m_visShapes;
	StaticBuffer m_indirectShape;
	vector<Lighting_Component*> m_queue;
	size_t m_size;


	// Shadows
	vec2 m_shadowSize;
	GLuint m_shadowFBO, m_shadowDepth, m_shadowDistance, m_shadowWNormal, m_shadowRFlux;
	GLuint m_shadowCount;
	deque<unsigned int>	m_freedShadowSpots;
};

#endif // POINT_TECH