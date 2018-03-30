#pragma once
#ifndef MappedBuffer_H
#define MappedBuffer_H
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#include "GL\glew.h"
#include <utility>


class MappedBuffer
{
public:
	// Public (de)Constructors
	~MappedBuffer() {
		if (m_bufferID != 0) {
			glUnmapNamedBuffer(m_bufferID);
			glDeleteBuffers(1, &m_bufferID);
		}
		if (m_fence)
			glDeleteSync(m_fence);
	}
	/** Default. */
	MappedBuffer() {
		m_bufferID = 0;
		m_index = 0;
		m_bufferPtr = nullptr;
		m_fence = nullptr;		
	}
	/** Explicit Instantion. */
	MappedBuffer(const GLsizeiptr & size, const void * data, const GLbitfield & mapFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT) : MappedBuffer() {
		glCreateBuffers(1, &m_bufferID);
		glNamedBufferStorage(m_bufferID, size, data, GL_DYNAMIC_STORAGE_BIT | mapFlags);
		m_bufferPtr = glMapNamedBufferRange(m_bufferID, 0, size, mapFlags);
	}
	/** Move gl object from 1 instance to another. */
	MappedBuffer & operator=(MappedBuffer && o) noexcept {
		m_bufferID = (std::move(o.m_bufferID));
		m_bufferPtr = (std::move(o.m_bufferPtr));
		m_fence = (std::move(o.m_fence));
		o.m_bufferID = 0;
		o.m_bufferPtr = nullptr;
		o.m_fence = nullptr;
		return *this;
	}
		

	// Public Methods
	/** Bind this buffer.
	* @param	target	the target type of this buffer */
	void bindBuffer(const GLenum & target) {
		glBindBuffer(target, m_bufferID);
	}
	/** Bind this buffer to a particular binding point for shaders.
	 * @param	target	the target type of this buffer
	 * @param	index	the binding point index to use */
	void bindBufferBase(const GLenum & target, const GLuint & index) {
		glBindBufferBase(target, index, m_bufferID);
	}
	/** Cause a synchronization point if the sync fence hasn't been passed. */
	void checkFence() {
		// Check if we should cause a synchronization point
		if (m_fence != nullptr) {
			auto state = GL_UNSIGNALED;
			while (state != GL_ALREADY_SIGNALED && state != GL_CONDITION_SATISFIED || state == GL_WAIT_FAILED)
				state = glClientWaitSync(m_fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
			glDeleteSync(m_fence);
			m_fence = nullptr;
		}
	}
	/** Create a sync fence, use if this buffers data was just changed. */
	void placeFence() {		
		if (m_fence)
			glDeleteSync(m_fence);
		m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	}
	/** Cast this buffer's pointer to a type, as to allow modifying its underlying data. 
	 * @return			the pointer to this data in memory, cast to the type specified
	 * @param	<T>		the type to cast this to */
	template <typename T>
	T castPointer() {
		return reinterpret_cast<T>(m_bufferPtr);
	}
	/** Write the supplied data to GPU memory
	 * @param	offset	byte offset from the beginning
	 * @param	size	the size of the data to write
	 * @param	data	the data to write */
	void write(const GLsizeiptr & offset, const GLsizeiptr & size, const void * data) {
		checkFence();
		std::memcpy(reinterpret_cast<unsigned char*>(m_bufferPtr) + offset, data, size);
		placeFence();
	}
	/** Write the supplied data to GPU memory
	* @param	offset	byte offset from the beginning
	* @param	size	the size of the data to write
	* @param	data	the data to write */
	void write_immediate(const GLuint & offset, const GLsizeiptr & size, const void * data) {
		checkFence();
		glNamedBufferSubData(m_bufferID, offset, size, data);
		placeFence();
	}
	/** Write the supplied data to GPU memory
	* @param	offset	byte offset from the beginning
	* @param	size	the size of the data to write
	* @param	data	the data to write */
	void fullWrite3X(const GLenum & target, const GLuint & index, const GLsizeiptr & size, const void * data) {
		checkFence();

		// buffer divided into 3 equal sized chunks
		// write to m_index section only
		const GLuint offset = (m_index * size);
		std::memcpy(reinterpret_cast<unsigned char*>(m_bufferPtr) + offset, data, size);
		glBindBufferRange(target, index, m_bufferID, offset, size);
		m_index = (m_index + 1) % 3;

		placeFence();
	}
	/** Retrieve the mapped buffer pointer. */
	void * getBufferPointer() const {
		return m_bufferPtr;
	}


private:
	// Private Attributes
	GLuint m_bufferID;
	void * m_bufferPtr;
	GLsync m_fence;
	GLuint m_index;
};

#endif // MappedBuffer_H