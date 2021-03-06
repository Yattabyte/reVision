#pragma once
#ifndef INDIRECTDRAW_H
#define INDIRECTDRAW_H

#include "Utilities/GL/StaticMultiBuffer.h"


/** A helper class encapsulating the data needed to perform an indirect draw call in OpenGL. */
template <int BufferCount = 3>
class IndirectDraw {
public:
	// Public (De)Constructors
	/** Default Constructor. */
	inline IndirectDraw() noexcept {}
	/** Construct an Indirect Draw Object.
	@param	count			the number of vertices to draw.
	@param	primitiveCount	the number of times to draw this object.
	@param	first			offset to the first vertex.
	@param	storageFlags	storage type flag. */
	inline IndirectDraw(
		const GLuint& count,
		const GLuint& primitiveCount,
		const GLuint& first,
		const GLbitfield& storageFlags = GL_DYNAMIC_STORAGE_BIT
	) noexcept : m_count(count), m_primitiveCount(primitiveCount), m_first(first), m_storageFlags(storageFlags) {
		// Populate Buffer
		const GLuint data[4] = { count, primitiveCount, first, 0 };
		m_buffer = StaticMultiBuffer<BufferCount>(sizeof(GLuint) * 4, data, storageFlags);
	}


	// Public Methods
	/** Bind this draw call to the OpenGL indirect buffer target. */
	inline void bind() noexcept {
		m_buffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
	}
	/** Bind this buffer and also perform an indirect draw call.
	@param	indirect		an indirect pointer. */
	inline void drawCall(const void* indirect = nullptr) noexcept {
		bind();
		glDrawArraysIndirect(GL_TRIANGLES, indirect);
	}
	/** Prepare this buffer for writing, waiting on its sync fence. */
	inline void beginWriting() const noexcept {
		m_buffer.beginWriting();
	}
	/** Signal that this buffer has finished being written to. */
	inline void endWriting() const noexcept {
		m_buffer.endWriting();
	}
	/** Signal that this buffer has finished being read from. */
	inline void endReading() noexcept {
		m_buffer.endReading();
	}
	/** Specify how many vertices will be rendered.
	@param	count			the vertex count. */
	inline void setCount(const GLuint& count) noexcept {
		m_count = count;
		m_buffer.write(0, (GLsizeiptr)(sizeof(GLuint)), &count);
	}
	/** Specify how many primitives will be rendered.
	@param	primitiveCount	the number of primitives to be rendered. */
	inline void setPrimitiveCount(const GLuint& primitiveCount) noexcept {
		m_primitiveCount = primitiveCount;
		m_buffer.write((GLsizeiptr)(sizeof(GLuint)), (GLsizeiptr)(sizeof(GLuint)), &primitiveCount);
	}
	/** Specify the offset to the first vertex to be rendered.
	@param	first			the offset to the first rendered vertex. */
	inline void setFirst(const GLuint& first) noexcept {
		m_first = first;
		m_buffer.write((GLsizeiptr)(sizeof(GLuint)) * 2ull, (GLsizeiptr)(sizeof(GLuint)), &first);
	}


private:
	// Private Attributes
	GLuint
		m_count = 0,
		m_primitiveCount = 0,
		m_first = 0;
	GLbitfield m_storageFlags = GL_DYNAMIC_STORAGE_BIT;
	StaticMultiBuffer<BufferCount> m_buffer;
};

#endif // INDIRECTDRAW_H