#pragma once
#ifndef BLOOM_H
#define BLOOM_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Engine.h"


class Lighting_FBO;

/** A post-processing technique for generating bloom from a lighting buffer. */
class Bloom : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline ~Bloom() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	inline Bloom(Engine * engine)
		: m_engine(engine) {
		// Asset Loading
		m_shaderBloomExtract = Shared_Shader(m_engine, "Effects\\Bloom Extraction");
		m_shaderCopy = Shared_Shader(m_engine, "Effects\\Copy Texture");
		m_shaderGB = Shared_Shader(m_engine, "Effects\\Gaussian Blur");
		m_shapeQuad = Shared_Primitive(engine, "quad");

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData, 0);
		});

		// Preference Callbacks
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_BLOOM, m_enabled);
		preferences.addCallback(PreferenceState::C_BLOOM, m_aliveIndicator, [&](const float &f) { m_enabled = (bool)f; });
		preferences.getOrSetValue(PreferenceState::C_BLOOM_STRENGTH, m_bloomStrength);
		preferences.addCallback(PreferenceState::C_BLOOM_STRENGTH, m_aliveIndicator, [&](const float &f) { setBloomStrength((int)f); });
	}


	// Public Interface Implementations.
	inline virtual void applyTechnique(const float & deltaTime) override {
		if (!m_enabled || !m_shapeQuad->existsYet() || !m_shaderBloomExtract->existsYet() || !m_shaderCopy->existsYet() || !m_shaderGB->existsYet())
			return;
		// Extract bright regions from lighting buffer
		m_shaderBloomExtract->bind();
		glBindTextureUnit(0, m_gfxFBOS->getTexID("LIGHTING", 0));
		m_gfxFBOS->bindForWriting("BLOOM");
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		size_t bloomSpot = 0;

		if (m_bloomStrength > 0) {
			// Read from desired texture, blur into this frame buffer
			bool horizontal = false;
			glBindTextureUnit(0, m_gfxFBOS->getTexID("BLOOM", 0));
			glBindTextureUnit(1, m_gfxFBOS->getTexID("BLOOM", 1));
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
			m_shaderGB->bind();
			m_shaderGB->setUniform(0, horizontal);
			m_shaderGB->setUniform(1, (*m_cameraBuffer)->Dimensions);
			glBindVertexArray(m_shapeQuad->m_vaoID);
			m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);

			// Blur remainder of the times
			for (int i = 0; i < m_bloomStrength; i++) {
				horizontal = !horizontal;
				glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
				m_shaderGB->setUniform(0, horizontal);
				glDrawArraysIndirect(GL_TRIANGLES, 0);
			}
			bloomSpot = horizontal ? 1ull : 0ull;
		}

		// Copy to lighting buffer
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		m_gfxFBOS->bindForWriting("LIGHTING");
		glBindTextureUnit(0, m_gfxFBOS->getTexID("BLOOM", bloomSpot));
		m_shaderCopy->bind();
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		glDisable(GL_BLEND);
		glBindTextureUnit(0, m_gfxFBOS->getTexID("LIGHTING", 0));
	}


private:
	// Private Methods
	/** Change the strength of the bloom effect.
	@param	strength		the new strength of the bloom effect */
	inline void setBloomStrength(const int &strength) {
		m_bloomStrength = strength;
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Shader m_shaderBloomExtract, m_shaderCopy, m_shaderGB;
	Shared_Primitive m_shapeQuad;
	StaticBuffer m_quadIndirectBuffer;
	int m_bloomStrength = 5;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // BLOOM_H