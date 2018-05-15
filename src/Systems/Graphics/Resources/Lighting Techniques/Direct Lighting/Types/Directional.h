#pragma once
#ifndef DIRECTIONAL_TECH
#define DIRECTIONAL_TECH
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\Graphics\Resources\Lighting Techniques\Direct Lighting\Types\DS_Technique.h"
#include "Systems\Graphics\Resources\Light_Buffers.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\VectorBuffer.h"
#include "Utilities\GL\StaticBuffer.h"

class Shadow_FBO;

/**
* An interface for specific deferred shading lighting techniques.
* To be used only by the DS_Lighting class.
**/
class DT_ENGINE_API Directional_Tech : public DS_Technique {
public:
	// (de)Constructors
	/** Destructor. */
	~Directional_Tech();
	/** Constructor. */
	Directional_Tech(Shadow_FBO * shadowFBO, Light_Buffers * lightBuffers);


	// Interface Implementations
	virtual void updateData(const Visibility_Token & vis_token, const int & updateQuality, const vec3 & camPos);
	virtual void renderOcclusionCulling();
	virtual void renderShadows();
	virtual void renderLighting();


private:
	// Private Attributes
	Shared_Asset_Shader m_shader_Lighting, m_shader_Cull, m_shader_Shadow;
	Shared_Asset_Primitive m_shapeQuad;
	GLuint m_quadVAO;
	bool m_quadVAOLoaded;
	Shadow_FBO * m_shadowFBO;
	VectorBuffer<Directional_Struct> * m_lightSSBO; 
	StaticBuffer m_indirectShape;
	vector<Lighting_Component*> m_queue;
	size_t m_size;
};

#endif // DIRECTIONAL_TECH