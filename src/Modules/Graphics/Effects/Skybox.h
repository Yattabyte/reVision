#pragma once
#ifndef SKYBOX_H
#define SKYBOX_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Assets/Shader.h"
#include "Assets/Cubemap.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/StaticTripleBuffer.h"
#include "Engine.h"


/** A core-rendering technique for writing the frame time to the screen. */
class Skybox : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline ~Skybox() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	inline Skybox(Engine * engine)
		: m_engine(engine), Graphics_Technique(SECONDARY_LIGHTING) {
		// Asset Loading
		m_cubemapSky = Shared_Cubemap(engine, "night\\");
		m_shaderSky = Shared_Shader(engine, "Effects\\Skybox");
		m_shaderSkyReflect = Shared_Shader(engine, "Effects\\Skybox Reflection");
		m_shaderConvolute = Shared_Shader(engine, "Effects\\Sky_Convolution");
		m_shapeQuad = Shared_Primitive(engine, "quad");

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

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_quadIndirectBuffer = StaticTripleBuffer(sizeof(GLuint) * 4, quadData);
			const GLuint quad6Data[4] = { (GLuint)m_shapeQuad->getSize(), 6, 0, 0 };
			m_quad6IndirectBuffer = StaticTripleBuffer(sizeof(GLuint) * 4, quad6Data, GL_CLIENT_STORAGE_BIT);
		});
		m_cubemapSky->addCallback(m_aliveIndicator, [&](void) mutable {
			m_skyOutOfDate = true;
			m_skySize = m_cubemapSky->m_images[0]->m_size;
			glTextureStorage2D(m_cubemapMipped, 6, GL_RGB16F, m_skySize.x, m_skySize.x);
			for (int x = 0; x < 6; ++x) {
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_cubemapSky->m_pboIDs[x]);
				glTextureSubImage3D(m_cubemapMipped, 0, 0, 0, x, m_skySize.x, m_skySize.x, 1, GL_RGBA, GL_UNSIGNED_BYTE, (void*)0);
			}
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			glNamedFramebufferTexture(m_cubeFBO, GL_COLOR_ATTACHMENT0, m_cubemapMipped, 0);
			glNamedFramebufferDrawBuffer(m_cubeFBO, GL_COLOR_ATTACHMENT0);

			// Error Reporting
			if (glCheckNamedFramebufferStatus(m_cubeFBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				m_engine->getManager_Messages().error("Skybox Framebuffer has encountered an error.");
			if (!glIsTexture(m_cubemapMipped))
				m_engine->getManager_Messages().error("Skybox Texture is incomplete.");
		});
	}


	// Public Interface Implementations.
	inline virtual void prepareForNextFrame(const float & deltaTime) override {
		for (auto & camIndexBuffer : m_camIndexes)
			camIndexBuffer.endWriting();
		m_quadIndirectBuffer.endWriting();
		m_quad6IndirectBuffer.endWriting();
		m_drawIndex = 0;
	}
	inline virtual void renderTechnique(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const std::vector<std::pair<int, int>> & perspectives) override {
		if (!m_enabled || !m_shapeQuad->existsYet() || !m_shaderSky->existsYet() || !m_shaderSkyReflect->existsYet() || !m_shaderConvolute->existsYet() || !m_cubemapSky->existsYet())
			return;

		if (m_skyOutOfDate ) {
			convoluteSky(viewport);
			m_skyOutOfDate = false;
		}

		// Prepare camera index
		if (m_drawIndex >= m_camIndexes.size())
			m_camIndexes.resize(m_drawIndex + 1);
		auto &camBufferIndex = m_camIndexes[m_drawIndex];
		camBufferIndex.beginWriting();
		m_quadIndirectBuffer.beginWriting();
		m_quad6IndirectBuffer.beginWriting();
		std::vector<glm::ivec2> camIndices;
		for (auto &[camIndex, layer] : perspectives) 
			camIndices.push_back({ camIndex, layer });
		camBufferIndex.write(0, sizeof(glm::ivec2) * camIndices.size(), camIndices.data());
		GLuint instanceCount = perspectives.size();
		m_quadIndirectBuffer.write(sizeof(GLuint), sizeof(GLuint), &instanceCount);
		camBufferIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);

		glDisable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glEnable(GL_DEPTH_TEST);
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glBindTextureUnit(4, m_cubemapMipped);

		// Render skybox to reflection buffer
		// Uses the stencil buffer, last thing to write to it should be the reflector pass
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_EQUAL, 0, 0xFF);
		m_shaderSkyReflect->bind();
		viewport->m_gfxFBOS->bindForWriting("REFLECTION");
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		glDisable(GL_STENCIL_TEST);

		// Render skybox to lighting buffer
		m_shaderSky->bind();
		viewport->m_gfxFBOS->bindForWriting("LIGHTING");
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		glEnable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glDisable(GL_DEPTH_TEST);
		m_drawIndex++;
	}


private:
	// Private Methods
	/** Convolute the skybox cubemap, generating blurred mips (for rougher materials).
	@param	viewport	the viewport to render from. */
	inline void convoluteSky(const std::shared_ptr<Viewport> & viewport) {
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_shaderConvolute->bind();
		m_shaderConvolute->setUniform(0, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_cubeFBO);
		glBindTextureUnit(0, m_cubemapMipped);
		m_quad6IndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
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
		glTextureParameteri(m_cubemapMipped, GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(m_cubemapMipped, GL_TEXTURE_MAX_LEVEL, 5);
		glNamedFramebufferTexture(m_cubeFBO, GL_COLOR_ATTACHMENT0, m_cubemapMipped, 0);
		glViewport(0, 0, GLsizei(viewport->m_dimensions.x), GLsizei(viewport->m_dimensions.y));
		Shader::Release();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	GLuint m_cubeFBO = 0, m_cubemapMipped = 0;
	Shared_Cubemap m_cubemapSky;
	Shared_Shader m_shaderSky, m_shaderSkyReflect, m_shaderConvolute;
	Shared_Primitive m_shapeQuad;
	StaticTripleBuffer m_quadIndirectBuffer, m_quad6IndirectBuffer;
	bool m_skyOutOfDate = false;
	glm::ivec2 m_skySize = glm::ivec2(1);
	std::vector<DynamicBuffer> m_camIndexes;
	int	m_drawIndex = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // FRAMETIME_COUNTER_H