#pragma once
#ifndef CAMERABUFFER_H
#define CAMERABUFFER_H

#include "Utilities/GL/glad/glad.h"
#include "glm/glm.hpp"


/** Contains the data for a single point of view, and is mapped to the GPU using a triple buffer. */
class CameraBuffer {
public:
	// Public Structs
	constexpr static float ConstNearPlane = 0.5f;
	struct CamStruct {
		glm::mat4 pMatrix;
		glm::mat4 vMatrix;
		glm::vec3 EyePosition; float padding1;
		glm::vec2 Dimensions; 
		float NearPlane = ConstNearPlane;
		float FarPlane;
		float FOV;
	};


	// Public (de)Constructors
	/** Destroy the camera buffer. */
	inline ~CameraBuffer() {
		// Wait on all 3 fences before deleting
		for (int x = 0; x < 3; ++x)
			while (m_fence[x] != nullptr) {
				GLenum waitReturn = glClientWaitSync(m_fence[x], GL_SYNC_FLUSH_COMMANDS_BIT, 1);
				if (waitReturn == GL_SIGNALED || waitReturn == GL_ALREADY_SIGNALED || waitReturn == GL_CONDITION_SATISFIED) {
					glDeleteSync(m_fence[x]);
					m_fence[x] = nullptr;
					break;
				}
			}

		glUnmapNamedBuffer(m_bufferID[0]);
		glUnmapNamedBuffer(m_bufferID[1]);
		glUnmapNamedBuffer(m_bufferID[2]);
		glDeleteBuffers(3, m_bufferID);
	}
	/** Construct a camera buffer. */
	inline CameraBuffer() {
		constexpr GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		glCreateBuffers(3, m_bufferID);
		for (int x = 0; x < 3; ++x) {
			glNamedBufferStorage(m_bufferID[x], sizeof(CamStruct), 0, GL_DYNAMIC_STORAGE_BIT | flags);
			m_bufferPtr[x] = glMapNamedBufferRange(m_bufferID[x], 0, sizeof(CamStruct), flags);
		}
	}
	/** Move from another camera buffer. */
	inline CameraBuffer & operator=(CameraBuffer && o) noexcept {
		m_localData = (std::move(o.m_localData));
		m_bufferID[0] = (std::move(o.m_bufferID[0]));
		m_bufferID[1] = (std::move(o.m_bufferID[1]));
		m_bufferID[2] = (std::move(o.m_bufferID[2]));
		m_bufferPtr[0] = (std::move(o.m_bufferPtr[0]));
		m_bufferPtr[1] = (std::move(o.m_bufferPtr[1]));
		m_bufferPtr[2] = (std::move(o.m_bufferPtr[2]));

		o.m_bufferID[0] = 0;
		o.m_bufferID[1] = 0;
		o.m_bufferID[2] = 0;
		o.m_bufferPtr[0] = nullptr;
		o.m_bufferPtr[1] = nullptr;
		o.m_bufferPtr[2] = nullptr;
		return *this;
	}


	// Public Methods
	/** Retrieve a const pointer to the underlying data structure. */
	inline const CamStruct * operator-> () const {
		return &m_localData;
	}
	/** Retrieve a pointer to the underlying data structure. */
	inline CamStruct * operator-> () {
		return &m_localData;
	}
	/** Retrieve a const pointer to the underlying data structure. */
	inline const CamStruct * get() const {
		return &m_localData;
	}
	/** Retrieve a pointer to the underlying data structure. */
	inline CamStruct * get() {
		return &m_localData;
	}
	/***/
	inline void replace(const CamStruct & data) {
		m_localData = data;
	}
	/** Wait for the current fence to pass, for the current frame index. */
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
	/** Create a fence for the current point in time. */
	inline void endWriting() {
		if (!m_fence[m_writeIndex]) {
			m_fence[m_writeIndex] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			m_writeIndex = (m_writeIndex + 1) % 3;
		}
	}
	/** Commit the changes held in the local data, to the GPU, for a given frame index. */
	inline void pushChanges() {
		*(CamStruct*)(m_bufferPtr[m_writeIndex]) = m_localData;
	}
	/** Bind the GPU representation of this data, for a given frame index.
	@param	targetIndex		the binding point for the buffer in the shader. */
	inline void bind(const GLuint & targetIndex) const {
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, targetIndex, m_bufferID[m_writeIndex], 0, sizeof(CamStruct));
	}


private:
	// Private Attributes
	CamStruct m_localData;
	int m_writeIndex = 0;
	GLuint m_bufferID[3] = { 0,0,0 };
	void * m_bufferPtr[3] = { nullptr, nullptr, nullptr };
	GLsync m_fence[3] = { nullptr, nullptr, nullptr };
};

#endif // CAMERABUFFER_H