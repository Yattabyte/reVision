#pragma once
#ifndef PROPUPLOAD_SYSTEM_H
#define PROPUPLOAD_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Modules/Graphics/Geometry/Prop/PropData.h"
#include "Engine.h"

#define NUM_VERTEX_ATTRIBUTES 8


/** An ECS system responsible for uploading prop data to the GPU, such as geometrical data and material textures. */
class PropUpload_System final : public ecsBaseSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~PropUpload_System() {
		glDeleteBuffers(1, &m_vboID);
		glDeleteVertexArrays(1, &m_vaoID);
	}
	/** Construct this system. */
	inline PropUpload_System(Engine* engine, const std::shared_ptr<PropData>& frameData)
		: m_engine(engine), m_frameData(frameData) {
		addComponentType(Prop_Component::m_ID, FLAG_REQUIRED);

		// Create VBO's
		glCreateBuffers(1, &m_vboID);
		glNamedBufferStorage(m_vboID, 1, 0, GL_DYNAMIC_STORAGE_BIT);
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

		// Preference Values
		m_engine->getPreferenceState().getOrSetValue(PreferenceState::C_MATERIAL_SIZE, m_materialSize);

		// Size-dependent variable set up
		m_maxMips = GLsizei(floor(log2f(float(m_materialSize)) + 1.0f));
		glGetIntegerv(GL_MAX_SPARSE_ARRAY_TEXTURE_LAYERS, &m_maxTextureLayers);

		// Initialize the material array
		glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_matID);
		glTextureParameterf(m_matID, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
		glTextureParameteri(m_matID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_matID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_matID, GL_TEXTURE_SPARSE_ARB, GL_TRUE);
		glTextureStorage3D(m_matID, m_maxMips, GL_RGBA16F, m_materialSize, m_materialSize, m_maxTextureLayers);

		// Share material array for rendering purposes
		frameData->m_materialArrayID = m_matID;
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) override final {
		for each (const auto & componentParam in components) {
			Prop_Component* propComponent = (Prop_Component*)componentParam[0];
			auto& offset = propComponent->m_offset;
			auto& count = propComponent->m_count;
			auto& materialID = propComponent->m_materialID;
			auto& model = propComponent->m_model;
			auto& data = model->m_data;

			// Try to upload model data
			if (!model)
				model = Shared_Model(m_engine, propComponent->m_modelName);
			if (!propComponent->m_uploadModel && model->existsYet()) {
				tryInsertModel(model);
				offset = m_modelMap[model].first;
				count = m_modelMap[model].second;
				propComponent->m_uploadModel = true;
			}

			// Try to upload material data
			if (!propComponent->m_uploadMaterial && model->existsYet() && model->m_materialArray->existsYet()) {
				// Get spot in the material array
				tryInsertMaterial(model->m_materialArray);
				materialID = m_materialMap[model->m_materialArray];

				// Prepare fence
				propComponent->m_uploadMaterial = true;
			}
		}
	}


