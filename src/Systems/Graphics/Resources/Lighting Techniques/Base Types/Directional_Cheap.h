#pragma once
#ifndef DIRECTIONAL_CHEAP_TECH
#define DIRECTIONAL_CHEAP_TECH
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
#include "Utilities\GL\StaticBuffer.h"

/**
 * A deferred shading lighting technique that manages cheap directional lights
 **/
class DT_ENGINE_API Directional_Tech_Cheap : public Light_Tech {
public:
	// (de)Constructors
	/** Destructor. */
	~Directional_Tech_Cheap();
	/** Constructor. */
	Directional_Tech_Cheap(Light_Buffers * lightBuffers);


	// Interface Implementations
	virtual const char * getName() const { return "Directional_Tech_Cheap"; }
	virtual void updateData(const Visibility_Token & vis_token, const int & updateQuality, const vec3 & camPos);
	virtual void updateDataGI(const Visibility_Token & vis_token, const unsigned int & bounceResolution) {}
	virtual void renderOcclusionCulling() {}
	virtual void renderShadows() {}
	virtual void renderLightBounce() {}
	virtual void renderLighting();


private:
	// Private Attributes
	VectorBuffer<Directional_Cheap_Struct> * m_lightSSBO; 
	Shared_Asset_Shader m_shader_Lighting;
	Shared_Asset_Primitive m_shapeQuad;
	GLuint m_quadVAO;
	bool m_quadVAOLoaded;
	StaticBuffer m_indirectShape;
	size_t m_size;
};

#endif // DIRECTIONAL_CHEAP_TECH