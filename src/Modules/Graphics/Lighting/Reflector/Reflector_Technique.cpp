#include "Modules/Graphics/Lighting/Reflector/Reflector_Technique.h"
#include "Modules/Graphics/Lighting/Reflector/ReflectorVisibility_System.h"
#include "Modules/Graphics/Lighting/Reflector/ReflectorScheduler_System.h"
#include "Modules/Graphics/Lighting/Reflector/ReflectorSync_System.h"
#include "Engine.h"


Reflector_Technique::~Reflector_Technique()
{
	// Update indicator
	*m_aliveIndicator = false;
}

Reflector_Technique::Reflector_Technique(Engine& engine, std::vector<Camera*>& sceneCameras) :
	Graphics_Technique(Technique_Category::PRIMARY_LIGHTING),
	m_engine(engine),
	m_shaderLighting(Shared_Shader(engine, "Core\\Reflector\\IBL_Parallax")),
	m_shaderStencil(Shared_Shader(engine, "Core\\Reflector\\Stencil")),
	m_shaderCopy(Shared_Shader(engine, "Core\\Reflector\\2D_To_Cubemap")),
	m_shaderConvolute(Shared_Shader(engine, "Core\\Reflector\\Cube_Convolution")),
	m_shapeCube(Shared_Auto_Model(engine, "cube")),
	m_shapeQuad(Shared_Auto_Model(engine, "quad")),
	m_viewport(glm::ivec2(0), glm::ivec2(1), engine),
	m_sceneCameras(sceneCameras)
{
	// Auxiliary Systems
	m_auxilliarySystems.makeSystem<ReflectorScheduler_System>(engine, m_frameData);
	m_auxilliarySystems.makeSystem<ReflectorVisibility_System>(m_frameData);
	m_auxilliarySystems.makeSystem<ReflectorSync_System>(m_frameData);

	// Preferences
	auto& preferences = engine.getPreferenceState();
	preferences.getOrSetValue(PreferenceState::Preference::C_ENVMAP_SIZE, m_frameData.envmapSize.x);
	preferences.addCallback(PreferenceState::Preference::C_ENVMAP_SIZE, m_aliveIndicator, [&](const float& f) {
		m_frameData.envmapSize = glm::ivec2(std::max(1u, static_cast<unsigned int>(f)));
		m_viewport.resize(glm::ivec2(m_frameData.envmapSize), static_cast<int>(m_frameData.reflectorLayers));
		});
	// Environment Map
	m_frameData.envmapSize = glm::ivec2(std::max(1U, static_cast<unsigned int>(m_frameData.envmapSize.x)));
	m_viewport.resize(m_frameData.envmapSize, static_cast<int>(m_frameData.reflectorLayers));

	// Asset-Finished Callbacks
	m_shapeQuad->addCallback(m_aliveIndicator, [&]() noexcept {
		// count, primCount, first, reserved
		const GLuint quadData[4] = { static_cast<GLuint>(m_shapeQuad->getSize()), 1, 0, 0 };
		m_indirectQuad = StaticMultiBuffer(sizeof(GLuint) * 4, quadData);
		m_indirectQuadConvolute = StaticMultiBuffer(sizeof(GLuint) * 4, quadData);
		});
}

void Reflector_Technique::clearCache(const float& /*deltaTime*/) noexcept
{
	m_frameData.lightBuffer.endReading();
	m_frameData.viewInfo.clear();
	m_frameData.reflectorsToUpdate.clear();
	m_drawIndex = 0;
	m_drawData.clear();
}

void Reflector_Technique::updateCache(const float& deltaTime, ecsWorld& world)
{
	// Link together the dimensions of view info to that of the viewport vectors
	m_frameData.viewInfo.resize(m_sceneCameras.size());
	world.updateSystems(m_auxilliarySystems, deltaTime);
}

