#pragma once
#ifndef INDIRECTDRAW_H
#define INDIRECTDRAW_H

#include "Utilities/GL/StaticTripleBuffer.h"


/** A helper class encapsulating the data needed to perform an indirect draw call in OpenGL. */
template <int BufferCount = 3>
class IndirectDraw {
public:
	// Public (de)Constructors
	/** Destroy this Indirect Draw Object. */
	inline ~IndirectDraw() = default;
	/** Default Constructor. */
	inline IndirectDraw() = default;
	/** Construct an Indirect Draw Object. */
	inline IndirectDraw(
		const GLuint& count,
		const GLuint& primitiveCount,
		const GLuint& first,
		const GLbitfield& storageFlags = GL_DYNAMIC_STORAGE_BIT
	) : m_count(count), m_primitiveCount(primitiveCount), m_first(first) {
		// Populate Buffer
		const GLuint data[4] = { count, primitiveCount, first, 0 };
		m_buffer = StaticTripleBuffer<BufferCount>(sizeof(GLuint) * 4, data, storageFlags);
	}
	/** Copy an Indirect Draw Object. */
	inline IndirectDraw(const IndirectDraw& other)
		: m_buffer(other.m_buffer) {
		m_count = other.m_count;
		m_primitiveCount = other.m_primitiveCount;
		m_first = other.m_first;
		m_storageFlags = other.m_storageFlags;
	}
	/** Move an Indirect Draw Object. */
	inline IndirectDraw(IndirectDraw&& other) noexcept {
		*this = std::move(other);
	}
	/** Move an Indirect Draw Object. */
	inline IndirectDraw& operator=(IndirectDraw&& other) noexcept {
		if (this != &other) {
			m_count = other.m_count;
			m_primitiveCount = other.m_primitiveCount;
			m_first = other.m_first;
			m_storageFlags = other.m_storageFlags;
			m_buffer = std::move(other.m_buffer);
		}
		return *this;
	}


	// Public Methods
	/** Bind this draw call to the OpenGL indirect buffer target. */
	inline void bind() {
		m_buffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
	}
	/** Bind this buffer and also perform an indirect draw call. */
	inline void drawCall(const void* indirect = 0) {
		bind();
		glDrawArraysIndirect(GL_TRIANGLES, indirect);
	}
	/** Prepare this buffer for writing, waiting on its sync fence. */
	inline void beginWriting() {
		m_buffer.beginWriting();
	}
	/** Signal that this buffer has finished being written to, place a sync fence, switch to next buffer set. */
	inline void endWriting() {
		m_buffer.endWriting();
	}
	/***/
	inline void endReading() {
		m_buffer.endReading();
	}
	/** Specify how many vertices will be rendered.
	@param	count			the vertex count. */
	inline void setCount(const GLuint& count) {
		m_count = count;
		m_buffer.write(0, GLsizeiptr(sizeof(GLuint)), &count);
	}
	/** Specify how many primitives will be rendered.
	@param	primitiveCount	the number of primitives to be rendered. */
	inline void setPrimitiveCount(const GLuint& primitiveCount) {
		m_primitiveCount = primitiveCount;
		m_buffer.write(GLsizeiptr(sizeof(GLuint)), GLsizeiptr(sizeof(GLuint)), &primitiveCount);
	}
	/** Specify the offset to the first vertex to be rendered.
	@param	first			the offset to the first rendered vertex. */
	inline void setFirst(const GLuint& first) {
		m_first = first;
		m_buffer.write(GLsizeiptr(sizeof(GLuint)) * 2ull, GLsizeiptr(sizeof(GLuint)), &first);
	}


private:
	// Private Attributes
	GLuint
		m_count = 0,
		m_primitiveCount = 0,
		m_first = 0;
	GLbitfield m_storageFlags = GL_DYNAMIC_STORAGE_BIT;
	StaticTripleBuffer<BufferCount> m_buffer;
};

#endif // INDIRECTDRAW_H