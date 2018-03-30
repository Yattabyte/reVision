#pragma once
#ifndef FXAA_TECHNIQUE
#define FXAA_TECHNIQUE

#include "Systems\Graphics\FX Techniques\FX_Technique.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\MappedBuffer.h"

using namespace glm;
class EnginePackage;


/**
 * A post-processing technique for applying fxaa to the currently bound 2D image.
 **/
class FXAA_Tech : public FX_Technique {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~FXAA_Tech();
	/** Constructor. */
	FXAA_Tech();


	// Interface Implementations.
	virtual void applyEffect();	
	virtual void bindForReading() {}


private:
	// Private Attributes
	Shared_Asset_Shader m_shaderFXAA;
	Shared_Asset_Primitive m_shapeQuad;
	GLuint m_quadVAO;
	MappedBuffer m_quadIndirectBuffer;
};

#endif // FXAA_TECHNIQUE