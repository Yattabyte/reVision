#include "Modules/Graphics/Effects/HDR.h"
#include "Engine.h"


HDR::~HDR()
{
	// Update indicator
	*m_aliveIndicator = false;
}

HDR::HDR(Engine& engine) :
	Graphics_Technique(Technique_Category::POST_PROCESSING),
	m_engine(engine),
	m_shaderHDR(Shared_Shader(engine, "Effects\\HDR")),
	m_shapeQuad(Shared_Auto_Model(engine, "quad"))
{
	// Preferences
	auto& preferences = engine.getPreferenceState();
	preferences.getOrSetValue(PreferenceState::Preference::C_GAMMA, m_gamma);
	preferences.addCallback(PreferenceState::Preference::C_GAMMA, m_aliveIndicator, [&](const float& f) noexcept { m_gamma = f; });
}

void HDR::clearCache(const float& /*deltaTime*/) noexcept
{
	m_drawIndex = 0;
}

void HDR::renderTechnique(const float& /*deltaTime*/, Viewport& viewport, const std::vector<std::pair<int, int>>& perspectives)
{
	if (!m_enabled || !Asset::All_Ready(m_shapeQuad, m_shaderHDR))
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

	// Write HDR effect to own framebuffer
	camBufferIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
	viewport.m_gfxFBOS.bindForWriting("HDR");
	viewport.m_gfxFBOS.bindForReading("LIGHTING", 0);
	m_shaderHDR->bind();
	m_shaderHDR->setUniform(0, 1.0F);
	m_shaderHDR->setUniform(1, m_gamma);

	// Use the currently bound framebuffer from the prior effect
	glBindVertexArray(m_shapeQuad->m_vaoID);
	indirectQuad.drawCall();

	camBufferIndex.endReading();
	indirectQuad.endReading();
	Shader::Release();
	m_drawIndex++;
}