#pragma once
#ifndef BLOOM_TECHNIQUE_H
#define BLOOM_TECHNIQUE_H

#include "Systems\Graphics\Resources\FX Techniques\FX_Technique.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\StaticBuffer.h"


class Engine;
class Lighting_FBO;
class VisualFX;

/**
 * A post processing technique for generating bloom from a lighting buffer.
 **/
class Bloom_Tech : public FX_Technique {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~Bloom_Tech();
	/** Constructor. */
	Bloom_Tech(Engine * engine, Lighting_FBO * lightingFBO, VisualFX * visualFX);


	// Interface Implementations.
	virtual const char * getName() const { return "Bloom_Tech"; }
	virtual void applyEffect();	
	virtual void bindForReading();


	// Public Methods
	/** Change the strength of the bloom effect.
	* @param	strength		the new strength of the bloom effect */
	void setBloomStrength(const int &strength);
	/** Resize the frame buffer.
	* @param	size	the new size of the frame buffer */
	void resize(const vec2 & size);


private:
	// Private Attributes
	Engine * m_engine;
	Lighting_FBO * m_lightingFBO;
	VisualFX *m_visualFX;
	Shared_Asset_Shader m_shaderBloomExtract;
	Shared_Asset_Primitive m_shapeQuad;
	GLuint m_quadVAO;
	bool m_quadVAOLoaded;
	StaticBuffer m_quadIndirectBuffer;
	GLuint m_fbo, m_texture, m_texturesGB[2];
	vec2 m_renderSize;
	int m_bloomStrength;
};

#endif // BLOOM_TECHNIQUE_H