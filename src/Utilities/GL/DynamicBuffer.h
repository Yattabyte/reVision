#pragma once
#ifndef DYNAMICBUFFER_H
#define DYNAMICBUFFER_H
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "GL\glew.h"
#include <utility>


class DynamicBuffer
{
public:
	// Public (de)Constructors
	~DynamicBuffer() {
		if (m_bufferID != 0) {
			glUnmapNamedBuffer(m_bufferID);
			glDeleteBuffers(1, &m_bufferID);
		}
	}
	/** Default. */
	DynamicBuffer(const GLsizeiptr & size = 64, const void * data = 0, const GLbitfield & mapFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT) {
		m_maxCapacity = size;
		m_mapFlags = mapFlags;
		m_bufferID = 0;
		glCreateBuffers(1, &m_bufferID);
		glNamedBufferStorage(m_bufferID, size, data, GL_DYNAMIC_STORAGE_BIT | m_mapFlags);
		m_bufferPtr = glMapNamedBufferRange(m_bufferID, 0, size, m_mapFlags);
	}
	/** Move gl object from 1 instance to another. */
	DynamicBuffer & operator=(DynamicBuffer && o) noexcept {
		m_bufferID = (std::move(o.m_bufferID));
		m_bufferPtr = (std::move(o.m_bufferPtr));
		m_maxCapacity = (std::move(o.m_maxCapacity));
		m_mapFlags = (std::move(o.m_mapFlags));
		o.m_bufferID = 0;
		o.m_bufferPtr = nullptr;
		o.m_maxCapacity = 0;
		o.m_mapFlags = 0;
		return *this;
	}
		

	// Public Methods
	/** Bind this buffer.
	* @param	target	the target type of this buffer */
	void bindBuffer(const GLenum & target) const {
		glBindBuffer(target, m_bufferID);
	}
	/** Bind this buffer to a particular binding point for shaders.
	 * @param	target	the target type of this buffer
	 * @param	index	the binding point index to use */
	void bindBufferBase(const GLenum & target, const GLuint & index) const {
		glBindBufferBase(target, index, m_bufferID);
	}
	/** Cast this buffer's pointer to a type, as to allow modifying its underlying data. 
	 * @return			the pointer to this data in memory, cast to the type specified
	 * @param	<T>		the type to cast this to */
	template <typename T>
	T castPointer() {
		return reinterpret_cast<T>(m_bufferPtr);
	}
	/** Expand this buffer to fit the size provided.
	 * @param	size	the size to expand up to (if not already larger) */
	void setMaxSize(const GLsizeiptr & size) {
		expandToFit(0, size);
	}
	/** Write the supplied data to GPU memory
	 * @param	offset	byte offset from the beginning
	 * @param	size	the size of the data to write
	 * @param	data	the data to write */
	void write(const GLsizeiptr & offset, const GLsizeiptr & size, const void * data) {
		expandToFit(offset, size);

		std::memcpy(reinterpret_cast<unsigned char*>(m_bufferPtr) + offset, data, size);
	}
	/** Write the supplied data to GPU memory
	 * @param	offset	byte offset from the beginning
	 * @param	size	the size of the data to write
	 * @param	data	the data to write */
	void write_immediate(const GLsizeiptr & offset, const GLsizeiptr & size, const void * data) {
		expandToFit(offset, size);

		glNamedBufferSubData(m_bufferID, offset, size, data);
	}
	/* Expands this buffer's container if it can't fit the specified range to write into
	 * @note Technically creates a new a new buffer to replace the old one and copies the old data
	 * @param	offset	byte offset from the beginning
	 * @param	size	the size of the data to write */
	void expandToFit(const GLsizeiptr & offset, const GLsizeiptr & size) {
		if (offset + size > m_maxCapacity) {
			// Create new buffer large enough to fit old data + new data
			const GLsizeiptr oldSize = m_maxCapacity;
			m_maxCapacity += offset + (size * 1.5);

			// Create new buffer
			GLuint newBuffer = 0;
			glCreateBuffers(1, &newBuffer);
			glNamedBufferStorage(newBuffer, m_maxCapacity, 0, GL_DYNAMIC_STORAGE_BIT | m_mapFlags);

			// Copy old buffer
			if (oldSize)
				glCopyNamedBufferSubData(m_bufferID, newBuffer, 0, 0, oldSize);

			// Delete old buffer
			glUnmapNamedBuffer(m_bufferID);
			glDeleteBuffers(1, &m_bufferID);

			// Migrate new buffer
			m_bufferID = newBuffer;
			m_bufferPtr = glMapNamedBufferRange(m_bufferID, 0, m_maxCapacity, m_mapFlags);
		}
	}


private:
	// Private Attributes
	GLuint m_bufferID;
	void * m_bufferPtr;
	GLuint m_maxCapacity;
	GLbitfield m_mapFlags;
};

#endif // DYNAMICBUFFER_H