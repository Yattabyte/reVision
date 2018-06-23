#pragma once
#ifndef HDR_TECHNIQUE_H
#define HDR_TECHNIQUE_H

#include "Systems\Graphics\FX Techniques\FX_Technique.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\StaticBuffer.h"

using namespace glm;
class Engine;


/**
 * A post-processing technique for joining together light + bloom, and applying hdr to it by tone-mapping it and gamma correcting it.
 **/
class HDR_Tech : public FX_Technique {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~HDR_Tech();
	/** Constructor. */
	HDR_Tech(Engine * engine);


	// Interface Implementations.
	virtual const char * getName() const { return "HDR_Tech"; }
	virtual void applyEffect();	
	virtual void bindForReading();

	
	// Public Methods
	/** Resize the frame buffer. 
	 * @param	size	the new size of the frame buffer */
	void resize(const vec2 & size);


private:
	// Private Attributes
	GLuint m_fbo;
	GLuint m_texture;
	vec2 m_renderSize; 
	Engine * m_engine;
	Shared_Asset_Shader m_shaderHDR;
	Shared_Asset_Primitive m_shapeQuad;
	GLuint m_quadVAO;
	bool m_vaoLoaded;
	StaticBuffer m_quadIndirectBuffer;
};

#endif // HDR_TECHNIQUE_H