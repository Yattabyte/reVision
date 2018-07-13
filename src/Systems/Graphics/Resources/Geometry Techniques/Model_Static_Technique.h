#pragma once
#ifndef MODEL_STATIC_TECHNIQUE_H
#define MODEL_STATIC_TECHNIQUE_H

#include "Systems\Graphics\Resources\Geometry Techniques\Geometry_Technique.h"
#include "Systems\Graphics\Resources\GFX_DEFINES.h"
#include "Utilities\GL\VectorBuffer.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"


class Geometry_FBO; 
class Camera;

/**
 * Renders static models
 **/
class Model_Static_Technique : public Geometry_Technique {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~Model_Static_Technique();
	/** Constructor. */
	Model_Static_Technique(Engine * engine, Geometry_FBO * geometryFBO, VectorBuffer<Geometry_Static_Struct> * geometrySSBO);


	// Public Interface Implementations
	virtual void updateData(const std::vector<Camera*> & viewers);
	virtual void renderGeometry(Camera & viewers);
	virtual void occlusionCullBuffers(Camera & camera);


	// Public methods
	static void writeCameraBuffers(Camera & camera, const unsigned int & instanceCount = 1);


private:
	// Private Attributes
	Engine * m_engine;
	// Shared Attribute Pointers
	Geometry_FBO * m_geometryFBO;
	VectorBuffer<Geometry_Static_Struct> * m_geometryStaticSSBO;
	// Assets
	Shared_Asset_Shader m_shaderCull, m_shaderGeometry;
	Shared_Asset_Primitive m_shapeCube;
	bool m_cubeVAOLoaded;
	GLuint m_cubeVAO;
};

#endif // MODEL_STATIC_TECHNIQUE_H