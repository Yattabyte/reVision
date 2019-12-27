#include "Modules/Graphics/Effects/SSR.h"
#include "Engine.h"


SSR::~SSR() 
{
	// Update indicator
	*m_aliveIndicator = false;

	// Destroy OpenGL objects
	glDeleteTextures(1, &m_bayerID);
}

SSR::SSR(Engine& engine) :
	Graphics_Technique(Technique_Category::SECONDARY_LIGHTING),
	m_engine(engine),
	m_shaderSSR1(Shared_Shader(engine, "Effects\\SSR part 1")),
	m_shaderSSR2(Shared_Shader(engine, "Effects\\SSR part 2")),
	m_shaderCopy(Shared_Shader(engine, "Effects\\Copy Texture")),
	m_shaderConvMips(Shared_Shader(engine, "Effects\\Gaussian Blur MIP")),
	m_shapeQuad(Shared_Auto_Model(engine, "quad"))
{
	// Preferences
	auto& preferences = engine.getPreferenceState();
	preferences.getOrSetValue(PreferenceState::Preference::C_SSR, m_enabled);
	preferences.addCallback(PreferenceState::Preference::C_SSR, m_aliveIndicator, [&](const float& f) noexcept { m_enabled = static_cast<bool>(f); });

	// Bayer matrix
	constexpr GLubyte data[16] = { 0,8,2,10,12,4,14,6,3,11,1,9,15,7,13,5 };
	glCreateTextures(GL_TEXTURE_2D, 1, &m_bayerID);
	glTextureStorage2D(m_bayerID, 1, GL_R16F, 4, 4);
	glTextureSubImage2D(m_bayerID, 0, 0, 0, 4, 4, GL_RED, GL_UNSIGNED_BYTE, &data);
	glTextureParameteri(m_bayerID, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(m_bayerID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTextureParameteri(m_bayerID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(m_bayerID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Error Reporting
	auto& msgMgr = engine.getManager_Messages();
	if (!glIsTexture(m_bayerID))
		msgMgr.error("SSR Bayer Matrix Texture is incomplete.");
}

void SSR::clearCache(const float& /*deltaTime*/) noexcept
{
	m_drawIndex = 0;
}

void SSR::renderTechnique(const float& /*deltaTime*/, Viewport& viewport, const std::vector<std::pair<int, int>>& perspectives)
{
	if (!m_enabled || !Asset::All_Ready(m_shapeQuad, m_shaderCopy, m_shaderConvMips, m_shaderSSR1, m_shaderSSR2))
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

	// Bind common data
	camBufferIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
	glBindVertexArray(m_shapeQuad->m_vaoID);
	indirectQuad.bind();

	updateMIPChain(viewport);

	glDisable(GL_BLEND);
	viewport.m_gfxFBOS.bindForWriting("SSR");
	viewport.m_gfxFBOS.bindForReading("GEOMETRY", 0);
	m_shaderSSR1->bind();
	glBindTextureUnit(6, m_bayerID);
	glDrawArraysIndirect(GL_TRIANGLES, nullptr);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	viewport.m_gfxFBOS.bindForWriting("REFLECTION");
	glBindTextureUnit(5, viewport.m_gfxFBOS.getTexID("SSR", 0));
	glBindTextureUnit(6, viewport.m_gfxFBOS.getTexID("SSR_MIP", 0));
	m_shaderSSR2->bind();
	glDrawArraysIndirect(GL_TRIANGLES, nullptr);

	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_BLEND);
	camBufferIndex.endReading();
	indirectQuad.endReading();
	Shader::Release();
	m_drawIndex++;
}

void SSR::updateMIPChain(Viewport& viewport) 
{
	const auto mipFboID = viewport.m_gfxFBOS.getFboID("SSR_MIP");
	const auto mipTexID = viewport.m_gfxFBOS.getTexID("SSR_MIP", 0);
	const auto dimensions = glm::vec2(viewport.m_dimensions);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	// Copy lighting texture to one with a MIP chain
	m_shaderCopy->bind();
	viewport.m_gfxFBOS.bindForReading("LIGHTING", 0);
	viewport.m_gfxFBOS.bindForWriting("SSR_MIP");
	constexpr GLfloat clearColor[] = { 0.0F, 0.0F, 0.0F, 0.0F };
	glClearNamedFramebufferfv(mipFboID, GL_COLOR, 0, clearColor);
	glDrawArraysIndirect(GL_TRIANGLES, nullptr);

	// Blur MIP chain, reading from 1 MIP level and writing into next
	m_shaderConvMips->bind();
	glBindTextureUnit(0, mipTexID);
	for (int horizontal = 0; horizontal < 2; ++horizontal) {
		m_shaderConvMips->setUniform(0, horizontal);
		glm::ivec2 read_size = viewport.m_dimensions;
		for (int x = 1; x < 6; ++x) {
			// Ensure we are reading from MIP level x - 1
			m_shaderConvMips->setUniform(1, read_size);
			glTextureParameteri(mipTexID, GL_TEXTURE_BASE_LEVEL, x - 1);
			glTextureParameteri(mipTexID, GL_TEXTURE_MAX_LEVEL, x - 1);
			// Ensure we are writing to MIP level x
			const glm::ivec2 write_size = glm::ivec2(floor(dimensions.x / pow(2, x)), floor(dimensions.y / pow(2, x)));
			glNamedFramebufferTexture(mipFboID, GL_COLOR_ATTACHMENT0, mipTexID, x);

			glViewport(0, 0, std::max(1, write_size.x), std::max(1, write_size.y));
			glDrawArraysIndirect(GL_TRIANGLES, nullptr);
			read_size = write_size;
		}
		// Blend second pass
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
	}

	// Restore to default
	glTextureParameteri(mipTexID, GL_TEXTURE_BASE_LEVEL, 0);
	glTextureParameteri(mipTexID, GL_TEXTURE_MAX_LEVEL, 5);
	glNamedFramebufferTexture(mipFboID, GL_COLOR_ATTACHMENT0, mipTexID, 0);
	glViewport(0, 0, static_cast<GLsizei>(dimensions.x), static_cast<GLsizei>(dimensions.y));
	Shader::Release();
}