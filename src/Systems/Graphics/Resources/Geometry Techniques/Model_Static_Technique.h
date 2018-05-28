#pragma once
#ifndef MODEL_STATIC_TECHNIQUE
#define MODEL_STATIC_TECHNIQUE
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

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
class DT_ENGINE_API Model_Static_Technique : public Geometry_Technique {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~Model_Static_Technique();
	/** Constructor. */
	Model_Static_Technique(Geometry_FBO * geometryFBO, VectorBuffer<Geometry_Static_Struct> * geometrySSBO);


	// Public Interface Implementations
	virtual void updateData(const vector<Camera*> & viewers);
	virtual void renderGeometry(Camera & viewers);
	virtual void occlusionCullBuffers(Camera & camera);


	// Public methods
	static void writeCameraBuffers(Camera & camera, const unsigned int & instanceCount = 1);


private:
	// Private Attributes
	// Shared Attribute Pointers
	Geometry_FBO * m_geometryFBO;
	VectorBuffer<Geometry_Static_Struct> * m_geometryStaticSSBO;
	// Assets
	Shared_Asset_Shader m_shaderCull, m_shaderGeometry;
	Shared_Asset_Primitive m_shapeCube;
	bool m_cubeVAOLoaded;
	GLuint m_cubeVAO;
};

#endif // MODEL_STATIC_TECHNIQUE