#pragma once
#ifndef DIRECT_TECHNIQUE_H
#define DIRECT_TECHNIQUE_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Lighting/Direct/DirectData.h"
#include "Modules/Graphics/Lighting/Direct/DirectVisibility_System.h"
#include "Modules/Graphics/Lighting/Direct/DirectSync_System.h"
#include "Modules/ECS/ecsSystem.h"
#include "Assets/Shader.h"
#include "Assets/Mesh.h"
#include "Utilities/GL/StaticMultiBuffer.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Engine.h"
#include <random>


/** A core lighting technique responsible for direct-diffuse and direct-specular lighting. */
class Direct_Technique final : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Destructor. */
	inline ~Direct_Technique() {
		// Update indicator
		*m_aliveIndicator = false;

		// Free vertex array and buffer objects
		if (m_geometryReady && m_vboID != 0 && m_vaoID != 0) {
			glDeleteBuffers(1, &m_vboID);
			glDeleteVertexArrays(1, &m_vaoID);
		}
	}
	/** Constructor. */
	inline Direct_Technique(Engine* engine, const std::shared_ptr<ShadowData>& shadowData, const std::shared_ptr<Camera>& clientCamera, const std::shared_ptr<std::vector<Camera*>>& cameras)
		: m_engine(engine), m_cameras(cameras), Graphics_Technique(PRIMARY_LIGHTING) {
		m_frameData = std::make_shared<Direct_Light_Data>();
		m_frameData->clientCamera = clientCamera;
		m_frameData->shadowData = shadowData;

		// Auxiliary Systems
		m_auxilliarySystems.makeSystem<DirectVisibility_System>(m_frameData);
		m_auxilliarySystems.makeSystem<DirectSync_System>(m_frameData);

		// Asset Loading
		m_shader_Lighting = Shared_Shader(engine, "Core\\Light\\Direct");
		m_shader_Stencil = Shared_Shader(engine, "Core\\Light\\Stencil");
		m_shapeCube = Shared_Mesh(engine, "//Models//cube.obj");
		m_shapeSphere = Shared_Mesh(engine, "//Models//sphere.obj");
		m_shapeHemisphere = Shared_Mesh(engine, "//Models//hemisphere.obj");

		// Asset-finished callbacks
		m_shapeCube->addCallback(m_aliveIndicator, [&]() { registerLightShapes(); });
		m_shapeSphere->addCallback(m_aliveIndicator, [&]() { registerLightShapes(); });
		m_shapeHemisphere->addCallback(m_aliveIndicator, [&]() { registerLightShapes(); });
	}


	// Public Interface Implementations
	inline virtual void prepareForNextFrame(const float& deltaTime) override final {
		m_frameData->lightBuffer.endReading();
		m_drawIndex = 0;
	}
	inline virtual void updateCache(const float& deltaTime, ecsWorld& world) override final {
		// Link together the dimensions of view info to that of the viewport vectors
		m_frameData->viewInfo.resize(m_cameras->size());
		world.updateSystems(m_auxilliarySystems, deltaTime);
	}
	inline virtual void renderTechnique(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::vector<std::pair<int, int>>& perspectives) override final {
		// Exit Early
		if (m_enabled && m_geometryReady && m_frameData->viewInfo.size() && m_shapeCube->existsYet() && m_shader_Lighting->existsYet()) {
			if (m_drawIndex >= m_drawData.size())
				m_drawData.resize(size_t(m_drawIndex) + 1ull);
			// Accumulate all visibility info for the cameras passed in
			std::vector<glm::ivec2> camIndices;
			std::vector<GLint> lightIndices;
			std::vector<glm::ivec4> drawData;
			for (auto& [camIndex, layer] : perspectives) {
				const std::vector<glm::ivec2> tempIndices(m_frameData->viewInfo[camIndex].lightIndices.size(), { camIndex, layer });
				camIndices.insert(camIndices.end(), tempIndices.begin(), tempIndices.end());
				lightIndices.insert(lightIndices.end(), m_frameData->viewInfo[camIndex].lightIndices.begin(), m_frameData->viewInfo[camIndex].lightIndices.end());
				for each (const auto & type in m_frameData->viewInfo[camIndex].lightTypes) {
					if (type == Light_Component::Light_Type::DIRECTIONAL)
						drawData.push_back({ m_cubeParams.second, 1, m_cubeParams.first, 0 });
					else if (type == Light_Component::Light_Type::POINT)
						drawData.push_back({ m_sphereParams.second, 1, m_sphereParams.first, 0 });
					else if (type == Light_Component::Light_Type::SPOT)
						drawData.push_back({ m_hemisphereParams.second, 1, m_hemisphereParams.first, 0 });
				}
			}

			// Render lights
			if (lightIndices.size()) {
				// Write accumulated data
				auto& drawBuffer = m_drawData[m_drawIndex];
				drawBuffer.bufferCamIndex.beginWriting();
				drawBuffer.visLights.beginWriting();
				drawBuffer.indirectShape.beginWriting();
				drawBuffer.bufferCamIndex.write(0, sizeof(glm::ivec2) * camIndices.size(), camIndices.data());
				drawBuffer.visLights.write(0, sizeof(GLuint) * lightIndices.size(), lightIndices.data());
				drawBuffer.indirectShape.write(0, sizeof(glm::ivec4) * drawData.size(), drawData.data());
				drawBuffer.bufferCamIndex.endWriting();
				drawBuffer.visLights.endWriting();
				drawBuffer.indirectShape.endWriting();

				// Prepare rendering state
				glEnable(GL_STENCIL_TEST);
				glEnable(GL_BLEND);
				glBlendEquation(GL_FUNC_ADD);
				glBlendFunc(GL_ONE, GL_ONE);
				glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
				glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
				glStencilMask(0xFF);

				// Draw only into depth-stencil buffer
				m_shader_Stencil->bind();															// Shader (point)
				viewport->m_gfxFBOS->bindForWriting("LIGHTING");									// Ensure writing to lighting FBO
				viewport->m_gfxFBOS->bindForReading("GEOMETRY", 0);									// Read from Geometry FBO
				glBindTextureUnit(4, m_frameData->shadowData->shadowFBO.m_texDepth);				// Shadow map(linear depth texture)
				m_drawData[m_drawIndex].bufferCamIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
				m_drawData[m_drawIndex].visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);		// SSBO visible light indices
				m_frameData->lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
				m_drawData[m_drawIndex].indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);			// Draw call buffer
				glBindVertexArray(m_vaoID);
				glDepthMask(GL_FALSE);
				glEnable(GL_DEPTH_TEST);
				glDisable(GL_CULL_FACE);
				glClear(GL_STENCIL_BUFFER_BIT);
				glStencilFunc(GL_ALWAYS, 0, 0);
				glMultiDrawArraysIndirect(GL_TRIANGLES, 0, GLsizei(drawData.size()), 0);

				// Now draw into color buffers
				m_shader_Lighting->bind();
				m_shader_Lighting->setUniform(0, m_frameData->shadowData->shadowSizeRCP);
				glDisable(GL_DEPTH_TEST);
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);
				glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
				glMultiDrawArraysIndirect(GL_TRIANGLES, 0, GLsizei(drawData.size()), 0);

				// Revert Rendering state
				glClear(GL_STENCIL_BUFFER_BIT);
				glCullFace(GL_BACK);
				glDepthMask(GL_TRUE);
				glBlendFunc(GL_ONE, GL_ZERO);
				glDisable(GL_BLEND);
				glDisable(GL_STENCIL_TEST);
				drawBuffer.bufferCamIndex.endReading();
				drawBuffer.visLights.endReading();
				drawBuffer.indirectShape.endReading();
				Shader::Release();

				m_drawIndex++;
			}
		}
	}


