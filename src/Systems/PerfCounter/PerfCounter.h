#pragma once
#ifndef SYSTEM_PERFCOUNTER
#define SYSTEM_PERFCOUNTER
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\System_Interface.h"
#include "Assets\Asset_Texture.h"
#include "Assets\Asset_Primitive.h"
#include "Assets\Asset_Shader.h"
#include "Utilities\GL\StaticBuffer.h"

class EnginePackage;


/**
* An engine system responsible for rendering. Creates Geometry_FBO, Lighting_FBO, Shadow_FBO, and VisualFX
* @note	performs physically based rendering techniques.
**/
class DT_ENGINE_API System_PerfCounter : public System
{
public:
	// (de)Constructors
	/** Destroy the rendering system. */
	~System_PerfCounter();
	/** Construct the rendering system. */
	System_PerfCounter();


	// Interface Implementations
	virtual void initialize(EnginePackage * enginePackage);
	virtual void update(const float & deltaTime);
	virtual void updateThreaded(const float & deltaTime) {};


private:
	// Private Methods
	void resize(const ivec2 s);


	// Private Attributes
	Shared_Asset_Texture m_numberTexture;
	Shared_Asset_Primitive m_shapeQuad;
	Shared_Asset_Shader m_shader;
	GLuint m_quadVAO;	
	bool m_quadVAOLoaded;
	StaticBuffer m_indirectQuad;
	ivec2 m_renderSize;
	mat4 m_projMatrix;
};

#endif // SYSTEM_PERFCOUNTER