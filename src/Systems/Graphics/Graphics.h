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

#include "Systems\System_Interface.h"
#include "Systems\Graphics\Resources\Frame Buffers\Geometry_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Lighting_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Shadow_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Reflection_FBO.h"
#include "Systems\Graphics\Resources\Geometry Techniques\Model_Techniques.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Lighting_Technique.h"
#include "Systems\Graphics\FX Techniques\FX_Technique.h"
#include "Systems\Graphics\Resources\Light_Buffers.h"
#include "Systems\Graphics\Resources\VisualFX.h"
#include "Systems\World\Visibility_Token.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Utilities\GL\VectorBuffer.h"
#include "Utilities\GL\DynamicBuffer.h"
#include <vector>

class EnginePackage;
class Camera;


/**
 * An engine system responsible for rendering. Creates Geometry_FBO, Lighting_FBO, Shadow_FBO, and VisualFX
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


	// Public Attributes
	// Frame Buffers
	Geometry_FBO	m_geometryFBO;
	Lighting_FBO	m_lightingFBO;
	Shadow_FBO		m_shadowFBO;
	Reflection_FBO	m_reflectionFBO;
	// Storage Buffers
	Light_Buffers	m_lightBuffers;
	VectorBuffer<Geometry_Struct>		m_geometrySSBO;
	VectorBuffer<Reflection_Struct>		m_reflectionSSBO;
	

private:
	// Private Methods
	/** Regenerate the noise kernel. */
	void generateKernal();
	/** Sends data to GPU in one pass.
	 * For example, sending updated mat4's into buffers. */
	void send2GPU(const Visibility_Token & vis_token);
	/** Perform pre-passes and update data present on the GPU. 
	 * For example, performing GPU accelerated occlusion culling or shadow mapping. */
	void updateOnGPU(const Visibility_Token & vis_token);
	/** Render a single frame. */
	void renderFrame(const Visibility_Token & vis_token);


	// Private Attributes
	ivec2			m_renderSize;
	VisualFX		m_visualFX;
	StaticBuffer	m_userBuffer;

	// Rendering Techniques
	vector<Geometry_Technique*> m_geometryTechs;
	vector<Lighting_Technique*> m_lightingTechs;
	vector<FX_Technique*>		m_fxTechs;
};

#endif // SYSTEM_GRAPHICS