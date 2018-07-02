#pragma once
#ifndef FXAA_TECHNIQUE_H
#define FXAA_TECHNIQUE_H

#include "Systems\Graphics\FX Techniques\FX_Technique.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\StaticBuffer.h"

using namespace glm;
class Engine;


/**
 * A post-processing technique for applying fxaa to the currently bound 2D image.
 **/
class FXAA_Tech : public FX_Technique {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~FXAA_Tech();
	/** Constructor. */
	FXAA_Tech(Engine * engine);


	// Interface Implementations.
	virtual const char * getName() const { return "FXAA_Tech"; }
	virtual void applyEffect();	
	virtual void bindForReading() {}


private:
	// Private Attributes
	Engine * m_engine;
	Shared_Asset_Shader m_shaderFXAA;
	Shared_Asset_Primitive m_shapeQuad;
	GLuint m_quadVAO;
	bool m_quadVAOLoaded;
	StaticBuffer m_quadIndirectBuffer;
};

#endif // FXAA_TECHNIQUE_H