void Reflector_Technique::updatePass(const float& deltaTime)
{
	// Exit Early
	if (m_enabled && Asset::All_Ready(m_shapeQuad, m_shaderCopy, m_shaderConvolute))
		updateReflectors(deltaTime);
}

void Reflector_Technique::renderTechnique(const float& deltaTime, Viewport& viewport, const std::vector<std::pair<int, int>>& perspectives)
{
	// Exit Early
	if (m_enabled && (!m_frameData.viewInfo.empty()) && Asset::All_Ready(m_shapeCube, m_shaderLighting, m_shaderStencil)) {
		if (m_drawIndex >= m_drawData.size())
			m_drawData.resize(size_t(m_drawIndex) + 1ULL);

		// Accumulate all visibility info for the cameras passed in
		std::vector<glm::ivec2> camIndices;
		std::vector<GLint> lightIndices;
		for (auto& [camIndex, layer] : perspectives) {
			const std::vector<glm::ivec2> tempIndices(m_frameData.viewInfo[camIndex].lightIndices.size(), { camIndex, layer });
			camIndices.insert(camIndices.end(), tempIndices.begin(), tempIndices.end());
			lightIndices.insert(lightIndices.end(), m_frameData.viewInfo[camIndex].lightIndices.begin(), m_frameData.viewInfo[camIndex].lightIndices.end());
		}

		if (!lightIndices.empty()) {
			auto& drawBuffer = m_drawData[m_drawIndex];
			auto& camBufferIndex = drawBuffer.bufferCamIndex;
			auto& lightBufferIndex = drawBuffer.visLights;

			// Write accumulated data
			camBufferIndex.beginWriting();
			lightBufferIndex.beginWriting();
			drawBuffer.indirectShape.beginWriting();
			camBufferIndex.write(0, sizeof(glm::ivec2) * camIndices.size(), camIndices.data());
			lightBufferIndex.write(0, sizeof(GLuint) * lightIndices.size(), lightIndices.data());
			const GLuint dataShape[] = { static_cast<GLuint>(m_shapeCube->getSize()), static_cast<GLuint>(lightIndices.size()), 0,0 };
			drawBuffer.indirectShape.write(0, sizeof(GLuint) * 4, &dataShape);
			camBufferIndex.endWriting();
			lightBufferIndex.endWriting();
			drawBuffer.indirectShape.endWriting();

			// Render lights
			renderReflectors(deltaTime, viewport);
			camBufferIndex.endReading();
			lightBufferIndex.endReading();
			drawBuffer.indirectShape.endReading();

			m_drawIndex++;
		}
	}
}

