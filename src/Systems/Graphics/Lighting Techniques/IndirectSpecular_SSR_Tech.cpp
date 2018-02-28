#include "Systems\Graphics\Lighting Techniques\IndirectSpecular_SSR_Tech.h"
#include "Systems\GraphiCS\Frame Buffers\Geometry_Buffer.h"
#include "Systems\GraphiCS\Frame Buffers\Lighting_Buffer.h"
#include "Systems\Graphics\VisualFX.h"
#include "Managers\Message_Manager.h"
#include "Utilities\EnginePackage.h"
#include <minmax.h>

struct Primitiveee_Observer : Asset_Observer {
	Primitiveee_Observer(Shared_Asset_Primitive & asset, const GLuint vao) : Asset_Observer(asset.get()), m_vao_id(vao) {};
	virtual void Notify_Finalized() {
		if (m_asset->existsYet())
			dynamic_pointer_cast<Asset_Primitive>(m_asset)->updateVAO(m_vao_id);
	}
	GLuint m_vao_id;
};

IndirectSpecular_SSR_Tech::~IndirectSpecular_SSR_Tech()
{
	glDeleteBuffers(1, &m_ssrUBO);
	glDeleteTextures(1, &m_texture);
	glDeleteFramebuffers(1, &m_fbo);
	m_enginePackage->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
	m_enginePackage->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);
	delete m_QuadObserver; 
}

IndirectSpecular_SSR_Tech::IndirectSpecular_SSR_Tech(EnginePackage * enginePackage, Geometry_Buffer * gBuffer, Lighting_Buffer * lBuffer, VisualFX * visualFX)
{
	m_enginePackage = enginePackage;
	m_gBuffer = gBuffer;
	m_lBuffer = lBuffer;
	m_visualFX = visualFX;
	m_fbo = 0;
	m_texture = 0;
	m_ssrUBO = 0;

	Asset_Loader::load_asset(m_shaderCopy, "fx\\copyTexture");
	Asset_Loader::load_asset(m_shaderBlur, "fx\\gaussianBlur_MIP");
	Asset_Loader::load_asset(m_shaderSSR, "Lighting\\ssr");
	Asset_Loader::load_asset(m_brdfMap, "brdfLUT.png");
	Asset_Loader::load_asset(m_shapeQuad, "quad");
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_QuadObserver = (void*)(new Primitiveee_Observer(m_shapeQuad, m_quadVAO)); 
	m_renderSize.x = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {resize(vec2(f, m_renderSize.y)); });
	m_renderSize.y = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {resize(vec2(m_renderSize.x, f)); });

	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 5);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 5); 
	for (int x = 0; x < 6; ++x) {
		ivec2 size(floor(m_renderSize.x / pow(2, x)), floor(m_renderSize.y / pow(2, x)));
		glTexImage2D(GL_TEXTURE_2D, x, GL_RGB16F, size.x, size.y, 0, GL_RGB, GL_FLOAT, NULL);
	}
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR) {
		std::string errorString = std::string(reinterpret_cast<char const *>(glewGetErrorString(Status)));
		MSG_Manager::Error(MSG_Manager::FBO_INCOMPLETE, "Lighting Buffer", errorString);

		// Delete before returning
		glDeleteTextures(1, &m_texture);
		glDeleteFramebuffers(1, &m_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenBuffers(1, &m_ssrUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_ssrUBO);
	glBindBufferBase(GL_UNIFORM_BUFFER, 6, m_ssrUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SSR_Buffer), &m_ssrBuffer, GL_DYNAMIC_COPY);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void IndirectSpecular_SSR_Tech::resize(const vec2 & size)
{
	m_renderSize = size;
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	for (int x = 0; x < 6; ++x) {
		ivec2 size(floor(m_renderSize.x / pow(2, x)), floor(m_renderSize.y / pow(2, x)));
		glTexImage2D(GL_TEXTURE_2D, x, GL_RGB16F, size.x, size.y, 0, GL_RGB, GL_FLOAT, NULL);
	}
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void IndirectSpecular_SSR_Tech::updateLighting(const Visibility_Token & vis_token)
{
	
}

void IndirectSpecular_SSR_Tech::applyLighting(const Visibility_Token & vis_token)
{
	blurLight();
	reflectLight();
}

void IndirectSpecular_SSR_Tech::bindForWriting(const GLuint &bounceSpot)
{
	
}

void IndirectSpecular_SSR_Tech::bindForReading(const GLuint &bounceSpot, const GLuint textureUnit)
{
	
}

void IndirectSpecular_SSR_Tech::blurLight()
{
	const int quad_size = m_shapeQuad->getSize();
	glBindVertexArray(m_quadVAO);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	// Copy lighting texture to one with a MIP chain
	m_shaderCopy->bind();
	m_lBuffer->bindForReading();
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, quad_size);

	// Blur MIP chain, reading from 1 MIP level and writing into next
	m_shaderBlur->bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	for (int horizontal = 0; horizontal < 2; ++horizontal) {
		Asset_Shader::Set_Uniform(0, horizontal);
		ivec2 read_size = m_renderSize;
		for (int x = 1; x < 6; ++x) {
			// Ensure we are reading from MIP level x - 1
			Asset_Shader::Set_Uniform(1, read_size);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, x - 1);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, x - 1);
			// Ensure we are writing to MIP level x
			ivec2 write_size(floor(m_renderSize.x / pow(2, x)), floor(m_renderSize.y / pow(2, x)));
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, x);

			glViewport(0, 0, max(1.0f, write_size.x), max(1.0f, write_size.y));
			glDrawArrays(GL_TRIANGLES, 0, quad_size);
			read_size = write_size;
		}
		// Blend second pass
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
	}

	// Restore to default
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 5);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindVertexArray(0);
	glViewport(0, 0, m_renderSize.x, m_renderSize.y);
	Asset_Shader::Release();
	// Maintain state for next function call: reflectLight()
}

void IndirectSpecular_SSR_Tech::reflectLight()
{
	const int quad_size = m_shapeQuad->getSize();
	m_lBuffer->bindForWriting();

	glBindVertexArray(m_quadVAO);
	m_shaderSSR->bind();
	m_gBuffer->bindForReading(); // Gbuffer
	m_brdfMap->bind(GL_TEXTURE4); // BRDF LUT
	glBindMultiTextureEXT(GL_TEXTURE5, GL_TEXTURE_2D, m_texture); // blurred light MIP-chain
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE); 
	glBindBuffer(GL_UNIFORM_BUFFER, m_ssrUBO);
	glBindBufferBase(GL_UNIFORM_BUFFER, 6, m_ssrUBO);
	glDrawArrays(GL_TRIANGLES, 0, quad_size);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_ONE, GL_ZERO);
	glDisable(GL_BLEND);
	glCullFace(GL_BACK);
	Asset_Shader::Release();
}
