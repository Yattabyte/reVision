#include "Modules/Graphics/Effects/SSAO.h"
#include "Engine.h"
#include <random>


SSAO::~SSAO()
{
	// Update indicator
	*m_aliveIndicator = false;
}

SSAO::SSAO(Engine& engine) :
	Graphics_Technique(Technique_Category::SECONDARY_LIGHTING),
	m_engine(engine),
	m_shader(Shared_Shader(engine, "Effects\\SSAO")),
	m_shaderCopyAO(Shared_Shader(engine, "Effects\\SSAO To AO")),
	m_shaderGB_A(Shared_Shader(engine, "Effects\\Gaussian Blur Alpha")),
	m_shapeQuad(Shared_Auto_Model(engine, "quad"))
{
	// Preferences
	auto& preferences = engine.getPreferenceState();
	preferences.getOrSetValue(PreferenceState::Preference::C_SSAO, m_enabled);
	preferences.addCallback(PreferenceState::Preference::C_SSAO, m_aliveIndicator, [&](const float& f) noexcept { m_enabled = (bool)f; });
	preferences.getOrSetValue(PreferenceState::Preference::C_SSAO_RADIUS, m_radius);
	preferences.addCallback(PreferenceState::Preference::C_SSAO_RADIUS, m_aliveIndicator, [&](const float& f) noexcept { m_radius = f; if (m_shader->ready()) m_shader->setUniform(0, m_radius); });
	preferences.getOrSetValue(PreferenceState::Preference::C_SSAO_QUALITY, m_quality);
	preferences.addCallback(PreferenceState::Preference::C_SSAO_QUALITY, m_aliveIndicator, [&](const float& f) noexcept { m_quality = (int)f; if (m_shader->ready()) m_shader->setUniform(1, m_quality); });
	preferences.getOrSetValue(PreferenceState::Preference::C_SSAO_BLUR_STRENGTH, m_blurStrength);
	preferences.addCallback(PreferenceState::Preference::C_SSAO_BLUR_STRENGTH, m_aliveIndicator, [&](const float& f) noexcept { m_blurStrength = (int)f; });

	// Prepare the noise texture and kernel
	const std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
	std::default_random_engine generator;
	glm::vec3 noiseArray[16]{};
	for (GLuint i = 0; i < 16; i++) 
		noiseArray[i] = glm::vec3(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f);	
	glCreateTextures(GL_TEXTURE_2D, 1, &m_noiseID);
	glTextureStorage2D(m_noiseID, 1, GL_RGB16F, 4, 4);
	glTextureSubImage2D(m_noiseID, 0, 0, 0, 4, 4, GL_RGB, GL_FLOAT, &noiseArray[0]);
	glTextureParameteri(m_noiseID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_noiseID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(m_noiseID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_noiseID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	m_shader->addCallback(m_aliveIndicator, [&] {
		const std::uniform_real_distribution<GLfloat> rdmFloats(0.0, 1.0);
		std::default_random_engine gen;
		glm::vec4 new_kernel[MAX_KERNEL_SIZE]{};
		for (int i = 0, t = 0; i < MAX_KERNEL_SIZE; i++, t++) {
			glm::vec3 sample(
				rdmFloats(gen) * 2.0 - 1.0,
				rdmFloats(gen) * 2.0 - 1.0,
				rdmFloats(gen)
			);
			sample = glm::normalize(sample);
			sample *= rdmFloats(gen);
			GLfloat scale = GLfloat(i) / (GLfloat)(MAX_KERNEL_SIZE);
			scale = 0.1f + (scale * scale) * (1.0f - 0.1f);
			sample *= scale;
			new_kernel[t] = glm::vec4(sample, 1);
		}
		m_shader->setUniform(0, m_radius);
		m_shader->setUniform(1, m_quality);
		m_shader->setUniformArray(3, new_kernel, MAX_KERNEL_SIZE);
		});

	// Error Reporting
	auto& msgMgr = m_engine.getManager_Messages();
	if (!glIsTexture(m_noiseID))
		msgMgr.error("SSAO Noise Texture is incomplete.");
}

void SSAO::clearCache(const float&) noexcept
{
	m_drawIndex = 0;
}

void SSAO::renderTechnique(const float&, Viewport& viewport, const std::vector<std::pair<int, int>>& perspectives)
{
	if (!m_enabled || !Asset::All_Ready(m_shapeQuad, m_shader, m_shaderCopyAO, m_shaderGB_A))
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

	// Bind common data
	camBufferIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	m_shader->bind();
	viewport.m_gfxFBOS.bindForWriting("SSAO");
	viewport.m_gfxFBOS.bindForReading("GEOMETRY", 0);
	glBindTextureUnit(6, m_noiseID);
	glBindVertexArray(m_shapeQuad->m_vaoID);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	indirectQuad.bind();
	glDrawArraysIndirect(GL_TRIANGLES, nullptr);

	// Gaussian blur the lines we got from the SSAO pass
	size_t aoSpot = 0;
	if (m_blurStrength > 0) {
		// Clear the second attachment
		constexpr GLfloat clearRed = 0.0f;
		glClearTexImage(viewport.m_gfxFBOS.getTexID("SSAO", 1), 0, GL_RED, GL_FLOAT, &clearRed);

		// Read from desired texture, blur into this frame buffer
		bool horizontal = false;
		glBindTextureUnit(0, viewport.m_gfxFBOS.getTexID("SSAO", 0));
		glBindTextureUnit(1, viewport.m_gfxFBOS.getTexID("SSAO", 1));
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + (int)(horizontal));
		m_shaderGB_A->bind();
		m_shaderGB_A->setUniform(0, horizontal);
		m_shaderGB_A->setUniform(1, glm::vec2(viewport.m_dimensions));

		// Blur remainder of the times
		for (int i = 0; i < m_blurStrength; i++) {
			horizontal = !horizontal;
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + (int)(horizontal));
			m_shaderGB_A->setUniform(0, horizontal);
			glDrawArraysIndirect(GL_TRIANGLES, nullptr);
		}
		aoSpot = horizontal ? 1ull : 0ull;
	}

	// Overlay SSAO on top of AO channel of Geometry Buffer
	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_ONE, GL_ONE, GL_DST_ALPHA, GL_ZERO);
	m_shaderCopyAO->bind();
	viewport.m_gfxFBOS.bindForWriting("GEOMETRY");
	glDrawBuffer(GL_COLOR_ATTACHMENT2);
	glBindTextureUnit(0, viewport.m_gfxFBOS.getTexID("SSAO", aoSpot));
	glDrawArraysIndirect(GL_TRIANGLES, nullptr);

	constexpr GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, drawBuffers);
	glDisable(GL_BLEND);
	camBufferIndex.endReading();
	indirectQuad.endReading();
	Shader::Release();
	m_drawIndex++;
}