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
#include "Systems\Graphics\Resources\GFX_DEFINES.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\DynamicBuffer.h"
#include "Utilities\GL\VectorBuffer.h"
#include "Utilities\GL\StaticBuffer.h"

class Geometry_FBO; 
class Shadow_FBO;
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
	Model_Technique( Geometry_FBO * geometryFBO, Shadow_FBO * shadowFBO,
		VectorBuffer<Directional_Struct> * lightDirSSBO, VectorBuffer<Point_Struct> *lightPointSSBO, VectorBuffer<Spot_Struct> *lightSpotSSBO
	);


	// Public Interface Implementations
	virtual void updateData(const vector<Camera*> & viewers);
	virtual void renderGeometry(Camera & viewers);
	virtual void occlusionCullBuffers(Camera & camera);


	// Public methods
	static void writeCameraBuffers(Camera & camera);


private:
	// Private Attributes
	// Shared FBO's
	Geometry_FBO * m_geometryFBO;
	Shadow_FBO * m_shadowFBO;
	// Shared SSBO's
	VectorBuffer<Directional_Struct> * m_lightDirSSBO;
	VectorBuffer<Point_Struct> * m_lightPointSSBO;
	VectorBuffer<Spot_Struct> * m_lightSpotSSBO;
	Shared_Asset_Shader m_shaderCull, m_shaderGeometry;
	Shared_Asset_Primitive m_shapeCube;
	bool m_cubeVAOLoaded;
	GLuint m_cubeVAO;
};

#endif // MODEL_TECHNIQUE