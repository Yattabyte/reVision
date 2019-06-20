#pragma once
#ifndef SSR_H
#define SSR_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Assets/Texture.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/FBO.h"
#include "Engine.h"


/** A core-rendering technique for deriving extra reflection information from the viewport itself. */
class SSR : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline ~SSR() {
		// Update indicator
		m_aliveIndicator = false;

		// Destroy OpenGL objects
		glDeleteTextures(1, &m_bayerID);
	}
	/** Constructor. */
	inline SSR(Engine * engine)
		: m_engine(engine) {
		// Asset Loading
		m_shaderSSR1 = Shared_Shader(m_engine, "Effects\\SSR part 1");
		m_shaderSSR2 = Shared_Shader(m_engine, "Effects\\SSR part 2");
		m_shaderCopy = Shared_Shader(m_engine, "Effects\\Copy Texture");
		m_shaderConvMips = Shared_Shader(m_engine, "Effects\\Gaussian Blur MIP");
		m_shapeQuad = Shared_Primitive(m_engine, "quad");

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_SSR, m_enabled);
		preferences.addCallback(PreferenceState::C_SSR, m_aliveIndicator, [&](const float &f) { m_enabled = (bool)f; });

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData, 0);
		});

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
	inline virtual void renderTechnique(const float & deltaTime) override {
		if (!m_enabled || !m_shapeQuad->existsYet() || !m_shaderCopy->existsYet() || !m_shaderConvMips->existsYet() || !m_shaderSSR1->existsYet() || !m_shaderSSR2->existsYet())
			return;
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);

		updateMIPChain();

		glDisable(GL_BLEND);
		m_gfxFBOS->bindForWriting("SSR");
		m_gfxFBOS->bindForReading("GEOMETRY", 0);
		m_shaderSSR1->bind();
		glBindTextureUnit(6, m_bayerID);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		m_gfxFBOS->bindForWriting("REFLECTION");
		glBindTextureUnit(5, m_gfxFBOS->getTexID("SSR", 0));
		glBindTextureUnit(6, m_gfxFBOS->getTexID("SSR_MIP", 0));
		m_shaderSSR2->bind();
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		glBlendFunc(GL_ONE, GL_ONE);
		glDisable(GL_BLEND);
	}


private:
	// Private Methods
	/** Convolute the lighting buffer into each of its mip levels. */
	inline void updateMIPChain() {
		const auto mipFboID = m_gfxFBOS->getFboID("SSR_MIP");
		const auto mipTexID = m_gfxFBOS->getTexID("SSR_MIP", 0);
		const auto dimensions = (*m_cameraBuffer)->Dimensions;

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		// Copy lighting texture to one with a MIP chain
		m_shaderCopy->bind();
		m_gfxFBOS->bindForReading("LIGHTING", 0);
		m_gfxFBOS->bindForWriting("SSR_MIP");
		GLfloat clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		glClearNamedFramebufferfv(mipFboID, GL_COLOR, 0, clearColor);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Blur MIP chain, reading from 1 MIP level and writing into next
		m_shaderConvMips->bind();
		glBindTextureUnit(0, mipTexID);
		for (int horizontal = 0; horizontal < 2; ++horizontal) {
			m_shaderConvMips->setUniform(0, horizontal);
			auto read_size = glm::ivec2((*m_cameraBuffer)->Dimensions);
			for (int x = 1; x < 6; ++x) {
				// Ensure we are reading from MIP level x - 1
				m_shaderConvMips->setUniform(1, read_size);
				glTextureParameteri(mipTexID, GL_TEXTURE_BASE_LEVEL, x - 1);
				glTextureParameteri(mipTexID, GL_TEXTURE_MAX_LEVEL, x - 1);
				// Ensure we are writing to MIP level x
				const auto write_size = glm::ivec2(floor(dimensions.x / pow(2, x)), floor(dimensions.y / pow(2, x)));
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
	Shared_Primitive m_shapeQuad;
	GLuint m_bayerID = 0;
	StaticBuffer m_quadIndirectBuffer;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // SSR_H