void Reflector_Technique::updateReflectors(const float& deltaTime)
{
	auto clientTime = Engine::GetSystemTime();
	if (!m_frameData.reflectorsToUpdate.empty()) {
		m_viewport.resize(m_frameData.envmapSize, static_cast<int>(m_frameData.reflectorLayers));
		m_viewport.bind();
		m_viewport.clear();

		// Accumulate Perspective Data
		std::vector<std::pair<int, int>> perspectives;
		perspectives.reserve(m_frameData.reflectorsToUpdate.size());
		for (auto& [importance, time, reflectorSpot, camera] : m_frameData.reflectorsToUpdate) {
			// Accumulate all visibility info for the cameras passed in
			int visibilityIndex = 0;
			bool found = false;
			const auto cameraCount = m_sceneCameras.size();
			for (int y = 0; y < cameraCount; ++y)
				if (m_sceneCameras.at(y) == camera) {
					visibilityIndex = y;
					found = true;
					break;
				}
			if (found)
				perspectives.push_back({ visibilityIndex, reflectorSpot });
			*time = clientTime;
			camera->setEnabled(false);
		}
		m_indirectQuad.beginWriting();
		m_indirectQuadConvolute.beginWriting();
		const auto instanceCount = static_cast<GLuint>(perspectives.size());
		const auto convoluteCount = static_cast<GLuint>(m_frameData.reflectorLayers);
		m_indirectQuad.write(sizeof(GLuint), sizeof(GLuint), &instanceCount);
		m_indirectQuadConvolute.write(sizeof(GLuint), sizeof(GLuint), &convoluteCount);
		m_indirectQuad.endWriting();
		m_indirectQuadConvolute.endWriting();

		// Update all reflectors at once
		constexpr unsigned int categories = static_cast<unsigned int>(Graphics_Technique::Technique_Category::GEOMETRY) | static_cast<unsigned int>(Graphics_Technique::Technique_Category::PRIMARY_LIGHTING) | static_cast<unsigned int>(Graphics_Technique::Technique_Category::SECONDARY_LIGHTING);
		m_engine.getModule_Graphics().getPipeline().render(deltaTime, m_viewport, perspectives, categories);

		// Copy all lighting results into cube faces, generating cubemap's
		m_viewport.m_gfxFBOS.bindForReading("LIGHTING", 0);
		m_frameData.envmapFBO.bindForWriting(0);
		m_shaderCopy->bind();
		m_indirectQuad.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glDisable(GL_STENCIL_TEST);
		glBindVertexArray(m_shapeQuad->m_vaoID);
		glDrawArraysIndirect(GL_TRIANGLES, nullptr);
		m_indirectQuad.endReading();

		// Convolute all completed cubemap's, not just what was done this frame
		m_shaderConvolute->bind();
		m_indirectQuadConvolute.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		m_frameData.envmapFBO.bindForReading(0);
		for (int r = 1; r < 6; ++r) {
			// Ensure we are writing to MIP level r
			const unsigned int write_size = static_cast<unsigned int>(std::max(1.0F, (floor(static_cast<float>(m_frameData.envmapSize.x) / pow(2.0F, static_cast<float>(r))))));
			glViewport(0, 0, write_size, write_size);
			m_shaderConvolute->setUniform(1, static_cast<float>(r) / 5.0F);
			m_frameData.envmapFBO.bindForWriting(r);

			// Convolute the 6 faces for this roughness level
			glDrawArraysIndirect(GL_TRIANGLES, nullptr);
		}

		m_indirectQuadConvolute.endReading();
		Shader::Release();
		m_frameData.reflectorsToUpdate.clear();
	}
}

void Reflector_Technique::renderReflectors(const float& /*unused*/, Viewport& viewport)
{
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
	glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
	glStencilMask(0xFF);

	// Draw only into depth-stencil buffer
	m_shaderStencil->bind();										// Shader (reflector)
	viewport.m_gfxFBOS.bindForWriting("REFLECTION");				// Ensure writing to reflection FBO
	viewport.m_gfxFBOS.bindForReading("GEOMETRY", 0);				// Read from Geometry FBO
	m_frameData.envmapFBO.bindForReading(4);						// Reflection map (environment texture)
	m_drawData[m_drawIndex].bufferCamIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
	m_drawData[m_drawIndex].visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);	// SSBO visible light indices
	m_frameData.lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);	// Reflection buffer
	m_drawData[m_drawIndex].indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);				// Draw call buffer
	glBindVertexArray(m_shapeCube->m_vaoID);						// Quad VAO
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glClear(GL_STENCIL_BUFFER_BIT);
	glStencilFunc(GL_ALWAYS, 0, 0);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	glFrontFace(GL_CW);
	glDrawArraysIndirect(GL_TRIANGLES, nullptr);
	glFrontFace(GL_CCW);

	// Now draw into color buffers
	m_shaderLighting->bind();
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glCullFace(GL_FRONT);
	glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDrawArraysIndirect(GL_TRIANGLES, nullptr);

	glClear(GL_STENCIL_BUFFER_BIT);
	glDepthMask(GL_TRUE);
	glCullFace(GL_BACK);
	glBlendFunc(GL_ONE, GL_ZERO);
	glDisable(GL_STENCIL_TEST);
	Shader::Release();
}