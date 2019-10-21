#pragma once
#ifndef OUTLINE_SYSTEM_H
#define OUTLINE_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Modules/Editor/Editor_M.h"
#include "Assets/Mesh.h"
#include "Assets/Shader.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Engine.h"


/** An ECS system responsible for rendering outlines of selected entities bounding objects. */
class Outline_System final : public ecsBaseSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~Outline_System() {
		// Update indicator
		*m_aliveIndicator = false;

		if (m_vboID != 0 && m_vaoID != 0) {
			glDeleteBuffers(1, &m_vboID);
			glDeleteVertexArrays(1, &m_vaoID);
		}
	}
	/** Construct this system.
	@param	engine		the currently active engine.
	@param	editor		the level editor. */
	inline Outline_System(Engine* engine, LevelEditor_Module* editor)
		: m_engine(engine), m_editor(editor) {
		// Declare component types used
		addComponentType(Selected_Component::m_ID);
		addComponentType(Transform_Component::m_ID);
		addComponentType(Prop_Component::m_ID, FLAG_OPTIONAL);
		addComponentType(Light_Component::m_ID, FLAG_OPTIONAL);

		// Preferences
		auto& preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::E_OUTLINE_SCALE, m_renderScale);
		preferences.addCallback(PreferenceState::E_OUTLINE_SCALE, m_aliveIndicator, [&](const float& f) {
			m_renderScale = f;
			});

		// Create VBO's
		glCreateBuffers(1, &m_vboID);
		glNamedBufferStorage(m_vboID, 1, 0, GL_DYNAMIC_STORAGE_BIT);

		// Create VAO
		glCreateVertexArrays(1, &m_vaoID);
		glEnableVertexArrayAttrib(m_vaoID, 0);
		glEnableVertexArrayAttrib(m_vaoID, 1);
		glVertexArrayAttribBinding(m_vaoID, 0, 0);
		glVertexArrayAttribBinding(m_vaoID, 1, 0);
		glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribFormat(m_vaoID, 1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3));
		glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(glm::vec3) * 2);

		// Assets
		m_shader = Shared_Shader(engine, "Editor//outline");
		m_cube = Shared_Mesh(engine, "//Models//cube.obj");
		m_sphere = Shared_Mesh(engine, "//Models//sphere.obj");
		m_cone = Shared_Mesh(engine, "//Models//cone.obj");
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) override final {
		if (m_shader->existsYet()) {
			// Collate all component data to generate a draw call
			const auto& camera = m_engine->getModule_Graphics().getClientCamera()->get();
			const auto pMatrix = camera->pMatrix;
			const auto vMatrix = camera->vMatrix;
			std::vector<glm::mat4> baseTransforms;
			std::vector<glm::ivec4> drawData;
			for each (const auto & componentParam in components) {
				//auto* selectedComponent = (Selected_Component*)componentParam[0];
				auto* trans = (Transform_Component*)componentParam[1];
				auto* prop = (Prop_Component*)componentParam[2];
				auto* light = (Light_Component*)componentParam[3];

				const auto tryRegisterComponentModel = [&](const ComponentHandle& componentHandle, const Shared_Mesh& mesh) {
					tryInsertModel(mesh);
					m_geometryParams.insert_or_assign(componentHandle, m_meshMap[mesh]);
				};
				if (prop && prop->m_model && prop->m_model->existsYet()) {
					tryRegisterComponentModel(prop->m_handle, prop->m_model->m_mesh);
					const auto& [offset, count] = m_geometryParams[prop->m_handle];
					drawData.push_back({ count, 1, offset, 1 });
					baseTransforms.push_back(pMatrix * vMatrix * trans->m_worldTransform.m_modelMatrix);
				}
				if (light) {
					if (light->m_type == Light_Component::Light_Type::DIRECTIONAL)
						tryRegisterComponentModel(light->m_handle, m_cube);
					else if (light->m_type == Light_Component::Light_Type::POINT)
						tryRegisterComponentModel(light->m_handle, m_sphere);
					else if (light->m_type == Light_Component::Light_Type::SPOT)
						tryRegisterComponentModel(light->m_handle, m_cone);
					const auto& [offset, count] = m_geometryParams[light->m_handle];
					drawData.push_back({ count, 1, offset, 1 });
					baseTransforms.push_back(pMatrix * vMatrix * trans->m_worldTransform.m_modelMatrix);
				}
			}

			// Write data
			m_ssboTransforms.beginWriting();
			m_ssboTransforms.write(0, sizeof(glm::mat4) * baseTransforms.size(), baseTransforms.data());
			m_indirectGeometry.write(0, sizeof(glm::ivec4) * drawData.size(), drawData.data());

			// Stencil-out the shapes themselves
			m_editor->bindFBO();
			m_shader->bind();
			m_shader->setUniform(0, 0.0f);
			m_shader->setUniform(1, camera->EyePosition);
			m_shader->setUniform(2, camera->pMatrixInverse);
			m_shader->setUniform(3, glm::vec4(0.33, 0.66, 0.99, 0.125));
			m_ssboTransforms.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
			m_indirectGeometry.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glBindVertexArray(m_vaoID);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_STENCIL_TEST);
			glDisable(GL_CULL_FACE);
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ZERO);
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			glStencilFunc(GL_ALWAYS, 1, 0xFF);
			glStencilMask(0xFF);
			glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			glMultiDrawArraysIndirect(GL_TRIANGLES, 0, GLsizei(drawData.size()), 0);

			// Render the shapes larger, cutting out previous region
			m_shader->setUniform(0, 0.01f * m_renderScale);
			m_shader->setUniform(3, glm::vec4(1, 0.8, 0.1, 1.0));
			glStencilMask(0x00);
			glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
			glMultiDrawArraysIndirect(GL_TRIANGLES, 0, GLsizei(drawData.size()), 0);

			glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			glDisable(GL_BLEND);
			glEnable(GL_DEPTH_TEST);
			glDisable(GL_STENCIL_TEST);
			glEnable(GL_CULL_FACE);
			m_ssboTransforms.endWriting();
			Shader::Release();
		}
	}


