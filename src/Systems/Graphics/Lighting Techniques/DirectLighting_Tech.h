#pragma once
#ifndef DIRECT_LIGHTING_PBR
#define DIRECT_LIGHTING_PBR

#include "Systems\Graphics\Lighting Techniques\Lighting_Technique.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"

class Geometry_Buffer;
class Lighting_Buffer;
class Shadow_Buffer;


/**
 * A lighting technique that calculates direct diffuse and direct specular lighting contribution for directional, point, and spot light types, using PBR techniques.
 **/
class DirectLighting_Tech : public Lighting_Technique {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~DirectLighting_Tech();
	/** Constructor. */
	DirectLighting_Tech(Geometry_Buffer * gBuffer, Lighting_Buffer * lBuffer, Shadow_Buffer *sBuffer);


	// Interface Implementations.
	virtual void updateLighting(const Visibility_Token & vis_token);
	virtual void applyLighting(const Visibility_Token & vis_token);	


private:
	// Private Attributes
	Geometry_Buffer * m_gBuffer;
	Lighting_Buffer * m_lBuffer;
	Shadow_Buffer * m_sBuffer;
	Shared_Asset_Shader m_shaderDirectional, m_shaderPoint, m_shaderSpot;
	Shared_Asset_Primitive m_shapeQuad, m_shapeCone, m_shapeSphere;
	GLuint m_quadVAO, m_coneVAO, m_sphereVAO;
	void* m_QuadObserver, *m_ConeObserver, *m_SphereObserver;
};

#endif // DIRECT_LIGHTING_PBR