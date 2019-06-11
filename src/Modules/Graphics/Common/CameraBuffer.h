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
	/** Create a fence at for the current point in time.
	@param	frameIndex		which frame of this triple buffer to use (0-2). */
	inline void lockFrame(const size_t & frameIndex) {
		if (m_fence[frameIndex])
			glDeleteSync(m_fence[frameIndex]);
		m_fence[frameIndex] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	}
	/** Wait for the current fence to pass, for the current frame index.
	@param	frameIndex		which frame of this triple buffer to use (0-2). */
	inline void waitFrame(const size_t & frameIndex) {
		if (m_fence[frameIndex])
			while (1) {
				GLenum waitReturn = glClientWaitSync(m_fence[frameIndex], GL_SYNC_FLUSH_COMMANDS_BIT, 1);
				if (waitReturn == GL_ALREADY_SIGNALED || waitReturn == GL_CONDITION_SATISFIED)
					return;
			}
	}
	/** Commit the changes held in the local data, to the GPU, for a given frame index. 
	@param	frameIndex		which frame of this triple buffer to use (0-2). */
	inline void pushChanges(const size_t & frameIndex) {
		reinterpret_cast<BufferStructure*>(m_ptr)[frameIndex] = m_localData;
	}
	/** Bind the GPU representation of this data, for a given frame index. 
	@param	targetIndex		the binding point for the buffer in the shader.
	@param	frameIndex		which frame of this triple buffer to use (0-2). */
	inline void bind(const GLuint & targetIndex, const size_t & frameIndex) const {
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, targetIndex, m_bufferID, sizeof(BufferStructure) * frameIndex, sizeof(BufferStructure));
	}


private:
	// Private Attributes
	BufferStructure m_localData;
	GLuint m_bufferID = 0;
	void * m_ptr = nullptr;
	GLsync m_fence[3] = { nullptr, nullptr, nullptr };
};

#endif // CAMERABUFFER_H