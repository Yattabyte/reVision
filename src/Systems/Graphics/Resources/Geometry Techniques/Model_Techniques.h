#pragma once
#ifndef MODEL_TECHNIQUE
#define MODEL_TECHNIQUE
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\Graphics\Resources\Geometry Techniques\Geometry_Technique.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"

class Geometry_FBO; 
class Camera;


/**
 * Renders models (animated or static props which support skeletons)
 **/
class DT_ENGINE_API Model_Technique : public Geometry_Technique {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~Model_Technique();
	/** Constructor. */
	Model_Technique(Geometry_FBO * geometryFBO);


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
	// Assets
	Shared_Asset_Shader m_shaderCull, m_shaderGeometry;
	Shared_Asset_Primitive m_shapeCube;
	bool m_cubeVAOLoaded;
	GLuint m_cubeVAO;
};

#endif // MODEL_TECHNIQUE