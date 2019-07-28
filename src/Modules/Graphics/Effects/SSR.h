#pragma once
#ifndef SSR_H
#define SSR_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Assets/Texture.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/StaticTripleBuffer.h"
#include "Engine.h"


/** A core-rendering technique for deriving extra reflection information from the viewport itself. */
class SSR : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline ~SSR() {
		// Update indicator
		*m_aliveIndicator = false;

		// Destroy OpenGL objects
		glDeleteTextures(1, &m_bayerID);
	}
	/** Constructor. */
	inline SSR(Engine * engine)
		: m_engine(engine), Graphics_Technique(SECONDARY_LIGHTING) {
		// Asset Loading
		m_shaderSSR1 = Shared_Shader(m_engine, "Effects\\SSR part 1");
		m_shaderSSR2 = Shared_Shader(m_engine, "Effects\\SSR part 2");
		m_shaderCopy = Shared_Shader(m_engine, "Effects\\Copy Texture");
		m_shaderConvMips = Shared_Shader(m_engine, "Effects\\Gaussian Blur MIP");
		m_shapeQuad = Shared_Auto_Model(m_engine, "quad");

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_SSR, m_enabled);
		preferences.addCallback(PreferenceState::C_SSR, m_aliveIndicator, [&](const float &f) { m_enabled = (bool)f; });

		// Bayer matrix
		GLubyte data[16] = { 0,8,2,10,12,4,14,6,3,11,1,9,15,7,13,5 };
		glCreateTextures(GL_TEXTURE_2D, 1, &m_bayerID);
		glTextureStorage2D(m_bayerID, 1, GL_R16F, 4, 4);
		glTextureSubImage2D(m_bayerID, 0, 0, 0, 4, 4, GL_RED, GL_UNSIGNED_BYTE, &data);
		glTextureParameteri(m_bayerID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_bayerID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(m_bayerID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_bayerID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		// Error Reporting
		auto & msgMgr = m_engine->getManager_Messages();
		if (!glIsTexture(m_bayerID))
			msgMgr.error("SSR Bayer Matrix Texture is incomplete.");
	}


	// Public Interface Implementations.
	inline virtual void prepareForNextFrame(const float & deltaTime) override {
		for (auto &[camIndexBuffer, quadIndirectBuffer] : m_drawData) {
			camIndexBuffer.endWriting();
			quadIndirectBuffer.endWriting();
		}
		m_drawIndex = 0;
	}
	inline virtual void renderTechnique(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const std::vector<std::pair<int, int>> & perspectives) override {
		if (!m_enabled || !m_shapeQuad->existsYet() || !m_shaderCopy->existsYet() || !m_shaderConvMips->existsYet() || !m_shaderSSR1->existsYet() || !m_shaderSSR2->existsYet())
			return;

		// Prepare camera index
		if (m_drawIndex >= m_drawData.size())
			m_drawData.resize(m_drawIndex + 1);
		auto &[camBufferIndex, quadIndirectBuffer] = m_drawData[m_drawIndex];
		camBufferIndex.beginWriting();
		quadIndirectBuffer.beginWriting();
		std::vector<glm::ivec2> camIndices;
		for (auto &[camIndex, layer] : perspectives)
			camIndices.push_back({ camIndex, layer });
		camBufferIndex.write(0, sizeof(glm::ivec2) * camIndices.size(), camIndices.data());
		const auto instanceCount = (GLuint)perspectives.size();
		quadIndirectBuffer.write(sizeof(GLuint), sizeof(GLuint), &instanceCount);
		camBufferIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);

		glBindVertexArray(m_shapeQuad->m_vaoID);
		quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);

		updateMIPChain(viewport);

		glDisable(GL_BLEND);
		viewport->m_gfxFBOS->bindForWriting("SSR");
		viewport->m_gfxFBOS->bindForReading("GEOMETRY", 0);
		m_shaderSSR1->bind();
		glBindTextureUnit(6, m_bayerID);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		viewport->m_gfxFBOS->bindForWriting("REFLECTION");
		glBindTextureUnit(5, viewport->m_gfxFBOS->getTexID("SSR", 0));
		glBindTextureUnit(6, viewport->m_gfxFBOS->getTexID("SSR_MIP", 0));
		m_shaderSSR2->bind();
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		glBlendFunc(GL_ONE, GL_ONE);
		glDisable(GL_BLEND);
		m_drawIndex++;
	}


private:
	// Private Methods
	/** Convolute the lighting buffer into each of its mip levels.
	@param	viewport	the viewport to render from. */
	inline void updateMIPChain(const std::shared_ptr<Viewport> & viewport) {
		const auto mipFboID = viewport->m_gfxFBOS->getFboID("SSR_MIP");
		const auto mipTexID = viewport->m_gfxFBOS->getTexID("SSR_MIP", 0);
		const auto dimensions = glm::vec2(viewport->m_dimensions);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		// Copy lighting texture to one with a MIP chain
		m_shaderCopy->bind();
		viewport->m_gfxFBOS->bindForReading("LIGHTING", 0);
		viewport->m_gfxFBOS->bindForWriting("SSR_MIP");
		GLfloat clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		glClearNamedFramebufferfv(mipFboID, GL_COLOR, 0, clearColor);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Blur MIP chain, reading from 1 MIP level and writing into next
		m_shaderConvMips->bind();
		glBindTextureUnit(0, mipTexID);
		for (int horizontal = 0; horizontal < 2; ++horizontal) {
			m_shaderConvMips->setUniform(0, horizontal);
			glm::ivec2 read_size = viewport->m_dimensions;
			for (int x = 1; x < 6; ++x) {
				// Ensure we are reading from MIP level x - 1
				m_shaderConvMips->setUniform(1, read_size);
				glTextureParameteri(mipTexID, GL_TEXTURE_BASE_LEVEL, x - 1);
				glTextureParameteri(mipTexID, GL_TEXTURE_MAX_LEVEL, x - 1);
				// Ensure we are writing to MIP level x
				const glm::ivec2 write_size = glm::ivec2(floor(dimensions.x / pow(2, x)), floor(dimensions.y / pow(2, x)));
				glNamedFramebufferTexture(mipFboID, GL_COLOR_ATTACHMENT0, mipTexID, x);

				glViewport(0, 0, std::max(1, write_size.x), std::max(1, write_size.y));
				glDrawArraysIndirect(GL_TRIANGLES, 0);
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
		glViewport(0, 0, (GLsizei)dimensions.x, (GLsizei)dimensions.y);
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Shader m_shaderSSR1, m_shaderSSR2, m_shaderCopy, m_shaderConvMips;
	Shared_Auto_Model m_shapeQuad;
	GLuint m_bayerID = 0;
	struct DrawData {
		DynamicBuffer camBufferIndex;
		constexpr static GLuint quadData[4] = { (GLuint)6, 1, 0, 0 };
		StaticTripleBuffer quadIndirectBuffer = StaticTripleBuffer(sizeof(GLuint) * 4, quadData);
	};
	std::vector<DrawData> m_drawData;
	int	m_drawIndex = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // SSR_H