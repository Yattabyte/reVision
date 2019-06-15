#pragma once
#ifndef GL_ARRAYBUFFER_H
#define GL_ARRAYBUFFER_H

#include "Utilities/GL/glad/glad.h"
#include <vector>

/** A data buffer that resides locally as well as on the GPU. 
Forms an expandable array of elements type T.
@param	<T>		the type of element to construct an array of. */
template <typename T>
class GL_ArrayBuffer {
public:
	// Public (de)Constructors
	/** Destroy this buffer. */
	inline ~GL_ArrayBuffer() {
		if (m_bufferID != 0) {
			glUnmapNamedBuffer(m_bufferID);
			glDeleteBuffers(1, &m_bufferID);
		}
		for (auto pointer : m_indexPointers)
			*pointer = 0ull;
		m_indexPointers.clear();
	}
	/** Construct a buffer.
	@param	capacity		the starting capacity (1 or more). */
	inline GL_ArrayBuffer(const size_t & capacity = 1) : m_capacity(std::max<size_t>(1ull, capacity)) {
		// WRITE ONLY, DON'T READ FROM IT (on our side, not GPU side)
		constexpr GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		const auto bufferSize = sizeof(T) * m_capacity;
		glCreateBuffers(1, &m_bufferID);
		glNamedBufferStorage(m_bufferID, bufferSize, 0, GL_DYNAMIC_STORAGE_BIT | flags);
		m_ptrContainer = (T*)glMapNamedBufferRange(m_bufferID, 0, bufferSize, flags);
	}
	/** Assignment operator, for moving 1 buffer to another. 
	@param	o				the other buffer to move to here (invalidating it). */
	inline GL_ArrayBuffer & operator=(GL_ArrayBuffer && o) noexcept {
		m_bufferID = std::move(o.m_bufferID);
		m_indexPointers = std::move(o.m_indexPointers);
		m_capacity = std::move(o.m_capacity);
		m_ptrContainer = std::move(o.m_ptrContainer);

		o.m_bufferID = 0;
		o.m_indexPointers.clear();
		o.m_capacity = 0;
		o.m_ptrContainer = nullptr;
		return *this;
	}


	// Public Methods
	/** Add a new element to this array. 
	@return					pointer to the index into this array (buffer owns pointer). */
	inline std::shared_ptr<size_t> newElement() {
		constexpr auto elementSize = sizeof(T);
		auto index = std::make_shared<size_t>(m_indexPointers.size());

		// See if we must expand this container
		if (m_indexPointers.size() >= m_capacity) {
			const auto oldSize = sizeof(T) * m_capacity;

			// Double the previous capacity
			m_capacity *= 2ull;
			const auto newSize = sizeof(T) * m_capacity;

			// Create new buffer
			constexpr GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
			GLuint newBuffer = 0;
			glCreateBuffers(1, &newBuffer);
			glNamedBufferStorage(newBuffer, newSize, 0, GL_DYNAMIC_STORAGE_BIT | flags);

			// Copy old buffer
			if (oldSize)
				glCopyNamedBufferSubData(m_bufferID, newBuffer, 0, 0, oldSize);

			// Delete old buffer
			glUnmapNamedBuffer(m_bufferID);
			glDeleteBuffers(1, &m_bufferID);

			// Migrate new buffer
			m_bufferID = newBuffer;
			m_ptrContainer = (T*)glMapNamedBufferRange(m_bufferID, 0, newSize, flags);
		}

		// Add new index to the list, return it
		m_indexPointers.push_back(index);
		return index;
	}
	/** Remove an element from this array at the index provided. 
	@param	index			pointer to the index of the element. */
	inline void removeElement(const size_t * index) {
		// Decrement the value for every index pointer PAST the input index
		for (size_t x = (*index) + 1; x < m_indexPointers.size(); ++x)
			*m_indexPointers[x] -= 1;
		delete index;
	}
	/** Functionally clears the buffer, but doesn't zero the underlying data. Retains capacity. */
	inline void clear() {
		for (auto pointer : m_indexPointers)
			*pointer = 0ull;
		m_indexPointers.clear();
	}
	/** Retrieve a reference to the element contained at the index specified.
	@param	index			index to the element desired.
	@return					reference to the element desired. */
	inline T& operator [] (const size_t & index) {
		return m_ptrContainer[index];
	}
	/** Retrieve the length of this arrayt (the number of elements in it).
	@return					the number of elemetns in this array. */
	inline size_t getLength() const {
		return m_indexPointers.size();
	}
	/** Bind this buffer to the target specified.
	@param	target			the target type of this buffer */
	inline void bindBuffer(const GLenum & target) const {
		glBindBuffer(target, m_bufferID);
	}
	/** Bind this buffer to a particular binding point for shaders.
	@param	target			the target type of this buffer
	@param	index			the binding point index to use */
	inline void bindBufferBase(const GLenum & target, const GLuint & index) const {
		glBindBufferBase(target, index, m_bufferID);
	}
	/** Bind this buffer to a particular binding point for shaders, within a given range.
	@param	target			the target type of this buffer
	@param	index			the binding point index to use
	@param	offset			the offset to start the binding at
	@param	size			the size of the buffer to bind */
	inline void bindBufferBaseRange(const GLenum & target, const GLuint & index, const GLintptr & offset, const GLsizeiptr & size) const {
		glBindBufferRange(target, index, m_bufferID, offset, size);
	}


private:
	// Private Attributes
	GLuint m_bufferID = 0;
	std::vector<std::shared_ptr<size_t>> m_indexPointers;
	size_t m_capacity = 0;
	T * m_ptrContainer = nullptr;
};

#endif // GL_ARRAYBUFFER_H