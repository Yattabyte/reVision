#include "Modules/Graphics/Effects/Bloom.h"
#include "Engine.h"


Bloom::~Bloom()
{
	// Update indicator
	*m_aliveIndicator = false;
}

Bloom::Bloom(Engine& engine) :
	Graphics_Technique(Technique_Category::POST_PROCESSING),
	m_engine(engine),
	m_shaderBloomExtract(Shared_Shader(engine, "Effects\\Bloom Extraction")),
	m_shaderCopy(Shared_Shader(engine, "Effects\\Copy Texture")),
	m_shaderGB(Shared_Shader(engine, "Effects\\Gaussian Blur")),
	m_shapeQuad(Shared_Auto_Model(engine, "quad"))
{
	// Preference Callbacks
	auto& preferences = engine.getPreferenceState();
	preferences.getOrSetValue(PreferenceState::Preference::C_BLOOM, m_enabled);
	preferences.addCallback(PreferenceState::Preference::C_BLOOM, m_aliveIndicator, [&](const float& f) noexcept { m_enabled = static_cast<bool>(f); });
	preferences.getOrSetValue(PreferenceState::Preference::C_BLOOM_STRENGTH, m_bloomStrength);
	preferences.addCallback(PreferenceState::Preference::C_BLOOM_STRENGTH, m_aliveIndicator, [&](const float& f) noexcept { setBloomStrength(static_cast<int>(f)); });
}

void Bloom::clearCache(const float& /*deltaTime*/) noexcept
{
	m_drawIndex = 0;
}

void Bloom::renderTechnique(const float& /*deltaTime*/, Viewport& viewport, const std::vector<std::pair<int, int>>& perspectives)
{
	if (!m_enabled || !Asset::All_Ready(m_shapeQuad, m_shaderBloomExtract, m_shaderCopy, m_shaderGB))
		return;

	// Prepare camera index
	if (m_drawIndex >= m_drawData.size())
		m_drawData.resize(size_t(m_drawIndex) + 1ULL);
	auto& [camBufferIndex, indirectQuad] = m_drawData[m_drawIndex];
	camBufferIndex.beginWriting();
	indirectQuad.beginWriting();
	std::vector<glm::ivec2> camIndices;
	camIndices.reserve(perspectives.size());
for (auto& [camIndex, layer] : perspectives)
		camIndices.push_back({ camIndex, layer });
	camBufferIndex.write(0, sizeof(glm::ivec2) * camIndices.size(), camIndices.data());
	indirectQuad.setPrimitiveCount(static_cast<GLuint>(perspectives.size()));
	camBufferIndex.endWriting();
	indirectQuad.endWriting();

	// Extract bright regions from lighting buffer
	camBufferIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
	m_shaderBloomExtract->bind();
	viewport.m_gfxFBOS.bindForWriting("BLOOM");
	viewport.m_gfxFBOS.bindForReading("LIGHTING", 0);
	glBindVertexArray(m_shapeQuad->m_vaoID);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	indirectQuad.bind();
	glDrawArraysIndirect(GL_TRIANGLES, nullptr);
	size_t bloomSpot = 0;

	if (m_bloomStrength > 0) {
		// Read from desired texture, blur into this frame buffer
		bool horizontal = false;
		glBindTextureUnit(0, viewport.m_gfxFBOS.getTexID("BLOOM", 0));
		glBindTextureUnit(1, viewport.m_gfxFBOS.getTexID("BLOOM", 1));
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + static_cast<int>(horizontal));
		m_shaderGB->bind();
		m_shaderGB->setUniform(0, horizontal);
		m_shaderGB->setUniform(1, glm::vec2(viewport.m_dimensions));

		// Blur remainder of the times
		for (int i = 0; i < m_bloomStrength; i++) {
			horizontal = !horizontal;
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + static_cast<int>(horizontal));
			m_shaderGB->setUniform(0, horizontal);
			glDrawArraysIndirect(GL_TRIANGLES, nullptr);
		}
		bloomSpot = horizontal ? 1ULL : 0ULL;
	}

	// Copy to lighting buffer
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	viewport.m_gfxFBOS.bindForWriting("LIGHTING");
	glBindTextureUnit(0, viewport.m_gfxFBOS.getTexID("BLOOM", bloomSpot));
	m_shaderCopy->bind();
	glDrawArraysIndirect(GL_TRIANGLES, nullptr);
	glDisable(GL_BLEND);
	camBufferIndex.endReading();
	indirectQuad.endReading();
	Shader::Release();
	m_drawIndex++;
}

void Bloom::setBloomStrength(const int& strength) noexcept
{
	m_bloomStrength = strength;
}