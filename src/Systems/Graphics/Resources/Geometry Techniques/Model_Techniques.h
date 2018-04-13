#pragma once
#ifndef MODEL_TECHNIQUE
#define MODEL_TECHNIQUE
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\Graphics\Resources\Geometry Techniques\Geometry_Technique.h"
#include "Systems\Graphics\Resources\GFX_DEFINES.h"
#include "Assets\Asset_Shader.h"
#include "Utilities\GL\DynamicBuffer.h"
#include "Utilities\GL\VectorBuffer.h"

class EnginePackage;
class Geometry_FBO; 
class Shadow_FBO;


/**
 * Renders models (animated or static props which support skeletons)
 **/
class DT_ENGINE_API Model_Technique : public Geometry_Technique {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~Model_Technique();
	/** Constructor. */
	Model_Technique(
		EnginePackage * enginePackage, Geometry_FBO * geometryFBO, Shadow_FBO * shadowFBO,
		VectorBuffer<Directional_Struct> * lightDirSSBO, VectorBuffer<Point_Struct> *lightPointSSBO
	);


	// Public Interface Implementations
	virtual void updateData(const Visibility_Token & vis_token);
	virtual void renderGeometry(const Visibility_Token & vis_token);
	virtual void renderShadows(const Visibility_Token & vis_token);


private:
	// Private Attributes
	int m_updateQuality;
	EnginePackage * m_enginePackage;
	// Shared FBO's
	Geometry_FBO * m_geometryFBO;
	Shadow_FBO * m_shadowFBO;
	// Shared SSBO's
	VectorBuffer<Directional_Struct> * m_lightDirSSBO;
	VectorBuffer<Point_Struct> * m_lightPointSSBO;
	DynamicBuffer m_visGeoUBO, m_indirectGeo;
	Shared_Asset_Shader m_shaderGeometry, m_shaderDirectional_Shadow, m_shaderPoint_Shadow, m_shaderSpot_Shadow;
};

#endif // MODEL_TECHNIQUE