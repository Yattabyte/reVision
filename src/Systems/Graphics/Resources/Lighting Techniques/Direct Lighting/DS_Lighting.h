#pragma once
#ifndef DS_LIGHTING_H
#define DS_LIGHTING_H

#include "Systems\Graphics\Resources\Lighting Techniques\Lighting_Technique.h"
#include "Systems\Graphics\Resources\Lights\Light_Tech.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\VectorBuffer.h"
#include "Utilities\MappedChar.h"


class Engine;
class Geometry_FBO;
class Lighting_FBO;
class Lighting_Component;
class Geometry_Buffers;

/**
 * Performs basic lighting calculation using deferred shading.
 * Responsible for entire direct portion of the lighting equation: direct diffuse and dirrect specular.
 * Supports physically based shaders.
 * Supports directional, point, and spot lights.
 **/
class DS_Lighting : public Lighting_Technique {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~DS_Lighting();
	/** Constructor. */
	DS_Lighting(
		Engine * engine,
		Geometry_FBO * geometryFBO, Lighting_FBO * lightingFBO, 
		std::vector<Light_Tech*> * baseTechs,
		Geometry_Buffers * geometryBuffers
	);


	// Public Functions
	/** Returns a type-casted technique that matches the given name.
	* @param	c	a const char array name of the desired technique to find
	* @return		the technique requested */
	template <typename T> T * getTechnique(const char * c) {
		return (T*)m_techMap[c];
	}


	// Interface Implementations
	virtual const char * getName() const { return "DS_Lighting"; }
	virtual void updateData(const Visibility_Token & vis_token);
	virtual void applyPrePass(const Visibility_Token & vis_token);
	virtual void applyLighting(const Visibility_Token & vis_token);	


private:
	// Private Attributes
	Engine * m_engine;
	std::vector<Light_Tech*> * m_baseTechs;
	int m_updateQuality;
	// Shared FBO's
	Geometry_FBO * m_geometryFBO;
	Lighting_FBO * m_lightingFBO;
	// Shared Buffers
	Geometry_Buffers * m_geometryBuffers;
	// Primitives
	Shared_Asset_Primitive m_shapeCube;
	GLuint m_cubeVAO;
	bool m_cubeVAOLoaded;
};

#endif // DS_LIGHTING_H