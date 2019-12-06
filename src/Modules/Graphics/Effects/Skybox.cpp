#include "Modules/Graphics/Effects/Skybox.h"
#include "Engine.h"


Skybox::~Skybox() noexcept 
{
	// Update indicator
	*m_aliveIndicator = false;
}

Skybox::Skybox(Engine& engine) noexcept :
	Graphics_Technique(Technique_Category::PRIMARY_LIGHTING),
	m_engine(engine),
	m_cubemapSky(Shared_Cubemap(engine, "sky\\")),
	m_shaderSky(Shared_Shader(engine, "Effects\\Skybox")),
	m_shaderSkyReflect(Shared_Shader(engine, "Effects\\Skybox Reflection")),
	m_shaderConvolute(Shared_Shader(engine, "Effects\\Sky_Convolution")),
	m_shapeQuad(Shared_Auto_Model(engine, "quad"))
{
	glCreateFramebuffers(1, &m_cubeFBO);
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_cubemapMipped);
	glTextureParameteri(m_cubemapMipped, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(m_cubemapMipped, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(m_cubemapMipped, GL_TEXTURE_MIN_LOD, 0);
	glTextureParameteri(m_cubemapMipped, GL_TEXTURE_MAX_LOD, 5);
	glTextureParameteri(m_cubemapMipped, GL_TEXTURE_BASE_LEVEL, 0);
	glTextureParameteri(m_cubemapMipped, GL_TEXTURE_MAX_LEVEL, 5);
	glTextureParameteri(m_cubemapMipped, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_cubemapMipped, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_cubemapMipped, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	m_cubemapSky->addCallback(m_aliveIndicator, [&](void) mutable {
		m_skyOutOfDate = true;
		m_skySize = m_cubemapSky->m_images[0]->m_size;
		glTextureStorage2D(m_cubemapMipped, 6, GL_RGB16F, m_skySize.x, m_skySize.x);
		for (int x = 0; x < 6; ++x) 
			glTextureSubImage3D(m_cubemapMipped, 0, 0, 0, x, m_skySize.x, m_skySize.x, 1, GL_RGBA, GL_UNSIGNED_BYTE, &m_cubemapSky->m_images[x]->m_pixelData[0]);		
		glNamedFramebufferTexture(m_cubeFBO, GL_COLOR_ATTACHMENT0, m_cubemapMipped, 0);
		glNamedFramebufferDrawBuffer(m_cubeFBO, GL_COLOR_ATTACHMENT0);

		// Error Reporting
		if (glCheckNamedFramebufferStatus(m_cubeFBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			m_engine.getManager_Messages().error("Skybox Framebuffer has encountered an error.");
		if (!glIsTexture(m_cubemapMipped))
			m_engine.getManager_Messages().error("Skybox Texture is incomplete.");
		});
}

void Skybox::clearCache(const float&) noexcept 
{
	m_drawIndex = 0;
}

void Skybox::renderTechnique(const float&, const std::shared_ptr<Viewport>& viewport, const std::vector<std::pair<int, int>>& perspectives) noexcept 
{
	if (!m_enabled || !Asset::All_Ready(m_shapeQuad, m_shaderSky, m_shaderSkyReflect, m_shaderConvolute, m_cubemapSky))
		return;

	// Prepare camera index
	if (m_drawIndex >= m_drawData.size())
		m_drawData.resize(size_t(m_drawIndex) + 1ull);

	if (m_skyOutOfDate) {
		convoluteSky(viewport);
		m_skyOutOfDate = false;
	}

	auto& [camBufferIndex, indirectQuad, quad6IndirectBuffer] = m_drawData[m_drawIndex];
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
	glDisable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glBindVertexArray(m_shapeQuad->m_vaoID);
	indirectQuad.bind();
	glBindTextureUnit(4, m_cubemapMipped);

	// Render skybox to reflection buffer
	m_shaderSkyReflect->bind();
	viewport->m_gfxFBOS.bindForReading("GEOMETRY", 0);
	viewport->m_gfxFBOS.bindForWriting("REFLECTION");
	glDrawArraysIndirect(GL_TRIANGLES, 0);

	// Render skybox to lighting buffer
	m_shaderSky->bind();
	viewport->m_gfxFBOS.bindForWriting("LIGHTING");
	glDrawArraysIndirect(GL_TRIANGLES, 0);

	glDisable(GL_DEPTH_TEST);
	camBufferIndex.endReading();
	indirectQuad.endReading();
	Shader::Release();
	m_drawIndex++;
}

void Skybox::convoluteSky(const std::shared_ptr<Viewport>& viewport) noexcept 
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glBindVertexArray(m_shapeQuad->m_vaoID);
	m_shaderConvolute->bind();
	m_shaderConvolute->setUniform(0, 0);
	GLint previousFBO(0);
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previousFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_cubeFBO);
	glBindTextureUnit(0, m_cubemapMipped);
	auto& quad6IndirectBuffer = m_drawData[m_drawIndex].quad6IndirectBuffer;
	quad6IndirectBuffer.beginWriting();
	quad6IndirectBuffer.endWriting();
	quad6IndirectBuffer.bind();

	for (unsigned int r = 1; r < 6; ++r) {
		// Ensure we are writing to MIP level r
		const unsigned int write_size = (unsigned int)std::max(1.0f, (floor((float)m_skySize.x / pow(2.0f, (float)r))));
		glViewport(0, 0, write_size, write_size);
		m_shaderConvolute->setUniform(1, (float)r / 5.0f);
		glNamedFramebufferTexture(m_cubeFBO, GL_COLOR_ATTACHMENT0, m_cubemapMipped, r);

		// Ensure we are reading from MIP level r - 1
		glTextureParameteri(m_cubemapMipped, GL_TEXTURE_BASE_LEVEL, r - 1);
		glTextureParameteri(m_cubemapMipped, GL_TEXTURE_MAX_LEVEL, r - 1);

		// Convolute the 6 faces for this roughness level (RENDERS 6 TIMES)
		glDrawArraysIndirect(GL_TRIANGLES, 0);
	}

	// Reset
	quad6IndirectBuffer.endReading();
	glTextureParameteri(m_cubemapMipped, GL_TEXTURE_BASE_LEVEL, 0);
	glTextureParameteri(m_cubemapMipped, GL_TEXTURE_MAX_LEVEL, 5);
	glNamedFramebufferTexture(m_cubeFBO, GL_COLOR_ATTACHMENT0, m_cubemapMipped, 0);
	glViewport(0, 0, GLsizei(viewport->m_dimensions.x), GLsizei(viewport->m_dimensions.y));
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, previousFBO);
	Shader::Release();
}