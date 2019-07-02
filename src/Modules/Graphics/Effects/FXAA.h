#pragma once
#ifndef FXAA_H
#define FXAA_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Engine.h"


/** A post-processing technique for applying fxaa to the currently bound 2D image. */
class FXAA : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline ~FXAA() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	inline FXAA(Engine * engine) 
		: m_engine(engine), Graphics_Technique(POST_PROCESSING) {
		// Asset Loading
		m_shaderFXAA = Shared_Shader(m_engine, "Effects\\FXAA");
		m_shapeQuad = Shared_Primitive(m_engine, "quad");

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData, 0);
		});

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_FXAA, m_enabled);
		preferences.addCallback(PreferenceState::C_FXAA, m_aliveIndicator, [&](const float &f) { m_enabled = (bool)f; });
	}


	// Public Interface Implementations.
	inline virtual void renderTechnique(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const std::shared_ptr<CameraBuffer> & camera) override {
		if (!m_enabled || !m_shapeQuad->existsYet() || !m_shaderFXAA->existsYet())
			return;

		// Apply FXAA effect
		viewport->m_gfxFBOS->bindForWriting("FXAA");
		m_shaderFXAA->bind();
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Bind for reading by next effect	
		glBindTextureUnit(0, viewport->m_gfxFBOS->getTexID("FXAA", 0));
	}
	

private:
	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Shader m_shaderFXAA;
	Shared_Primitive m_shapeQuad;
	StaticBuffer m_quadIndirectBuffer;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // FXAA_H