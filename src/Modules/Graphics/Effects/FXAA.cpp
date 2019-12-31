#include "Modules/Graphics/Effects/FXAA.h"
#include "Engine.h"


FXAA::~FXAA()
{
	// Update indicator
	*m_aliveIndicator = false;
}

 FXAA::FXAA(Engine& engine) :
	Graphics_Technique(Technique_Category::POST_PROCESSING),
	m_engine(engine),
	m_shaderFXAA(Shared_Shader(engine, "Effects\\FXAA")),
	m_shapeQuad(Shared_Auto_Model(engine, "quad"))
{
	// Preferences
	auto& preferences = engine.getPreferenceState();
	preferences.getOrSetValue(PreferenceState::Preference::C_FXAA, m_enabled);
	preferences.addCallback(PreferenceState::Preference::C_FXAA, m_aliveIndicator, [&](const float& f) noexcept { m_enabled = static_cast<bool>(f); });
}

void FXAA::clearCache(const float& /*deltaTime*/) noexcept
{
	m_drawIndex = 0;
}

void FXAA::renderTechnique(const float& /*deltaTime*/, Viewport& viewport, const std::vector<std::pair<int, int>>& perspectives)
{
	if (!m_enabled || !Asset::All_Ready(m_shapeQuad, m_shaderFXAA))
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

	// Apply FXAA effect
	camBufferIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
	viewport.m_gfxFBOS.bindForWriting("FXAA");
	viewport.m_gfxFBOS.bindForReading("HDR", 0);
	m_shaderFXAA->bind();
	glBindVertexArray(m_shapeQuad->m_vaoID);
	indirectQuad.drawCall();

	// Bind for reading by next effect
	glBindTextureUnit(0, viewport.m_gfxFBOS.getTexID("FXAA", 0));
	camBufferIndex.endReading();
	indirectQuad.endReading();
	Shader::Release();
	m_drawIndex++;
}