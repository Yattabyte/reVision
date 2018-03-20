#include "Systems\Graphics\Frame Buffers\Geometry_Buffer.h"
#include "Utilities\EnginePackage.h"
#include "Systems\Graphics\VisualFX.h"
#include <algorithm>
#include <random>


static void AssignTextureProperties(const GLuint & texID)
{
	glTextureParameteri(texID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(texID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(texID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(texID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

Geometry_Buffer::~Geometry_Buffer()
{	
	if (m_Initialized) {
		// Destroy OpenGL objects
		glDeleteTextures(1, &m_noiseID);
		glDeleteTextures(2, m_texturesGB);
		glDeleteTextures(1, &m_depth_stencil);
		glDeleteTextures(GBUFFER_NUM_TEXTURES, m_textures);
		if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
		m_enginePackage->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
		m_enginePackage->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);
	}
}

Geometry_Buffer::Geometry_Buffer()
{		
	m_depth_stencil = 0;
	m_quadVAO = 0;
	for (int x = 0; x < GBUFFER_NUM_TEXTURES; ++x)
		m_textures[x] = 0;
	for (int x = 0; x < 2; ++x)
		m_texturesGB[x] = 0;
}

void Geometry_Buffer::initialize(EnginePackage * enginePackage, VisualFX * visualFX)
{
	if (!m_Initialized) {
		m_enginePackage = enginePackage;
		m_visualFX = visualFX;
		Asset_Loader::load_asset(m_shaderSSAO, "FX\\SSAO");
		Asset_Loader::load_asset(m_shapeQuad, "quad");
		m_quadVAO = Asset_Primitive::Generate_VAO();
		m_shapeQuad->addCallback(this, [&]() { m_shapeQuad->updateVAO(m_quadVAO); });
		m_renderSize.x = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {resize(ivec2(f, m_renderSize.y)); });
		m_renderSize.y = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {resize(ivec2(m_renderSize.x, f)); });
		initialize_noise();
		Frame_Buffer::initialize();

		// Create the gbuffer textures
		glCreateTextures(GL_TEXTURE_2D, GBUFFER_NUM_TEXTURES, m_textures);
		for (int x = 0; x < GBUFFER_NUM_TEXTURES; ++x) {
			glTextureImage2DEXT(m_textures[x], GL_TEXTURE_2D, 0, GL_RGBA32F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
			AssignTextureProperties(m_textures[x]);
			glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0 + x, m_textures[x], 0);
		}
		// Depth-stencil buffer texture
		glCreateTextures(GL_TEXTURE_2D, 1, &m_depth_stencil);
		glTextureImage2DEXT(m_depth_stencil, GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_renderSize.x, m_renderSize.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		AssignTextureProperties(m_depth_stencil);
		glNamedFramebufferTexture(m_fbo, GL_DEPTH_STENCIL_ATTACHMENT, m_depth_stencil, 0);
		validate();
	}
}

void Geometry_Buffer::initialize_noise()
{
	if (!m_Initialized) {
		glCreateTextures(GL_TEXTURE_2D, 2, m_texturesGB);
		for (int x = 0; x < 2; ++x) {
			glTextureImage2DEXT(m_texturesGB[x], GL_TEXTURE_2D, 1, GL_RGBA32F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
			AssignTextureProperties(m_texturesGB[x]);
		}

		// Prepare the noise texture and kernal	
		std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
		std::default_random_engine generator;
		vec3 noiseArray[16];
		for (GLuint i = 0; i < 16; i++) {
			glm::vec3 noise( randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f);
			noiseArray[i] = (noise);
		}
		glCreateTextures(GL_TEXTURE_2D, 1, &m_noiseID);
		glTextureStorage2D(m_noiseID, 1, GL_RGB16F, 4, 4);
		glTextureSubImage2D(m_noiseID, 0, 0, 0, 4, 4, GL_RGB, GL_FLOAT, &noiseArray[0]);
		AssignTextureProperties(m_noiseID);
	}
}

void Geometry_Buffer::clear()
{
	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	GLfloat clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	GLfloat clearDepth = 1.0f;
	GLint clearStencil = 0;
	glNamedFramebufferDrawBuffers(m_fbo, GBUFFER_NUM_TEXTURES, DrawBuffers);
	for (int x = 0; x < GBUFFER_NUM_TEXTURES; ++x)
		glClearNamedFramebufferfv(m_fbo, GL_COLOR, x, clearColor);
	glClearNamedFramebufferfi(m_fbo, GL_DEPTH_STENCIL, 0, clearDepth, clearStencil);
}

void Geometry_Buffer::bindForWriting()
{
	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glNamedFramebufferDrawBuffers(m_fbo, GBUFFER_NUM_TEXTURES, DrawBuffers);
}

void Geometry_Buffer::bindForReading()
{
	for (unsigned int i = 0; i < GBUFFER_NUM_TEXTURES; i++) 
		glBindTextureUnit(i, m_textures[i]);	
}

void Geometry_Buffer::resize(const ivec2 & size)
{
	Frame_Buffer::resize(size);

	// Main textures
	for (int x = 0; x < GBUFFER_NUM_TEXTURES; ++x) {
		glTextureImage2DEXT(m_textures[x], GL_TEXTURE_2D, 0, GL_RGBA32F, size.x, size.y, 0, GL_RGBA, GL_FLOAT, NULL);
		glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0 + x, m_textures[x], 0);
	}

	// Depth-stencil buffer texture
	glTextureImage2DEXT(m_depth_stencil, GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, size.x, size.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glNamedFramebufferTexture(m_fbo, GL_DEPTH_STENCIL_ATTACHMENT, m_depth_stencil, 0);
	
	// Gaussian blur textures
	for (int x = 0; x < 2; ++x) 
		glTextureImage2DEXT(m_texturesGB[x], GL_TEXTURE_2D, 0, GL_RGBA32F, size.x, size.y, 0, GL_RGBA, GL_FLOAT, NULL);	
}

void Geometry_Buffer::end()
{
	// Return the borrowed depth-stencil texture
	glNamedFramebufferTexture(m_fbo, GL_DEPTH_STENCIL_ATTACHMENT, m_depth_stencil, 0);
}

void Geometry_Buffer::applyAO()
{
	if (m_shapeQuad->existsYet() && m_shaderSSAO->existsYet()) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
		glDrawBuffer(GL_COLOR_ATTACHMENT2);

		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFuncSeparate(GL_ONE, GL_ONE, GL_DST_ALPHA, GL_ZERO);

		glBindTextureUnit(0, m_textures[GBUFFER_TEXTURE_TYPE_IMAGE]);
		glBindTextureUnit(1, m_textures[GBUFFER_TEXTURE_TYPE_VIEWNORMAL]);
		glBindTextureUnit(2, m_noiseID);

		m_shaderSSAO->bind();
		glBindVertexArray(m_quadVAO);
		const size_t &quad_size = m_shapeQuad->getSize();
		glDrawArrays(GL_TRIANGLES, 0, quad_size);
		glBindVertexArray(0);
		Asset_Shader::Release();

		m_visualFX->applyGaussianBlur_Alpha(m_textures[GBUFFER_TEXTURE_TYPE_SPECULAR], m_texturesGB, m_renderSize, 5);

		glEnable(GL_DEPTH_TEST);
		glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
		glBlendFunc(GL_ONE, GL_ZERO);
		glDisable(GL_BLEND);
	}
}