private:
	// Private Methods
	/***/
	inline void registerLightShapes() {
		if (m_shapeCube && m_shapeCube->existsYet() && m_shapeSphere && m_shapeSphere->existsYet() && m_shapeHemisphere && m_shapeHemisphere->existsYet()) {
			// Create a container to store all vertices
			std::vector<glm::vec3> combinedData;
			combinedData.reserve(m_shapeCube->m_geometry.vertices.size() + m_shapeSphere->m_geometry.vertices.size() + m_shapeHemisphere->m_geometry.vertices.size());

			// Join all vertex sets together, note the offset and count of each
			m_cubeParams = { (unsigned int)combinedData.size(),  (unsigned int)m_shapeCube->m_geometry.vertices.size() };
			combinedData.insert(combinedData.end(), m_shapeCube->m_geometry.vertices.cbegin(), m_shapeCube->m_geometry.vertices.cend());
			m_sphereParams = { (unsigned int)combinedData.size(),  (unsigned int)m_shapeSphere->m_geometry.vertices.size() };
			combinedData.insert(combinedData.end(), m_shapeSphere->m_geometry.vertices.cbegin(), m_shapeSphere->m_geometry.vertices.cend());
			m_hemisphereParams = { (unsigned int)combinedData.size(),  (unsigned int)m_shapeHemisphere->m_geometry.vertices.size() };
			combinedData.insert(combinedData.end(), m_shapeHemisphere->m_geometry.vertices.cbegin(), m_shapeHemisphere->m_geometry.vertices.cend());

			// Create VBO's
			glCreateBuffers(1, &m_vboID);
			glNamedBufferStorage(m_vboID, GLsizeiptr(sizeof(glm::vec3) * combinedData.size()), &combinedData[0], GL_CLIENT_STORAGE_BIT);

			// Create VAO
			glCreateVertexArrays(1, &m_vaoID);
			glEnableVertexArrayAttrib(m_vaoID, 0);
			glVertexArrayAttribBinding(m_vaoID, 0, 0);
			glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, 0);
			glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(glm::vec3));
			m_geometryReady = true;
		}
	}


	// Private Attributes
	Engine* m_engine = nullptr;
	Shared_Shader m_shader_Lighting, m_shader_Stencil;
	Shared_Mesh m_shapeCube, m_shapeSphere, m_shapeHemisphere;
	std::pair<GLuint, GLuint> m_cubeParams, m_sphereParams, m_hemisphereParams;
	bool m_geometryReady = false;
	GLuint m_vaoID = 0, m_vboID = 0;
	struct DrawData {
		DynamicBuffer<> bufferCamIndex, visLights, indirectShape;
	};
	int m_drawIndex = 0;
	std::vector<DrawData> m_drawData;
	ecsSystemList m_auxilliarySystems;

	// Shared Attributes
	std::shared_ptr<Direct_Light_Data> m_frameData;
	std::shared_ptr<std::vector<Camera*>> m_cameras;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // DIRECT_TECHNIQUE_H