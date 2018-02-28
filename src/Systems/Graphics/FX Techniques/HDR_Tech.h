#pragma once
#ifndef HDR_TECHNIQUE
#define HDR_TECHNIQUE

#include "Systems\Graphics\FX Techniques\FX_Technique.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"

using namespace glm;
class EnginePackage;


/**
 * A post-processing technique for joining together light + bloom, and applying hdr to it by tone-mapping it and gamma correcting it.
 **/
class HDR_Tech : public FX_Technique {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~HDR_Tech();
	/** Constructor. */
	HDR_Tech(EnginePackage * enginePackage);


	// Interface Implementations.
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
	EnginePackage * m_enginePackage;
	Shared_Asset_Shader m_shaderHDR;
	Shared_Asset_Primitive m_shapeQuad;
	GLuint m_quadVAO;
	void* m_QuadObserver;
};

#endif // HDR_TECHNIQUE