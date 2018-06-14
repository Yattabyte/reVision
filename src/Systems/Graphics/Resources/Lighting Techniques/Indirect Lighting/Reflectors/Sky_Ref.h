#pragma once
#ifndef SKY_REF_H
#define SKY_REF_H

#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\Reflectors\Reflector_Tech.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Assets\Asset_Cubemap.h"
#include "Utilities\GL\StaticBuffer.h"

using namespace glm;


/**
 * A reflection technique that uses the user's viewport to generate reflections
 */
class Sky_Ref_Tech : public Reflector_Tech {
public:
	// (de)Constructors
	/** Destructor. */
	~Sky_Ref_Tech();
	/** Constructor. */
	Sky_Ref_Tech();


	// Interface Implementations
	virtual void updateData(const Visibility_Token & vis_token) {};
	virtual void applyPrePass() {};
	virtual void applyEffect();


private:
	// Private Attributes
	bool m_quadVAOLoaded;
	GLuint m_quadVAO;
	Shared_Asset_Shader m_shaderEffect;
	Shared_Asset_Cubemap m_textureSky;
	Shared_Asset_Primitive m_shapeQuad;
	StaticBuffer m_quadIndirectBuffer;

};
#endif // SSR_H
