#pragma once
#ifndef LIGHTING_BUFFER
#define LIGHTING_BUFFER
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\Graphics\Frame Buffers\Frame_Buffer.h"

using namespace glm;
class EnginePackage;
class VisualFX;


/**
 * A specialized framebuffer that accumulates lighting information for a single frame.
 * Supports bloom, accumulates over-brightened lights in a second render-target.
 **/
class DT_ENGINE_API Lighting_Buffer : public Frame_Buffer
{
public:
	// (de)Constructors
	/** Destroy the lighting buffer. */
	~Lighting_Buffer();
	/** Construct the lighting buffer. */
	Lighting_Buffer();


	// Public Methods
	/** Initialize the framebuffer.
	 * @param	enginePackage	the engine package
	 * @param	depthStencil	reference to the depthStencil texture from the gBuffer */
	void initialize(EnginePackage * enginePackage, const GLuint & depthStencil);	
	/** Binds the framebuffer and its render-targets for writing. */
	virtual void bindForWriting();
	/** Binds the framebuffer and its render-targets for reading. */
	virtual void bindForReading();
	/** Change the size of the framebuffer object. 
	 * @param	size			the new size of the framebuffer */
	virtual void resize(const ivec2 & size);
	/** Apply blur filter to bloom attachment, finishing the bloom effect. */
	void end();
	

private:
	// Private Attributes
	GLuint  m_texture, m_depth_stencil; // Donated by the geometry buffer
	EnginePackage * m_enginePackage;
};

#endif // LIGHTING_BUFFER