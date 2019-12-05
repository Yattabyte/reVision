#include "Modules/Graphics/Effects/Join_Reflections.h"
#include "Modules/Graphics/Common/Viewport.h"


Join_Reflections::~Join_Reflections() noexcept 
{
	// Update indicator
	*m_aliveIndicator = false;
}

Join_Reflections::Join_Reflections(Engine& engine) noexcept :
	Graphics_Technique(Technique_Category::SECONDARY_LIGHTING),
	m_engine(engine),
	m_shader(Shared_Shader(engine, "Effects\\Join Reflections")),
	m_brdfMap(Shared_Texture(engine, "brdfLUT.png", GL_TEXTURE_2D, false, false)),
	m_shapeQuad(Shared_Auto_Model(engine, "quad"))
{
}

void Join_Reflections::clearCache(const float& deltaTime) noexcept 
{
	m_drawIndex = 0;
}

void Join_Reflections::renderTechnique(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::vector<std::pair<int, int>>& perspectives) noexcept 
{
	if (!m_enabled || !Asset::All_Ready(m_shapeQuad, m_shader))
		return;

	// Prepare camera index
	if (m_drawIndex >= m_drawData.size())
		m_drawData.resize(size_t(m_drawIndex) + 1ull);
	auto& [camBufferIndex, indirectQuad] = m_drawData[m_drawIndex];
	camBufferIndex.beginWriting();
	indirectQuad.beginWriting();
	std::vector<glm::ivec2> camIndices;
	for (auto& [camIndex, layer] : perspectives)
		camIndices.push_back({ camIndex, layer });
	camBufferIndex.write(0, sizeof(glm::ivec2) * camIndices.size(), camIndices.data());
	indirectQuad.setPrimitiveCount((GLuint)perspectives.size());
	camBufferIndex.endWriting();
	indirectQuad.endWriting();

	// Join the reflections
	camBufferIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	viewport->m_gfxFBOS.bindForWriting("LIGHTING");
	viewport->m_gfxFBOS.bindForReading("GEOMETRY", 0);
	viewport->m_gfxFBOS.bindForReading("BOUNCE", 4);
	viewport->m_gfxFBOS.bindForReading("REFLECTION", 5);
	m_brdfMap->bind(6);
	m_shader->bind();
	glBindVertexArray(m_shapeQuad->m_vaoID);
	indirectQuad.drawCall();

	glDisable(GL_BLEND);
	camBufferIndex.endReading();
	indirectQuad.endReading();
	Shader::Release();
	m_drawIndex++;
}