#pragma once
#ifndef PROPUPLOAD_SYSTEM_H
#define PROPUPLOAD_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Logical/components.h"
#include "Modules/Graphics/Geometry/components.h"
#include "Modules/Graphics/Geometry/Prop/PropData.h"
#include "Engine.h"

#define NUM_VERTEX_ATTRIBUTES 8


/***/
class PropUpload_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~PropUpload_System() {
		glDeleteBuffers(1, &m_vboID);
		glDeleteVertexArrays(1, &m_vaoID);
	}
	/***/
	inline PropUpload_System(Engine * engine, const std::shared_ptr<PropData> & frameData)
		: m_engine(engine), m_frameData(frameData) {
		addComponentType(Prop_Component::ID, FLAG_REQUIRED);

		// Create VBO's
		glCreateBuffers(1, &m_vboID);
		glNamedBufferStorage(m_vboID, 0, 0, GL_DYNAMIC_STORAGE_BIT);
		// Create VAO
		glCreateVertexArrays(1, &m_vaoID);
		// Enable 7 attribute locations which all source data from binding point 0
		for (unsigned int x = 0; x < NUM_VERTEX_ATTRIBUTES; ++x) {
			glEnableVertexArrayAttrib(m_vaoID, x);
			glVertexArrayAttribBinding(m_vaoID, x, 0);
		}
		// Specify how the data should be broken up, and into what layout locations in the shaders (7 attribute locations)
		glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, offsetof(SingleVertex, vertex));
		glVertexArrayAttribFormat(m_vaoID, 1, 3, GL_FLOAT, GL_FALSE, offsetof(SingleVertex, normal));
		glVertexArrayAttribFormat(m_vaoID, 2, 3, GL_FLOAT, GL_FALSE, offsetof(SingleVertex, tangent));
		glVertexArrayAttribFormat(m_vaoID, 3, 3, GL_FLOAT, GL_FALSE, offsetof(SingleVertex, bitangent));
		glVertexArrayAttribFormat(m_vaoID, 4, 2, GL_FLOAT, GL_FALSE, offsetof(SingleVertex, uv));
		glVertexArrayAttribIFormat(m_vaoID, 5, 1, GL_INT, offsetof(SingleVertex, matID));
		glVertexArrayAttribIFormat(m_vaoID, 6, 4, GL_INT, offsetof(SingleVertex, boneIDs));
		glVertexArrayAttribFormat(m_vaoID, 7, 4, GL_FLOAT, GL_FALSE, offsetof(SingleVertex, weights));
		// Specify data from the one vertex buffer to binding point 0
		glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(SingleVertex));
		
		// Share VAO for rendering purposes
		frameData->m_geometryVAOID = m_vaoID;

		// Clear state on world-unloaded
		m_engine->getModule_World().addLevelListener(m_aliveIndicator, [&](const World_Module::WorldState & state) {
			if (state == World_Module::unloaded)
				clear();
		});
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		for each (const auto & componentParam in components) {
			Prop_Component * propComponent = (Prop_Component*)componentParam[0];
			auto & offset = propComponent->m_offset;
			auto & count = propComponent->m_count;
			auto & model = propComponent->m_model;
			auto & data = model->m_data;

			if (offset == 0ull && count == 0ull && model->existsYet()) {
				// Prop hasn't been uploaded yet
				const size_t arraySize = data.m_vertices.size();
				// Check if we can fit the desired data
				tryToExpand(arraySize);

				offset = m_currentSize;
				count = arraySize;
				// No need to check fence, since we are writing to a NEW range
				glNamedBufferSubData(m_vboID, m_currentSize * sizeof(SingleVertex), arraySize * sizeof(SingleVertex), &data.m_vertices[0]);

				m_currentSize += arraySize;
				if (m_fence)
					glDeleteSync(m_fence);
				m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			}
		}
	}	


private:
	// Private Methods
	/***/
	inline void waitOnFence() {
		if (m_fence) {
			// Wait for data fence to be passed
			GLenum state = GL_UNSIGNALED;
			while (state != GL_SIGNALED && state != GL_ALREADY_SIGNALED && state != GL_CONDITION_SATISFIED)
				state = glClientWaitSync(m_fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
			glDeleteSync(m_fence);
			m_fence = nullptr;
		}
	}
	/***/
	inline void tryToExpand(const size_t & arraySize) {
		if (m_currentSize + arraySize > m_maxCapacity) {
			waitOnFence();

			// Create new set of VBO's large enough to fit old data + desired data
			m_maxCapacity += arraySize * 2;

			// Create the new VBO's
			GLuint newVBOID = 0;
			glCreateBuffers(1, &newVBOID);
			glNamedBufferStorage(newVBOID, m_maxCapacity * sizeof(SingleVertex), 0, GL_DYNAMIC_STORAGE_BIT);

			// Copy old VBO's
			glCopyNamedBufferSubData(m_vboID, newVBOID, 0, 0, m_currentSize * sizeof(SingleVertex));

			// Delete the old VBO's
			glDeleteBuffers(1, &m_vboID);

			// Overwrite old VBO ID's
			m_vboID = newVBOID;

			// Assign VAO to new VBO's
			glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(SingleVertex));
		}
	}
	/***/
	inline void clear() {
		// Reset size, and half the capacity
		m_currentSize = 0ull;
		m_maxCapacity /= 2ull;

		// Replace old vbo
		waitOnFence();
		GLuint newVBOID = 0;
		glCreateBuffers(1, &newVBOID);
		glNamedBufferStorage(newVBOID, m_maxCapacity * sizeof(SingleVertex), 0, GL_DYNAMIC_STORAGE_BIT);
		glDeleteBuffers(1, &m_vboID);
		m_vboID = newVBOID;
		glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(SingleVertex));
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	GLuint m_vaoID = 0, m_vboID = 0;
	size_t m_currentSize = 0ull, m_maxCapacity = 0ull;
	GLsync m_fence = nullptr;
	std::shared_ptr<PropData> m_frameData;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // PROPUPLOAD_SYSTEM_H