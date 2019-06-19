#pragma once
#ifndef CAMERABUFFER_H
#define CAMERABUFFER_H

#include "Utilities/GL/glad/glad.h"
#include "glm/glm.hpp"


/** Contains the data for a single point of view, and is mapped to the GPU using a triple buffer. */
class CameraBuffer {
public:
	// Public Structs
	struct BufferStructure {
		glm::mat4 pMatrix;
		glm::mat4 vMatrix;
		glm::vec3 EyePosition; float padding1;
		glm::vec2 Dimensions;
		float FarPlane; float FOV; // These 2 values are padded out unless shader uses "Dimensions" as vec4
		constexpr static float ConstNearPlane = 0.5f;
	};


	// Public (de)Constructors
	/** Destroy the camera buffer. */
	inline ~CameraBuffer() {
		if (m_bufferID != 0) {
			glUnmapNamedBuffer(m_bufferID);
			glDeleteBuffers(1, &m_bufferID);
		}
	}
	/** Construct a camera buffer. */
	inline CameraBuffer() {
		constexpr GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		glCreateBuffers(1, &m_bufferID);
		glNamedBufferStorage(m_bufferID, sizeof(BufferStructure) * 3, 0, GL_DYNAMIC_STORAGE_BIT | flags);
		m_ptr = glMapNamedBufferRange(m_bufferID, 0, sizeof(BufferStructure) * 3, flags);
	}
	/** Move from another camera buffer. */
	inline CameraBuffer & operator=(CameraBuffer && o) noexcept {
		m_localData = (std::move(o.m_localData));
		m_bufferID = (std::move(o.m_bufferID));
		m_ptr = (std::move(o.m_ptr));

		o.m_bufferID = 0;
		o.m_ptr = 0;
		return *this;
	}


	// Public Methods
	/** Retrieve a const pointer to the underlying data structure. */
	inline const BufferStructure * operator-> () const {
		return &m_localData;
	}
	/** Retrieve a pointer to the underlying data structure. */
	inline BufferStructure * operator-> () {
		return &m_localData;
	}
	/** Retrieve a const pointer to the underlying data structure. */
	inline const BufferStructure * get() const {
		return &m_localData;
	}
	/** Retrieve a pointer to the underlying data structure. */
	inline BufferStructure * get() {
		return &m_localData;
	}
	/** Wait for the current fence to pass, for the current frame index.*/
	inline void beginWriting() {
		if (m_fence[m_writeIndex] != nullptr)
			while (1) {
				GLenum waitReturn = glClientWaitSync(m_fence[m_writeIndex], GL_SYNC_FLUSH_COMMANDS_BIT, 1);
				if (waitReturn == GL_SIGNALED || waitReturn == GL_ALREADY_SIGNALED || waitReturn == GL_CONDITION_SATISFIED) {
					glDeleteSync(m_fence[m_writeIndex]);
					m_fence[m_writeIndex] = nullptr;
					return;
				}
			}
	}
	/** Create a fence for the current point in time.*/
	inline void endWriting() {
		if (m_fence[m_writeIndex])
			glDeleteSync(m_fence[m_writeIndex]);
		m_fence[m_writeIndex] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		m_writeIndex = (m_writeIndex + 1) % 3;
	}
	/** Commit the changes held in the local data, to the GPU, for a given frame index. */
	inline void pushChanges() {
		reinterpret_cast<BufferStructure*>(m_ptr)[m_writeIndex] = m_localData;
	}
	/** Bind the GPU representation of this data, for a given frame index. 
	@param	targetIndex		the binding point for the buffer in the shader. */
	inline void bind(const GLuint & targetIndex) const {
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, targetIndex, m_bufferID, sizeof(BufferStructure) * m_writeIndex, sizeof(BufferStructure));
	}


private:
	// Private Attributes
	BufferStructure m_localData;
	int m_writeIndex = 0;
	GLuint m_bufferID = 0;
	void * m_ptr = nullptr;
	GLsync m_fence[3] = { nullptr, nullptr, nullptr };
};

#endif // CAMERABUFFER_H