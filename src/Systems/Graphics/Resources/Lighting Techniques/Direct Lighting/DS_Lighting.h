#pragma once
#ifndef DS_LIGHTING
#define DS_LIGHTING
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\Graphics\Resources\Lighting Techniques\Lighting_Technique.h"
#include "Systems\Graphics\Resources\GFX_DEFINES.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Utilities\GL\DynamicBuffer.h"
#include "Utilities\GL\VectorBuffer.h"

class EnginePackage;
class Geometry_FBO;
class Lighting_FBO;
class Shadow_FBO;
class Lighting_Component;


/**
 * Performs basic lighting calculation using deferred shading.
 * Responsible for entire direct portion of the lighting equation: direct diffuse and dirrect specular.
 * Supports physically based shaders.
 * Supports directional, point, and spot lights.
 **/
class DT_ENGINE_API DS_Lighting : public Lighting_Technique {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~DS_Lighting();
	/** Constructor. */
	DS_Lighting(
		EnginePackage * enginePackage,
		Geometry_FBO * geometryFBO, Lighting_FBO * lightingFBO, Shadow_FBO *shadowFBO, 
		VectorBuffer<Directional_Struct> * lightDirSSBO, VectorBuffer<Point_Struct> *lightPointSSBO, VectorBuffer<Spot_Struct> *lightSpotSSBO
	);


	// Interface Implementations.
	virtual void updateData(const Visibility_Token & vis_token);
	virtual void applyPrePass(const Visibility_Token & vis_token);
	virtual void applyLighting(const Visibility_Token & vis_token);	


private:
	// Private Attributes
	EnginePackage * m_enginePackage;
	int m_updateQuality;
	// Shared FBO's
	Geometry_FBO * m_geometryFBO;
	Lighting_FBO * m_lightingFBO;
	Shadow_FBO * m_shadowFBO;
	// Shared SSBO's
	VectorBuffer<Directional_Struct> * m_lightDirSSBO;
	VectorBuffer<Point_Struct> * m_lightPointSSBO;
	VectorBuffer<Spot_Struct> * m_lightSpotSSBO;

	Shared_Asset_Shader m_shaderDirectional, m_shaderPoint, m_shaderSpot,
						m_shaderDirectional_Shadow, m_shaderPoint_Shadow, m_shaderSpot_Shadow;
	Shared_Asset_Primitive m_shapeQuad, m_shapeCone, m_shapeSphere;
	GLuint m_quadVAO, m_coneVAO, m_sphereVAO;
	bool m_quadVAOLoaded, m_coneVAOLoaded, m_sphereVAOLoaded; 
	StaticBuffer m_indirectDir, m_indirectPoint, m_indirectSpot;
	DynamicBuffer m_visPoints, m_visSpots;
	vector<Lighting_Component*> m_queueDir, m_queuePoint, m_queueSpot;
};

#endif // DS_LIGHTING