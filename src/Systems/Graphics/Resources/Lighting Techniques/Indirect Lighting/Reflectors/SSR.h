#pragma once
#ifndef SSR_H
#define SSR_H

#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\Reflectors\Reflector_Tech.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\StaticBuffer.h"

using namespace glm;
class EnginePackage;
class Geometry_FBO;
class Lighting_FBO;
class Reflection_FBO;


/**
 * A reflection technique that uses the skybox to generate reflections
 */
class SSR_Tech : public Reflector_Tech {
public:
	// (de)Constructors
	/** Destructor. */
	~SSR_Tech();
	/** Constructor. */
	SSR_Tech(EnginePackage * enginePackage, Geometry_FBO * geometryFBO, Lighting_FBO * lightingFBO, Reflection_FBO * reflectionFBO);


	// Interface Implementations
	virtual const char * getName() const { return "SSR_Tech"; }
	virtual void updateData(const Visibility_Token & vis_token) {};
	virtual void applyPrePass() {};
	virtual void applyEffect();


private:
	/** Nested buffer object struct for sending data to GPU */
	struct SSR_Buffer {
		int maxSteps = 16;
		float maxDistance = 50.0f;
		float numMips = 6;
		float fadeStart = 0.40;
		float fadeEnd = 0.9f;
	};


	// Private Functions
	/** Resize the frame buffer by the amount specified.
	 * @param	size	the amount to resize by */
	void resize(const ivec2 & size);
	/** Binds the light buffer for reading and convolute's it into several MIPs, representing increasing roughness. */
	void updateMipChain();	


	// Private Attributes
	EnginePackage * m_enginePackage;
	Geometry_FBO * m_geometryFBO;
	Lighting_FBO * m_lightingFBO;
	Reflection_FBO * m_reflectionFBO;
	bool m_quadVAOLoaded;
	GLuint m_quadVAO, m_fbo, m_texture;
	ivec2 m_renderSize;
	Shared_Asset_Shader m_shaderCopy, m_shaderBlur, m_shaderEffect;
	Shared_Asset_Primitive m_shapeQuad;
	StaticBuffer m_quadIndirectBuffer, m_ssrBuffer;

};
#endif // SSR_H
