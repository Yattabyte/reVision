#pragma once
#ifndef SYSTEM_PERFCOUNTER_H
#define SYSTEM_PERFCOUNTER_H

#include "Systems\System_Interface.h"
#include "Assets\Asset_Texture.h"
#include "Assets\Asset_Primitive.h"
#include "Assets\Asset_Shader.h"
#include "Utilities\GL\StaticBuffer.h"


class Engine;

/**
 * A performance counter
 **/
class System_PerfCounter : public System
{
public:
	// (de)Constructors
	/** Destroy the rendering system. */
	~System_PerfCounter();
	/** Construct the rendering system. */
	System_PerfCounter();


	// Interface Implementations
	virtual void initialize(Engine * engine);
	virtual void update(const float & deltaTime);
	virtual void updateThreaded(const float & deltaTime) {};


private:
	// Private Methods
	void resize(const ivec2 s);


	// Private Attributes
	Shared_Asset_Texture m_numberTexture;
	Shared_Asset_Primitive m_shapeQuad;
	Shared_Asset_Shader m_shader;
	GLuint m_quadVAO;	
	bool m_quadVAOLoaded;
	StaticBuffer m_indirectQuad;
	ivec2 m_renderSize;
	mat4 m_projMatrix;
};

#endif // SYSTEM_PERFCOUNTER_H