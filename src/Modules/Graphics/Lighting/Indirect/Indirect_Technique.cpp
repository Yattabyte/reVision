#include "Modules/Graphics/Lighting/Indirect/Indirect_Technique.h"
#include "Modules/Graphics/Lighting/Indirect/IndirectVisibility_System.h"
#include "Modules/Graphics/Lighting/Indirect/IndirectSync_System.h"
#include "Engine.h"
#include <random>


Indirect_Technique::~Indirect_Technique() 
{
	// Update indicator
	*m_aliveIndicator = false;
}

Indirect_Technique::Indirect_Technique(Engine& engine, ShadowData& shadowData, Camera& clientCamera, std::vector<Camera*>& sceneCameras) :
	Graphics_Technique(Technique_Category::PRIMARY_LIGHTING),
	m_engine(engine),
	m_shader_Bounce(Shared_Shader(engine, "Core\\Light\\Bounce")),
	m_shader_Recon(Shared_Shader(engine, "Core\\Light\\Reconstruction")),
	m_shader_Rebounce(Shared_Shader(engine, "Core\\Light\\Rebounce")),
	m_shapeQuad(Shared_Auto_Model(engine, "quad")),
	m_frameData(shadowData, clientCamera),
	m_sceneCameras(sceneCameras)
{
	// Auxiliary Systems
	m_auxilliarySystems.makeSystem<IndirectVisibility_System>(m_frameData);
	m_auxilliarySystems.makeSystem<IndirectSync_System>(m_frameData);

	// Noise Texture
	const std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
	std::default_random_engine generator;
	float texData[32 * 32 * 32 * 3]{};
	std::generate(std::begin(texData), std::end(texData), [&]() { return randomFloats(generator); });
	glCreateTextures(GL_TEXTURE_3D, 1, &m_textureNoise32);
	glTextureImage3DEXT(m_textureNoise32, GL_TEXTURE_3D, 0, GL_RGB16F, 32, 32, 32, 0, GL_RGB, GL_FLOAT, texData);
	glTextureParameteri(m_textureNoise32, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTextureParameteri(m_textureNoise32, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTextureParameteri(m_textureNoise32, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
	glTextureParameteri(m_textureNoise32, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(m_textureNoise32, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Preferences
	auto& preferences = engine.getPreferenceState();
	preferences.getOrSetValue(PreferenceState::Preference::C_RH_BOUNCE_SIZE, m_bounceSize);
	preferences.addCallback(PreferenceState::Preference::C_RH_BOUNCE_SIZE, m_aliveIndicator, [&](const float& f) noexcept { m_bounceSize = (GLuint)f; });
}

void Indirect_Technique::clearCache(const float&) noexcept
{
	m_frameData.lightBuffer.endReading();
	m_frameData.viewInfo.clear();
	m_drawIndex = 0;
}

void Indirect_Technique::updateCache(const float& deltaTime, ecsWorld& world) 
{
	// Link together the dimensions of view info to that of the viewport vectors
	m_frameData.viewInfo.resize(m_sceneCameras.size());
	world.updateSystems(m_auxilliarySystems, deltaTime);
}

void Indirect_Technique::renderTechnique(const float&, Viewport& viewport, const std::vector<std::pair<int, int>>& perspectives)
{
	// Update light-bounce volume
	if (m_enabled && m_frameData.viewInfo.size() && Asset::All_Ready(m_shapeQuad, m_shader_Bounce, m_shader_Recon, m_shader_Rebounce)) {
		// Light bounce using client camera
		if (m_drawIndex >= m_drawData.size())
			m_drawData.resize(size_t(m_drawIndex) + 1ull);
		auto& [camBufferIndex, camBufferRebounce, camBufferRecon, visLights, indirectBounce, indirectQuad, indirectQuadRecon] = m_drawData[m_drawIndex];

		// Accumulate all visibility info for the cameras passed in
		std::vector<glm::ivec2> camIndicesGen, camIndiciesRebounce, camIndiciesRecon;
		std::vector<GLint> lightIndices;
		for (auto& [camIndex, layer] : perspectives) {
			const std::vector<glm::ivec2> tempIndices(m_frameData.viewInfo[camIndex].lightIndices.size(), { camIndex, layer });
			camIndicesGen.insert(camIndicesGen.end(), tempIndices.begin(), tempIndices.end());
			lightIndices.insert(lightIndices.end(), m_frameData.viewInfo[camIndex].lightIndices.begin(), m_frameData.viewInfo[camIndex].lightIndices.end());
			const std::vector<glm::ivec2> tempIndices2(m_bounceSize, { camIndex, layer });
			camIndiciesRebounce.insert(camIndiciesRebounce.end(), tempIndices2.begin(), tempIndices2.end());
			camIndiciesRecon.push_back({ camIndex, layer });
		}
		const auto shadowCount = camIndicesGen.size();

		if (lightIndices.size()) {
			updateDrawParams(camIndicesGen, camIndiciesRebounce, camIndiciesRecon, lightIndices, shadowCount, perspectives.size());
			fillBounceVolume(shadowCount, viewport.m_gfxFBOS.m_rhVolume);
			rebounceVolume(viewport.m_gfxFBOS.m_rhVolume, camBufferRebounce, indirectQuad);
			reconstructVolume(viewport, camBufferRecon, indirectQuadRecon);
			camBufferIndex.endReading();
			camBufferRebounce.endReading();
			camBufferRecon.endReading();
			visLights.endReading();
			indirectBounce.endReading();
			indirectQuad.endReading();
			indirectQuadRecon.endReading();
			Shader::Release();

			m_drawIndex++;
		}
	}
}

void Indirect_Technique::updateDrawParams(std::vector<glm::ivec2>& camIndicesGen, std::vector<glm::ivec2>& camIndiciesRebounce, std::vector<glm::ivec2>& camIndiciesRecon, std::vector<int>& lightIndices, const size_t& shadowCount, const size_t& perspectivesSize) noexcept 
{
	// Write accumulated data
	auto& [camBufferIndex, camBufferRebounce, camBufferRecon, visLights, indirectBounce, indirectQuad, indirectQuadRecon] = m_drawData[m_drawIndex];
	camBufferIndex.beginWriting();
	camBufferRebounce.beginWriting();
	camBufferRecon.beginWriting();
	visLights.beginWriting();
	indirectBounce.beginWriting();
	indirectQuad.beginWriting();
	indirectQuadRecon.beginWriting();
	camBufferIndex.write(0, sizeof(glm::ivec2) * camIndicesGen.size(), camIndicesGen.data());
	camBufferRebounce.write(0, sizeof(glm::ivec2) * camIndiciesRebounce.size(), camIndiciesRebounce.data());
	camBufferRecon.write(0, sizeof(glm::ivec2) * camIndiciesRecon.size(), camIndiciesRecon.data());
	visLights.write(0, sizeof(GLuint) * lightIndices.size(), lightIndices.data());
	const GLuint dataBounce[] = { (GLuint)m_shapeQuad->getSize(), (GLuint)(shadowCount * m_bounceSize), 0, 0 };
	indirectBounce.write(0, sizeof(GLuint) * 4, &dataBounce);
	indirectQuad.setPrimitiveCount(m_bounceSize);
	indirectQuadRecon.setPrimitiveCount((GLuint)perspectivesSize);
	camBufferIndex.endWriting();
	camBufferRebounce.endWriting();
	camBufferRecon.endWriting();
	visLights.endWriting();
	indirectBounce.endWriting();
	indirectQuad.endWriting();
	indirectQuadRecon.endWriting();
}

void Indirect_Technique::fillBounceVolume(const size_t& shadowCount, RH_Volume& rhVolume) noexcept 
{
	// Prepare rendering state
	glBlendEquationSeparatei(0, GL_MIN, GL_MIN);
	glBindVertexArray(m_shapeQuad->m_vaoID);
	m_shader_Bounce->setUniform(0, (GLint)(shadowCount));
	m_shader_Bounce->setUniform(1, rhVolume.m_max);
	m_shader_Bounce->setUniform(2, rhVolume.m_min);
	m_shader_Bounce->setUniform(4, rhVolume.m_resolution);
	m_shader_Bounce->setUniform(6, rhVolume.m_unitSize);

	glViewport(0, 0, (GLsizei)rhVolume.m_resolution, (GLsizei)rhVolume.m_resolution);
	m_shader_Bounce->bind();
	rhVolume.writePrimary();
	glBindTextureUnit(0, m_frameData.shadowData.shadowFBO.m_texNormal);
	glBindTextureUnit(1, m_frameData.shadowData.shadowFBO.m_texColor);
	glBindTextureUnit(2, m_frameData.shadowData.shadowFBO.m_texDepth);
	glBindTextureUnit(4, m_textureNoise32);
	m_drawData[m_drawIndex].bufferCamIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
	m_drawData[m_drawIndex].visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);
	m_frameData.lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
	m_drawData[m_drawIndex].indirectBounce.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
	glDrawArraysIndirect(GL_TRIANGLES, nullptr);
}

void Indirect_Technique::rebounceVolume(RH_Volume& rhVolume, const DynamicBuffer<>& camBufferRebounce, IndirectDraw<>& indirectQuad) noexcept 
{
	// Bind common data
	glDepthMask(GL_TRUE);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ZERO);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	m_shader_Rebounce->setUniform(1, rhVolume.m_max);
	m_shader_Rebounce->setUniform(2, rhVolume.m_min);
	m_shader_Rebounce->setUniform(4, rhVolume.m_resolution);
	m_shader_Rebounce->setUniform(5, rhVolume.m_unitSize);
	m_shader_Recon->setUniform(1, rhVolume.m_max);
	m_shader_Recon->setUniform(2, rhVolume.m_min);
	m_shader_Recon->setUniform(3, rhVolume.m_resolution);
	glBindVertexArray(m_shapeQuad->m_vaoID);

	// Bounce light a second time
	m_shader_Rebounce->bind();
	rhVolume.readPrimary(0);
	rhVolume.writeSecondary();
	glViewport(0, 0, (GLsizei)rhVolume.m_resolution, (GLsizei)rhVolume.m_resolution);
	camBufferRebounce.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
	indirectQuad.drawCall();
}

void Indirect_Technique::reconstructVolume(Viewport& viewport, const DynamicBuffer<>& camBufferRecon, IndirectDraw<>& indirectQuadRecon) 
{
	// Reconstruct indirect radiance
	glViewport(0, 0, GLsizei(viewport.m_dimensions.x), GLsizei(viewport.m_dimensions.y));
	m_shader_Recon->bind();
	viewport.m_gfxFBOS.bindForReading("GEOMETRY", 0);
	viewport.m_gfxFBOS.m_rhVolume.readSecondary(4);
	viewport.m_gfxFBOS.bindForWriting("BOUNCE");
	camBufferRecon.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
	indirectQuadRecon.drawCall();
	glEnable(GL_DEPTH_TEST);
}