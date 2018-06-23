#pragma once
#ifndef IBL_PARALLAX_H
#define IBL_PARALLAX_H

#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\Reflectors\Reflector_Tech.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\StaticBuffer.h"

using namespace glm;
class Engine;
class Reflector_Component;


/**
 * A reflection technique that uses run-time generated cubemaps, applied in real-time
 */
class IBL_Parallax_Tech : public Reflector_Tech {
public:
	// (de)Constructors
	/** Destructor. */
	~IBL_Parallax_Tech();
	/** Constructor. */
	IBL_Parallax_Tech(Engine * engine);


	// Public Functions
	void addElement();
	void removeElement(const unsigned int & index);


	// Interface Implementations
	virtual const char * getName() const { return "IBL_Parallax_Tech"; }
	virtual void updateData(const Visibility_Token & vis_token);
	virtual void applyPrePass();
	virtual void applyEffect();


private:
	// Private Attributes
	Engine * m_engine;
	bool m_regenCubemap;
	bool m_quadVAOLoaded, m_cubeVAOLoaded;
	GLuint m_quadVAO, m_cubeVAO;
	Shared_Asset_Shader m_shaderEffect, m_shaderCopy, m_shaderConvolute;
	Shared_Asset_Primitive m_shapeQuad, m_shapeCube;
	StaticBuffer m_quadIndirectBuffer, m_quad6FaceIndirectBuffer, m_cubeIndirectBuffer, m_visRefUBO;
	vector<Reflector_Component*> m_refList;
	size_t m_size;
	GLuint m_fbo, m_texture;
	GLuint m_reflectorCount;
	

};
#endif // IBL_PARALLAX_H
