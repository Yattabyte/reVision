#include "Systems\Graphics\Frame Buffers\Geometry_Buffer.h"
#include "Utilities\EnginePackage.h"
#include "Systems\Graphics\VisualFX.h"
#include <algorithm>
#include <random>


static void AssignTextureProperties(const GLuint & texID)
{
	glTextureParameteriEXT(texID, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteriEXT(texID, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteriEXT(texID, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteriEXT(texID, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
		glGenTextures(GBUFFER_NUM_TEXTURES, m_textures);
		for (int x = 0; x < GBUFFER_NUM_TEXTURES; ++x) {
			glTextureImage2DEXT(m_textures[x], GL_TEXTURE_2D, 0, GL_RGBA32F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
			AssignTextureProperties(m_textures[x]);
			glNamedFramebufferTexture2DEXT(m_fbo, GL_COLOR_ATTACHMENT0 + x, GL_TEXTURE_2D, m_textures[x], 0);
		}
		// Depth-stencil buffer texture
		glGenTextures(1, &m_depth_stencil);
		glTextureImage2DEXT(m_depth_stencil, GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_renderSize.x, m_renderSize.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		AssignTextureProperties(m_depth_stencil);
		glNamedFramebufferTexture2DEXT(m_fbo, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depth_stencil, 0);
		validate();
	}
}

void Geometry_Buffer::initialize_noise()
{
	if (!m_Initialized) {
		glGenTextures(2, m_texturesGB);
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
		glGenTextures(1, &m_noiseID);
		glTextureImage2DEXT(m_noiseID, GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &noiseArray[0]);
		AssignTextureProperties(m_noiseID);
	}
}

void Geometry_Buffer::clear()
{
	bindForWriting();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Geometry_Buffer::bindForWriting()
{
	GLenum DrawBuffers[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2
	};
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glDrawBuffers(GBUFFER_NUM_TEXTURES, DrawBuffers);
}

void Geometry_Buffer::bindForReading()
{
	for (unsigned int i = 0; i < GBUFFER_NUM_TEXTURES; i++) 
		glBindMultiTextureEXT(GL_TEXTURE0 + i, GL_TEXTURE_2D, m_textures[i]);	
}

void Geometry_Buffer::resize(const ivec2 & size)
{
	Frame_Buffer::resize(size);

	// Main textures
	for (int x = 0; x < GBUFFER_NUM_TEXTURES; ++x) {
		glTextureImage2DEXT(m_textures[x], GL_TEXTURE_2D, 0, GL_RGBA32F, size.x, size.y, 0, GL_RGBA, GL_FLOAT, NULL);
		glNamedFramebufferTexture2DEXT(m_fbo, GL_COLOR_ATTACHMENT0 + x, GL_TEXTURE_2D, m_textures[x], 0);
	}

	// Depth-stencil buffer texture
	glTextureImage2DEXT(m_depth_stencil, GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, size.x, size.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glNamedFramebufferTexture2DEXT(m_fbo, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depth_stencil, 0);
	
	// Gaussian blur textures
	for (int x = 0; x < 2; ++x) 
		glTextureImage2DEXT(m_texturesGB[x], GL_TEXTURE_2D, 0, GL_RGBA32F, size.x, size.y, 0, GL_RGBA, GL_FLOAT, NULL);	
}

void Geometry_Buffer::end()
{
	// Return the borrowed depth-stencil texture
	glNamedFramebufferTexture2DEXT(m_fbo, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depth_stencil, 0);
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

		glBindMultiTextureEXT(GL_TEXTURE0, GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_IMAGE]);
		glBindMultiTextureEXT(GL_TEXTURE1, GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_VIEWNORMAL]);
		glBindMultiTextureEXT(GL_TEXTURE2, GL_TEXTURE_2D, m_noiseID);

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
