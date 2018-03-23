#pragma once
#ifndef REFLECTION_BUFFER
#define REFLECTION_BUFFER
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define GLEW_STATIC

#include "Systems\Graphics\Frame Buffers\Frame_Buffer.h"
#include "Utilities\GL_MappedBuffer.h"

class EnginePackage;


/**
* A specialized framebuffer that accumulates lighting information for a single frame.
* Supports bloom, accumulates over-brightened lights in a second render-target.
**/
class DT_ENGINE_API Reflection_Buffer : public Frame_Buffer
{
public:
	// (de)Constructors
	/** Destroy the reflection buffer. */
	~Reflection_Buffer();
	/** Construct the lighting buffer. */
	Reflection_Buffer();


	// Public Interface Implementations
	/** Initialize the framebuffer. 
	 * @param	enginePackage	the engine package */
	void initialize(EnginePackage * enginePackage);
	/** Binds the framebuffer and its render-targets for reading. */
	void bindForReading(const unsigned int & texture_unit);
	/** Binds the framebuffer and its render-targets for writing. */
	virtual void bindForWriting();
	/** Change the size of the framebuffer object.
	* @param	size	the new size of the framebuffer */
	virtual void resize(const vec2 & size);
	/**/
	void  * const addReflector(unsigned int & uboIndex);


private:
	// Private Attributes
	GLuint m_texture;
	GL_MappedBuffer m_buffer;
	unsigned int m_count;
	EnginePackage * m_enginePackage;
};

#endif // REFLECTION_BUFFER