private:
	// Private Methods
	/** Attempt to insert the mesh supplied into the mesh map, failing only if it is already present.
	@param	mesh		the mesh to insert only 1 copy of. */
	inline void tryInsertModel(const Shared_Mesh& mesh) {
		if (m_meshMap.find(mesh) == m_meshMap.end()) {
			// Prop hasn't been uploaded yet
			const size_t arraySize = mesh->m_geometry.vertices.size() * (sizeof(glm::vec3) * 2);
			// Check if we can fit the desired data
			waitOnFence();
			tryToExpand(arraySize);
			auto offset = (GLuint)(m_currentSize / (sizeof(glm::vec3) * 2));
			auto count = (GLuint)(arraySize / (sizeof(glm::vec3) * 2));

			// Upload vertex data
			std::vector<glm::vec3> mergedData;
			mergedData.reserve(mesh->m_geometry.vertices.size() * 2);
			for (size_t x = 0, size = std::min(mesh->m_geometry.vertices.size(), mesh->m_geometry.normals.size()); x < size; ++x) {
				mergedData.push_back(mesh->m_geometry.vertices[x]);
				mergedData.push_back(mesh->m_geometry.normals[x]);
			}
			glNamedBufferSubData(m_vboID, m_currentSize, arraySize, mergedData.data());
			m_currentSize += arraySize;

			// Prepare fence
			m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			m_meshMap[mesh] = { offset, count };
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
			glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, (sizeof(glm::vec3) * 2));
		}
	}


	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
	float m_renderScale = 0.02f;
	Shared_Mesh m_cube, m_sphere, m_cone;
	Shared_Shader m_shader;
	DynamicBuffer m_indirectGeometry, m_ssboTransforms;
	GLuint m_vaoID = 0, m_vboID = 0;
	size_t m_currentSize = 0ull, m_maxCapacity = 256ull;
	GLsync m_fence = nullptr;
	std::map<ComponentHandle, std::pair<GLuint, GLuint>> m_geometryParams;
	std::map<Shared_Mesh, std::pair<GLuint, GLuint>> m_meshMap;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // OUTLINE_SYSTEM_H