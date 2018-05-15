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
#include "Systems\Graphics\Resources\Lighting Techniques\Direct Lighting\Types\DS_Technique.h"
#include "Systems\Graphics\Resources\Light_Buffers.h"
#include "Assets\Asset_Primitive.h"
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
		Light_Buffers * lightBuffers
	);


	// Interface Implementations.
	virtual void updateData(const Visibility_Token & vis_token);
	virtual void applyPrePass(const Visibility_Token & vis_token);
	virtual void applyLighting(const Visibility_Token & vis_token);	


private:
	// Private Attributes
	vector<DS_Technique*> m_techniques;
	EnginePackage * m_enginePackage;
	int m_updateQuality;
	// Shared FBO's
	Geometry_FBO * m_geometryFBO;
	Lighting_FBO * m_lightingFBO;
	Shadow_FBO * m_shadowFBO;

	Shared_Asset_Primitive m_shapeCube;
	GLuint m_cubeVAO;
	bool m_cubeVAOLoaded;
};

#endif // DS_LIGHTING