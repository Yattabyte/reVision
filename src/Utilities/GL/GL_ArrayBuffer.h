#pragma once
#ifndef GL_ARRAYBUFFER_H
#define GL_ARRAYBUFFER_H

#include "Utilities/GL/glad/glad.h"
#include <vector>

/***/
template <typename T>
class GL_ArrayBuffer {
public:
	// Public (de)Constructors
	/***/
	inline ~GL_ArrayBuffer() {
		if (m_bufferID != 0) {
			glUnmapNamedBuffer(m_bufferID);
			glDeleteBuffers(1, &m_bufferID);
		}
	}
	/***/
	inline GL_ArrayBuffer(const size_t & capacity = 1) : m_capacity(capacity) {
		// WRITE ONLY, DON'T READ FROM IT (on our side, not GPU side)
		constexpr GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		const auto bufferSize = sizeof(T) * m_capacity;
		glCreateBuffers(1, &m_bufferID);
		glNamedBufferStorage(m_bufferID, bufferSize, 0, GL_DYNAMIC_STORAGE_BIT | flags);
		m_ptrContainer = (T*)glMapNamedBufferRange(m_bufferID, 0, bufferSize, flags);
	}
	/***/
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
	/***/
	inline size_t * newElement() {
		constexpr auto elementSize = sizeof(T);
		size_t * index = new size_t(m_indexPointers.size());

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
	/***/
	inline void removeElement(const size_t * index) {
		// Decrement the value for every index pointer PAST the input index
		for (size_t x = (*index) + 1; x < m_indexPointers.size(); ++x)
			*m_indexPointers[x] -= 1;
		delete index;
	}
	/***/
	inline T& operator [] (const size_t & index) {
		return m_ptrContainer[index];
	}
	/***/
	inline size_t getLength() const {
		return m_indexPointers.size();
	}
	/***/
	inline void bindBuffer(const GLenum & target) const {
		glBindBuffer(target, m_bufferID);
	}
	/***/
	inline void bindBufferBase(const GLenum & target, const GLuint & index) const {
		glBindBufferBase(target, index, m_bufferID);
	}
	/***/
	inline void bindBufferBaseRange(const GLenum & target, const GLuint & index, const GLintptr & offset, const GLsizeiptr & size) const {
		glBindBufferRange(target, index, m_bufferID, offset, size);
	}


private:
	// Private Attributes
	GLuint m_bufferID = 0;
	std::vector<size_t*> m_indexPointers;
	size_t m_capacity = 0;
	T * m_ptrContainer = nullptr;
};

#endif // GL_ARRAYBUFFER_H