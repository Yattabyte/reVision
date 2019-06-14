#pragma once
#ifndef HDR_H
#define HDR_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/FBO.h"
#include "Engine.h"


/** A post-processing technique for tone-mapping and gamma correcting the final lighting product. */
class HDR : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline ~HDR() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	inline HDR(Engine * engine)
		: m_engine(engine) {
		// Asset Loading
		m_shaderHDR = Shared_Shader(m_engine, "Effects\\HDR");
		m_shapeQuad = Shared_Primitive(engine, "quad");

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_GAMMA, m_gamma);
		preferences.addCallback(PreferenceState::C_GAMMA, m_aliveIndicator, [&](const float &f) { m_gamma = f; });

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData, 0);
		});
	}


	// Public Interface Implementations.
	inline virtual void applyTechnique(const float & deltaTime) override {
		if (!m_shapeQuad->existsYet() || !m_shaderHDR->existsYet())
			return;

		// Write HDR effect to own framebuffer
		m_gfxFBOS->bindForWriting("HDR");
		m_shaderHDR->bind();
		m_shaderHDR->setUniform(0, 1.0f);
		m_shaderHDR->setUniform(1, m_gamma);
		// Use the currently bound framebuffer from the prior effect
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);	
		
		// Bind for reading by next effect	
		glBindTextureUnit(0, m_gfxFBOS->getTexID("HDR", 0));
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	float m_gamma = 1.0f;
	Shared_Shader m_shaderHDR;
	Shared_Primitive m_shapeQuad;
	StaticBuffer m_quadIndirectBuffer;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // HDR_H