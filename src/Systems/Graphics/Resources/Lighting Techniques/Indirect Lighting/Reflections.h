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
	Reflections(Engine * engine, Geometry_FBO * geometryFBO, Lighting_FBO * lightingFBO, Reflection_FBO * reflectionFBO);


	// Public Functions
	/** Returns a type-casted reflector technique that matches the given name.
	 * @param	c	a const char array name of the desired technique to find
	 * @return		the technique requested */
	template <typename T> T * getReflectorTech(const char * c) {
		return (T*)m_refTechMap[c];
	}


	// Interface Implementations
	virtual const char * getName() const { return "Reflections"; }
	virtual void updateData(const Visibility_Token & vis_token);
	virtual void applyPrePass(const Visibility_Token & vis_token);
	virtual void applyLighting(const Visibility_Token & vis_token);	
	
	
private:
	// Private Attributes
	Geometry_FBO * m_geometryFBO;
	Lighting_FBO * m_lightingFBO;
	Reflection_FBO * m_reflectionFBO;
	Engine * m_engine;
	Shared_Asset_Shader m_shaderFinal;
	Shared_Asset_Primitive m_shapeQuad;
	Shared_Asset_Texture m_brdfMap;
	GLuint m_quadVAO;
	bool m_quadVAOLoaded;
	StaticBuffer m_quadIndirectBuffer;
	std::vector<Reflector_Tech*> m_refTechs;
	MappedChar<void*> m_refTechMap;
};

#endif // REFLECTIONS_H