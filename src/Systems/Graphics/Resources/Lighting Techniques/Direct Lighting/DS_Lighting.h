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
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Utilities\GL\DynamicBuffer.h"

class Geometry_FBO;
class Lighting_FBO;
class Shadow_FBO;


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
	DS_Lighting(Geometry_FBO * geometryFBO, Lighting_FBO * lightingFBO, Shadow_FBO *shadowFBO);


	// Interface Implementations.
	virtual void updateData(const Visibility_Token & vis_token);
	virtual void applyLighting(const Visibility_Token & vis_token);	


private:
	// Private Attributes
	Geometry_FBO * m_geometryFBO;
	Lighting_FBO * m_lightingFBO;
	Shadow_FBO * m_shadowFBO;
	Shared_Asset_Shader m_shaderDirectional, m_shaderPoint, m_shaderSpot;
	Shared_Asset_Primitive m_shapeQuad, m_shapeCone, m_shapeSphere;
	GLuint m_quadVAO, m_coneVAO, m_sphereVAO;
	bool m_quadVAOLoaded, m_coneVAOLoaded, m_sphereVAOLoaded; 
	StaticBuffer m_indirectDir;
};

#endif // DS_LIGHTING