#pragma once
#ifndef VECTORBUFFER_H
#define VECTORBUFFER_H

#include "GL\glew.h"
#include <utility>


/** An interface for an opengl buffer, handled like an stl vector */
class GL_Vector {
public:
	// Public Methods
	/** Bind this buffer.
	@param	target	the target type of this buffer */
	virtual void bindBuffer(const GLenum & target) const = 0;
	/** Bind this buffer to a particular binding point for shaders.
	@param	target	the target type of this buffer
	@param	index	the binding point index to use */
	virtual void bindBufferBase(const GLenum & target, const GLuint & index) const = 0;
	/** Bind this buffer to a particular binding point for shaders, within a given range.
	@param	target	the target type of this buffer
	@param	index	the binding point index to use
	@param	offset	the offset to start the binding at
	@param	size	the size of the buffer to bind */
	virtual void bindBufferBaseRange(const GLenum & target, const GLuint & index, const GLintptr & offset, const GLsizeiptr & size) const = 0;
};

/** An element within the Vector Buffer*/
template <typename T>
struct VB_Element {
	GLuint index;
	T * data;
	GLsync fence = nullptr;
	T* operator->() { 
		return data; 
	}
	void lock() {
		/*if (fence)
			glDeleteSync(fence);
		fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0); */
	}
	void wait() {
		/*if (fence) {
			while (true) {
				GLenum state = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
				if (state == GL_SIGNALED || state == GL_ALREADY_SIGNALED || state == GL_CONDITION_SATISFIED)
					return;
			}
		}*/
	}
};

/** An opengl buffer handled like an stl vector. */
template <typename T>
class VectorBuffer : public GL_Vector {
public:
	// Public (de)Constructors
	~VectorBuffer() {
		if (m_bufferID != 0) {
			glUnmapNamedBuffer(m_bufferID);
			glDeleteBuffers(1, &m_bufferID);
		}
	}
	/** Default Constructor. */
	VectorBuffer() {
		constexpr GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		glCreateBuffers(1, &m_bufferID);
		glNamedBufferStorage(m_bufferID, m_maxCapacity, 0, GL_DYNAMIC_STORAGE_BIT | flags);
		m_ptrContainer = glMapNamedBufferRange(m_bufferID, 0, m_maxCapacity, flags);
	}
	/** Explicit Constructor. */
	explicit VectorBuffer(const GLsizeiptr & sizeHint, const GLint & offsetAlignment) : m_maxCapacity(sizeHint), m_offsetAlignment(offsetAllighnment) {}
	/** Move gl object from 1 instance to another. */
	VectorBuffer & operator=(VectorBuffer && o) noexcept {
		m_count = (std::move(o.m_count));
		m_indexPointers = (std::move(o.m_indexPointers));
		m_bufferID = (std::move(o.m_bufferID));
		m_ptrContainer = (std::move(o.m_ptrContainer));
		m_maxCapacity = (std::move(o.m_maxCapacity));
		m_offsetAlignment = (std::move(o.m_offsetAlignment));

		o.m_count = 0;
		o.m_indexPointers = 0;
		o.m_bufferID = 0;
		o.m_ptrContainer = 0;
		o.m_maxCapacity = 0;
		o.m_offsetAlignment = 0;
		return *this;
	}
		

	// Public Methods	
	/** Add an element to this buffers list.
	@param	uboIndex	the element to add to this list
	@return	the current buffer pointer */
	VB_Element<T> * newElement() {
		const GLsizeiptr elementSize = sizeof(T);
		expandToFit(m_count * (elementSize + m_offsetAlignment), elementSize);

		VB_Element<T> * element = new VB_Element<T>();
		element->index = m_count++;
		unsigned char * bytePtr = reinterpret_cast<unsigned char*>(m_ptrContainer) + (element->index * m_offsetAlignment);
		element->data = &reinterpret_cast<T*>(bytePtr)[element->index];
		m_elements.push_back(element);
		return element;
	}	
	/** Remove an element from this buffers list. *
 	@param	uboIndex	the element to remove from this list */
	void removeElement(const unsigned int * uboIndex) {
		replaceWithEnd(uboIndex);
	}
	VB_Element<T> * getElement(const GLuint & index) {
		return m_elements[index];
	}
	void setOffsetAlignment(const GLint & offsetAlignment) {
		m_offsetAlignment = offsetAlignment;

		// Update all buffer references
		for each (VB_Element<T>* element in m_elements) {
			unsigned char * bytePtr = reinterpret_cast<unsigned char*>(m_ptrContainer) + (element->index * m_offsetAlignment);
			element->data = &reinterpret_cast<T*>(bytePtr)[element->index];
		}
	}
	const GLint getOffsetAlignment() {
		return m_offsetAlignment;
	}


