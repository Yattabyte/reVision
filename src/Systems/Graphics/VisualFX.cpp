#include "Systems\Graphics\VisualFX.h"
#include "Utilities\Engine_Package.h"

class Primitive_Observer : Asset_Observer
{
public:
	Primitive_Observer(Shared_Asset_Primitive &asset, const GLuint vao) : Asset_Observer(asset.get()), m_vao_id(vao), m_asset(asset) {};
	virtual ~Primitive_Observer() { m_asset->RemoveObserver(this); };
	virtual void Notify_Finalized() {
		if (m_asset->ExistsYet()) // in case this gets used more than once by mistake
			m_asset->UpdateVAO(m_vao_id);
	}

	GLuint m_vao_id;
	Shared_Asset_Primitive m_asset;
};

VisualFX::~VisualFX()
{
	if (m_Initialized) {
		delete m_observer;
	}
}

VisualFX::VisualFX()
{
	m_Initialized = false;
	m_vao_Quad = 0;
	m_fbo_GB = 0;
}

void VisualFX::Initialize(Engine_Package * enginePackage)
{
	if (!m_Initialized) {
		m_enginePackage = enginePackage;
		Asset_Loader::load_asset(m_shapeQuad, "quad");
		m_vao_Quad = Asset_Primitive::GenerateVAO();
		m_observer = (void*)(new Primitive_Observer(m_shapeQuad, m_vao_Quad));

		Initialize_CubeFilter();
		Initialize_GausianBlur();

		m_Initialized = true;
	}
}

void VisualFX::Initialize_CubeFilter()
{
}

void VisualFX::Initialize_GausianBlur()
{
	glGenFramebuffers(1, &m_fbo_GB);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_GB);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	Asset_Loader::load_asset(m_shaderGB, "FX\\gaussianBlur");
	Asset_Loader::load_asset(m_shaderGB_A, "FX\\gaussianBlur_Alpha");
}

void VisualFX::applyGaussianBlur(const GLuint & desiredTexture, const GLuint * flipTextures, const vec2 & size, const int & amount)
{
	if (desiredTexture && m_shapeQuad->ExistsYet()) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo_GB);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, flipTextures[0], 0);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, flipTextures[1], 0);
		glViewport(0, 0, size.x, size.y);

		// Read from desired texture, blur into this frame buffer
		GLboolean horizontal = false;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, desiredTexture);
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
		m_shaderGB->Bind();
		m_shaderGB->setLocationValue(0, horizontal);

		glBindVertexArray(m_vao_Quad);
		const int quad_size = m_shapeQuad->GetSize();
		glDrawArrays(GL_TRIANGLES, 0, quad_size);

		// Blur remainder of the times minus 1
		for (int i = 1; i < amount - 1; i++) {
			horizontal = !horizontal;
			glBindTexture(GL_TEXTURE_2D, flipTextures[!horizontal]);
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
			m_shaderGB->setLocationValue(0, horizontal);

			glDrawArrays(GL_TRIANGLES, 0, quad_size);
		}

		// Last blur back into desired texture
		horizontal = !horizontal;
		glBindTexture(GL_TEXTURE_2D, flipTextures[!horizontal]);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, desiredTexture, 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		m_shaderGB->setLocationValue(0, horizontal);

		glDrawArrays(GL_TRIANGLES, 0, quad_size);

		glBindVertexArray(0);
		Asset_Shader::Release();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void VisualFX::applyGaussianBlur_Alpha(const GLuint & desiredTexture, const GLuint * flipTextures, const vec2 & size, const int & amount)
{
	if (desiredTexture && m_shapeQuad->ExistsYet()) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo_GB);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, flipTextures[0], 0);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, flipTextures[1], 0);
		glViewport(0, 0, size.x, size.y);

		// Read from desired texture, blur into this frame buffer
		GLboolean horizontal = false;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, desiredTexture);
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
		glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);
		m_shaderGB_A->Bind();
		m_shaderGB_A->setLocationValue(0, horizontal);

		glBindVertexArray(m_vao_Quad);
		const int quad_size = m_shapeQuad->GetSize();
		glDrawArrays(GL_TRIANGLES, 0, quad_size);

		// Blur remainder of the times minus 1
		for (int i = 1; i < amount - 1; i++) {
			horizontal = !horizontal;
			glBindTexture(GL_TEXTURE_2D, flipTextures[!horizontal]);
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
			m_shaderGB_A->setLocationValue(0, horizontal);

			glDrawArrays(GL_TRIANGLES, 0, quad_size);
		}

		// Last blur back into desired texture
		horizontal = !horizontal;
		glBindTexture(GL_TEXTURE_2D, flipTextures[!horizontal]);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, desiredTexture, 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		m_shaderGB_A->setLocationValue(0, horizontal);

		glDrawArrays(GL_TRIANGLES, 0, quad_size);

		glBindVertexArray(0);
		Asset_Shader::Release();
		glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}