#include "Systems\Graphics\Geometry_Buffer.h"
#include "Systems\Graphics\VisualFX.h"
#include "Managers\Message_Manager.h"
#include <algorithm>
#include <random>

class Primitive_Observer : Asset_Observer
{
public:
	Primitive_Observer(Shared_Asset_Primitive &asset, const GLuint vao) : Asset_Observer(asset.get()), m_vao_id(vao), m_asset(asset) {};
	virtual ~Primitive_Observer() { m_asset->removeObserver(this); };
	virtual void Notify_Finalized() {
		if (m_asset->existsYet()) // in case this gets used more than once by mistake
			m_asset->updateVAO(m_vao_id);
	}

	GLuint m_vao_id;
	Shared_Asset_Primitive m_asset;
};

static void AssignTextureProperties()
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

Geometry_Buffer::~Geometry_Buffer()
{	
	if (m_Initialized) {
		// Destroy OpenGL objects
		glDeleteTextures(GBUFFER_NUM_TEXTURES, m_textures);
		glDeleteTextures(2, m_texturesGB);
		glDeleteTextures(1, &m_depth_stencil);
		glDeleteFramebuffers(1, &m_fbo);

		delete m_observer;
	}
}

Geometry_Buffer::Geometry_Buffer()
{	
	m_Initialized = false;
	m_fbo = 0;
	m_depth_stencil = 0;
	m_vao_Quad = 0;
	for (int x = 0; x < GBUFFER_NUM_TEXTURES; ++x)
		m_textures[x] = 0;
	for (int x = 0; x < 2; ++x)
		m_texturesGB[x] = 0;
}

void Geometry_Buffer::Initialize(const vec2 &size, VisualFX *visualFX)
{
	if (!m_Initialized) {
		m_visualFX = visualFX;
		Asset_Loader::load_asset(m_shaderSSAO, "FX\\SSAO");
		Asset_Loader::load_asset(m_shapeQuad, "quad");
		m_vao_Quad = Asset_Primitive::Generate_VAO();
		m_observer = (void*)(new Primitive_Observer(m_shapeQuad, m_vao_Quad));
		m_renderSize = size;
		
		// Create the FBO
		glGenFramebuffers(1, &m_fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

		// Create the gbuffer textures
		glGenTextures(GBUFFER_NUM_TEXTURES, m_textures);
		glGenTextures(1, &m_depth_stencil);

		for (int x = 0; x < GBUFFER_NUM_TEXTURES; ++x) {
			glBindTexture(GL_TEXTURE_2D, m_textures[x]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_renderSize.x, m_renderSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
			AssignTextureProperties();
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + x, GL_TEXTURE_2D, m_textures[x], 0);
		}

		// Depth-stencil buffer texture
		glBindTexture(GL_TEXTURE_2D, m_depth_stencil);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_renderSize.x, m_renderSize.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		AssignTextureProperties();
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depth_stencil, 0);

		GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR) {
			std::string errorString = std::string(reinterpret_cast<char const *>(glewGetErrorString(Status)));
			MSG::Error(FBO_INCOMPLETE, "Geometry Buffer", errorString);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			return;
		}
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		glGenTextures(2, m_texturesGB);
		for (int x = 0; x < 2; ++x) {
			glBindTexture(GL_TEXTURE_2D, m_texturesGB[x]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_renderSize.x, m_renderSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
			AssignTextureProperties();
		}

		// Prepare the noise texture and kernal	
		std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
		std::default_random_engine generator;
		vec3 noiseArray[16];
		for (GLuint i = 0; i < 16; i++) {
			glm::vec3 noise(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				0.0f);
			noiseArray[i] = (noise);
		}
		glGenTextures(1, &m_noiseID);
		glBindTexture(GL_TEXTURE_2D, m_noiseID);
		AssignTextureProperties();
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &noiseArray[0]);
		glBindTexture(GL_TEXTURE_2D, 0);
		m_Initialized = true;
	}
}

void Geometry_Buffer::Clear()
{
	BindForWriting();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Geometry_Buffer::BindForWriting()
{
	GLenum DrawBuffers[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2
	};
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glDrawBuffers(GBUFFER_NUM_TEXTURES, DrawBuffers);
}

void Geometry_Buffer::BindForReading()
{
	for (unsigned int i = 0; i < GBUFFER_NUM_TEXTURES; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textures[i]);
	}
}

void Geometry_Buffer::End()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

	// Return the borrowed depth-stencil texture
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depth_stencil, 0);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Geometry_Buffer::Resize(const vec2 &size)
{
	m_renderSize = size;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

	for (int x = 0; x < GBUFFER_NUM_TEXTURES; ++x) {
		glBindTexture(GL_TEXTURE_2D, m_textures[x]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, size.x, size.y, 0, GL_RGB, GL_FLOAT, NULL);
	}

	for (int x = 0; x < 2; ++x) {
		glBindTexture(GL_TEXTURE_2D, m_texturesGB[x]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, size.x, size.y, 0, GL_RGBA, GL_FLOAT, NULL);
	}

	// Depth-stencil buffer texture
	glBindTexture(GL_TEXTURE_2D, m_depth_stencil);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, size.x, size.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

	// restore default FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Geometry_Buffer::ApplyAO()
{
	if (m_shapeQuad->existsYet() && m_shaderSSAO->existsYet()) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
		glDrawBuffer(GL_COLOR_ATTACHMENT2);

		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFuncSeparate(GL_ONE, GL_ONE, GL_DST_ALPHA, GL_ZERO);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_IMAGE]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_VIEWNORMAL]);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_noiseID);

		m_shaderSSAO->bind();
		glBindVertexArray(m_vao_Quad);
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