	// Interface Implementation
	virtual void bindBuffer(const GLenum & target) const override {
		glBindBuffer(target, m_bufferID);
	}
	virtual void bindBufferBase(const GLenum & target, const GLuint & index) const override {
		glBindBufferBase(target, index, m_bufferID);
	}
	virtual void bindBufferBaseRange(const GLenum & target, const GLuint & index, const GLintptr & offset, const GLsizeiptr & size) const override {
		glBindBufferRange(target, index, m_bufferID, offset, size );
	}


private:
	// Private Methods	
	/** Expands this buffer's container if it can't fit the specified range to write into
	@note Technically creates a new a new buffer to replace the old one and copies the old data
	@param	offset	byte offset from the beginning
	@param	size	the size of the data to write */
	void expandToFit(const GLsizeiptr & offset, const GLsizeiptr & size) {
		if (offset + size > m_maxCapacity) {
			// Create new buffer large enough to fit old data + new data
			const GLsizeiptr oldSize = m_maxCapacity;
			m_maxCapacity += offset + (size * 2);

			// Create new buffer
			constexpr GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
			GLuint newBuffer = 0;
			glCreateBuffers(1, &newBuffer);
			glNamedBufferStorage(newBuffer, m_maxCapacity, 0, GL_DYNAMIC_STORAGE_BIT | flags);

			// Copy old buffer
			if (oldSize)
				glCopyNamedBufferSubData(m_bufferID, newBuffer, 0, 0, oldSize);

			// Delete old buffer
			glUnmapNamedBuffer(m_bufferID);
			glDeleteBuffers(1, &m_bufferID);

			// Migrate new buffer
			m_bufferID = newBuffer;
			m_ptrContainer = glMapNamedBufferRange(m_bufferID, 0, m_maxCapacity, flags);

			// Update all buffer references
			for each (VB_Element<T>* element in m_elements) {
				unsigned char * bytePtr = reinterpret_cast<unsigned char*>(m_ptrContainer) + (element->index * m_offsetAlignment);
				element->data = &reinterpret_cast<T*>(bytePtr)[element->index];
			}
		}
	}
	/** @todo */
	void replaceWithEnd(const unsigned int * uboIndex) {
		/*// Migrate last element of array into this index, replacing it.
		if ((*uboIndex) < (m_elements.size() - 1)) {
			// Move the pointer from the last element of the list to the spot we are deleting
			unsigned int * lastIndex = m_indexPointers.back();
			m_indexPointers.at(*uboIndex) = lastIndex;
			m_indexPointers.pop_back();

			// Move the memory from the last index to the old index
			T * oldData = &reinterpret_cast<T*>(m_ptrContainer)[*uboIndex];
			T * endData = &reinterpret_cast<T*>(m_ptrContainer)[*lastIndex];
			*oldData = *endData;

			// Ensure that the pointers hold the right VALUE for the index the represent
			*lastIndex = *uboIndex;
		}
		// This element is the last element, remove it.
		else {
			m_indexPointers.pop_back();
		}
		m_count--;*/
	}


	// Private Attributes
	unsigned int m_count = 0;
	GLuint m_bufferID = 0;
	void * m_ptrContainer = nullptr;
	GLsizeiptr m_maxCapacity = 256;
	GLint m_offsetAlignment = 0;
	std::vector<VB_Element<T>*> m_elements;
};

#endif // VECTORBUFFER_H