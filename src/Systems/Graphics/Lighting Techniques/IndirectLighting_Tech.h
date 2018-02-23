#pragma once
#ifndef INDIRECT_LIGHTING_PBR
#define INDIRECT_LIGHTING_PBR

#include "Systems\Graphics\Lighting Techniques\Lighting_Technique.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Systems\Graphics\Frame Buffers\GlobalIllumination_Buffer.h"

class EnginePackage;
class Geometry_Buffer;
class Lighting_Buffer;
class Shadow_Buffer;


/**
 * A lighting technique that calculates indirect diffuse and indirect specular lighting contribution for directional, point, and spot light types, using PBR techniques.
 **/
class IndirectLighting_Tech : public Lighting_Technique {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~IndirectLighting_Tech();
	/** Constructor. */
	IndirectLighting_Tech(EnginePackage * enginePackage, Geometry_Buffer * gBuffer, Lighting_Buffer * lBuffer, Shadow_Buffer *sBuffer);


	// Interface Implementations.
	virtual void updateLighting(const Visibility_Token & vis_token);
	virtual void applyLighting(const Visibility_Token & vis_token);	


private:
	// Private Attributes
	Geometry_Buffer * m_gBuffer;
	Lighting_Buffer * m_lBuffer;
	Shadow_Buffer * m_sBuffer;
	EnginePackage * m_enginePackage;
	Shared_Asset_Shader m_shaderDirectional_Bounce, m_shaderPoint_Bounce, m_shaderSpot_Bounce, m_shaderGISecondBounce, m_shaderGIReconstruct;
	Shared_Asset_Primitive m_shapeQuad;
	GLuint m_quadVAO, m_bounceVAO;
	void* m_QuadObserver;
	GlobalIllumination_Buffer m_giBuffer;
};

#endif // INDIRECT_LIGHTING_PBR