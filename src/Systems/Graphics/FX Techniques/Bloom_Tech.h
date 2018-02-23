#pragma once
#ifndef BLOOM_TECHNIQUE
#define BLOOM_TECHNIQUE

#include "Systems\Graphics\FX Techniques\FX_Technique.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"

class EnginePackage;
class Callback_Container;
class Lighting_Buffer;
class VisualFX;


/**
 * A lighting technique that calculates direct diffuse and direct specular lighting contribution for directional, point, and spot light types, using PBR techniques.
 **/
class Bloom_Tech : public FX_Technique {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~Bloom_Tech();
	/** Constructor. */
	Bloom_Tech(EnginePackage * enginePackage, Lighting_Buffer * lBuffer, VisualFX * visualFX, const vec2 & size);


	// Interface Implementations.
	virtual void applyEffect();	
	virtual void bindForReading();


	// Public Methods
	/** Change the strength of the bloom effect.
	* @param	strength		the new strength of the bloom effect */
	void setBloomStrength(const int &strength);


private:
	// Private Attributes
	EnginePackage * m_enginePackage;
	Lighting_Buffer * m_lBuffer;
	VisualFX *m_visualFX;
	Shared_Asset_Shader m_shaderBloomExtract;
	Shared_Asset_Primitive m_shapeQuad;
	GLuint m_quadVAO;
	void* m_QuadObserver;
	GLuint m_fbo, m_texture, m_texturesGB[2];
	vec2 m_renderSize;
	int m_bloomStrength;
	Callback_Container * m_bloomStrengthChangeCallback;
};

#endif // BLOOM_TECHNIQUE