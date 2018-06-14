#pragma once
#ifndef REFLECTIONS_H
#define REFLECTIONS_H
#define BRDF_SIZE 512

#include "Systems\Graphics\Resources\Lighting Techniques\Lighting_Technique.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\Reflectors\Reflector_Tech.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Assets\Asset_Texture.h"
#include "Utilities\GL\StaticBuffer.h"

class Geometry_FBO;
class Lighting_FBO;
class Reflection_FBO;
class Reflection_UBO;
class Reflector_Component;


/**
 * Performs a reflection pass using the screen as input - using the screen space reflection technique.
 * Also caches the viewport over time into a persistent cubemap, used as a fallback environment map.
 * Lastly, supports parallax corrected local cubemaps.
 * Responsible for indirect specular lighting.
 * Supports physically based shaders.
 **/
class Reflections : public Lighting_Technique {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~Reflections();
	/** Constructor. */
	Reflections(EnginePackage * enginePackage, Geometry_FBO * geometryFBO, Lighting_FBO * lightingFBO, Reflection_FBO * reflectionFBO);


	// Interface Implementations
	virtual void updateData(const Visibility_Token & vis_token);
	virtual void applyPrePass(const Visibility_Token & vis_token);
	virtual void applyLighting(const Visibility_Token & vis_token);	
	
	
private:
	// Private Attributes
	Geometry_FBO * m_geometryFBO;
	Lighting_FBO * m_lightingFBO;
	Reflection_FBO * m_reflectionFBO;
	EnginePackage * m_enginePackage;
	Shared_Asset_Shader m_shaderFinal, m_shaderParallax;
	Shared_Asset_Primitive m_shapeQuad, m_shapeCube;
	Shared_Asset_Texture m_brdfMap;
	GLuint m_quadVAO, m_cubeVAO;
	bool m_quadVAOLoaded, m_cubeVAOLoaded;
	StaticBuffer m_quadIndirectBuffer, m_cubeIndirectBuffer, m_visRefUBO;
	vector<Reflector_Component*> m_refList;
	vector<Reflector_Tech*> m_refTechs;
};

#endif // REFLECTIONS_H