private:
	// Private Methods
	/** Attempt to insert the model supplied into the model map, failing only if it is already present.
	@param	model		the model to insert only 1 copy of. */
	inline void tryInsertModel(const Shared_Model& model) {
		if (m_modelMap.find(model) == m_modelMap.end()) {
			// Prop hasn't been uploaded yet
			const size_t arraySize = model->m_data.m_vertices.size() * sizeof(SingleVertex);
			// Check if we can fit the desired data
			waitOnFence();
			tryToExpand(arraySize);
			auto offset = (GLuint)(m_currentSize / sizeof(SingleVertex));
			auto count = (GLuint)(arraySize / sizeof(SingleVertex));

			// Upload vertex data
			glNamedBufferSubData(m_vboID, m_currentSize, arraySize, &model->m_data.m_vertices[0]);
			m_currentSize += arraySize;

			// Prepare fence
			m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			m_modelMap[model] = { offset, count };
		}
	}
	/** Wait on the prop fence if it still exists. */
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
	/** Attempt to expand the props' vertex buffer if it isn't large enough.
	@param	arraySize	the new size to use. */
	inline void tryToExpand(const size_t& arraySize) {
		if (m_currentSize + arraySize > m_maxCapacity) {
			// Create new set of VBO's large enough to fit old data + desired data
			m_maxCapacity += arraySize * 2;

			// Create the new VBO's
			GLuint newVBOID = 0;
			glCreateBuffers(1, &newVBOID);
			glNamedBufferStorage(newVBOID, m_maxCapacity, 0, GL_DYNAMIC_STORAGE_BIT);

			// Copy old VBO's
			auto fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			glCopyNamedBufferSubData(m_vboID, newVBOID, 0, 0, m_currentSize);

			// Delete the old VBO's
			GLenum state = GL_UNSIGNALED;
			while (state != GL_SIGNALED && state != GL_ALREADY_SIGNALED && state != GL_CONDITION_SATISFIED)
				state = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
			glDeleteSync(fence);
			glDeleteBuffers(1, &m_vboID);

			// Overwrite old VBO ID's
			m_vboID = newVBOID;

			// Assign VAO to new VBO's
			glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(SingleVertex));
		}
	}
	/** Attempt to insert the material supplied into the material map, failing only if it is already present.
	@param	material	the material to insert only 1 copy of. */
	inline void tryInsertMaterial(const Shared_Material& material) {
		if (m_materialMap.find(material) == m_materialMap.end()) {
			// Get spot in the material array
			const auto imageCount = (GLsizei)((material->m_textures.size() / MAX_PHYSICAL_IMAGES) * MAX_DIGITAL_IMAGES);
			auto materialID = (GLuint)m_matCount;
			m_matCount += imageCount;
			if (m_matCount >= m_maxTextureLayers)
				m_engine->getManager_Messages().error("Out of room for more materials!");

			// Upload material data
			GLuint pboID = 0;
			glCreateBuffers(1, &pboID);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboID);
			glNamedBufferStorage(pboID, size_t(m_materialSize) * size_t(m_materialSize) * size_t(imageCount) * 4ull, material->m_materialData, GL_DYNAMIC_STORAGE_BIT);
			for (int x = 0; x < m_maxMips; ++x) {
				const GLsizei mipsize = (GLsizei)std::max(1.0f, (floor(m_materialSize / pow(2.0f, (float)x))));
				glTexturePageCommitmentEXT(m_matID, x, 0, 0, materialID, mipsize, mipsize, imageCount, GL_TRUE);
			}
			glTextureSubImage3D(m_matID, 0, 0, 0, materialID, m_materialSize, m_materialSize, imageCount, GL_RGBA, GL_UNSIGNED_BYTE, (void*)0);
			glGenerateTextureMipmap(m_matID);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			glDeleteBuffers(1, &pboID);
			m_materialMap[material] = materialID;
		}
	}
	/** Clear all data held by this system. */
	inline void clear() {
		// Reset size, and half the capacity
		m_currentSize = 0ull;
		m_maxCapacity = std::max<size_t>(256ull, m_maxCapacity / 2ull);
		m_modelMap.clear();

		// Replace old vbo
		waitOnFence();
		GLuint newVBOID = 0;
		glCreateBuffers(1, &newVBOID);
		glNamedBufferStorage(newVBOID, m_maxCapacity, 0, GL_DYNAMIC_STORAGE_BIT);
		glDeleteBuffers(1, &m_vboID);
		m_vboID = newVBOID;
		glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(SingleVertex));

		// Reset materials
		m_matCount = 0;
		m_materialMap.clear();
	}


	// Private Attributes
	Engine* m_engine = nullptr;
	GLuint m_vaoID = 0, m_vboID = 0, m_matID;
	size_t m_currentSize = 0ull, m_maxCapacity = 256ull, m_matCount = 0ull;
	GLsizei m_materialSize = 512u;
	GLint m_maxTextureLayers = 6, m_maxMips = 1;
	GLsync m_fence = nullptr;
	std::map<Shared_Model, std::pair<GLuint, GLuint>> m_modelMap;
	std::map<Shared_Material, GLuint> m_materialMap;
	std::shared_ptr<PropData> m_frameData;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // PROPUPLOAD_SYSTEM_H