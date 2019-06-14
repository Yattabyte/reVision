#pragma once
#ifndef RADIANCE_HINTS_H
#define RADIANCE_HINTS_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Common/Graphics_Framebuffers.h"
#include "Modules/Graphics/Common/CameraBuffer.h"
#include "Modules/Graphics/Common/RH_Volume.h"
#include "Assets/Shader.h"
#include "Assets/Texture.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Engine.h"


/** A core-rendering technique for approximating indirect diffuse lighting (irradiant light, global illumination, etc) */
class Radiance_Hints : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline ~Radiance_Hints() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	inline Radiance_Hints(Engine * engine)
		: m_engine(engine) {
		// Asset Loading
		m_shaderRecon = Shared_Shader(m_engine, "Effects\\RH Reconstruction");
		m_shaderRebounce = Shared_Shader(m_engine, "Effects\\RH Rebounce");
		m_shapeQuad = Shared_Primitive(m_engine, "quad");

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_RH_BOUNCE_SIZE, m_bounceSize);
		preferences.addCallback(PreferenceState::C_RH_BOUNCE_SIZE, m_aliveIndicator, [&](const float &f) { 
			m_bounceSize = (GLuint)f; 
			if (m_shapeQuad && m_shapeQuad->existsYet()) {
				const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), m_bounceSize, 0, 0 };
				m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData, 0);
			}
		});

		// Asset-Finished callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			// count, primCount, first, reserved
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), m_bounceSize, 0, 0 };
			m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData, 0);
		});

	}


	// Public Interface Implementations.
	inline virtual void applyTechnique(const float & deltaTime) override {
		if (!m_enabled || !m_shapeQuad->existsYet() || !m_shaderRecon->existsYet() || !m_shaderRebounce->existsYet())
			return;
		
		// Bind common data
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		m_shaderRebounce->setUniform(1, m_volumeRH->m_max);
		m_shaderRebounce->setUniform(2, m_volumeRH->m_min);
		m_shaderRebounce->setUniform(4, m_volumeRH->m_resolution);
		m_shaderRebounce->setUniform(5, m_volumeRH->m_unitSize);
		m_shaderRecon->setUniform(1, m_volumeRH->m_max);
		m_shaderRecon->setUniform(2, m_volumeRH->m_min);
		m_shaderRecon->setUniform(3, m_volumeRH->m_resolution);
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);

		// Bounce light a second time
		m_shaderRebounce->bind();
		m_volumeRH->readPrimary(0);
		m_volumeRH->writeSecondary();
		glViewport(0, 0, (GLsizei)m_volumeRH->m_resolution, (GLsizei)m_volumeRH->m_resolution);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		glViewport(0, 0, (*m_cameraBuffer)->Dimensions.x, (*m_cameraBuffer)->Dimensions.y);

		// Reconstruct indirect radiance
		m_shaderRecon->bind();
		m_gfxFBOS->bindForReading("GEOMETRY", 0);
		m_volumeRH->readSecondary(4);
		m_gfxFBOS->bindForWriting("BOUNCE");
		glDrawArraysIndirect(GL_TRIANGLES, 0);		

		// Bind for reading by next effect	
		glBindTextureUnit(4, m_gfxFBOS->getTexID("BOUNCE", 0));
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Shader m_shaderRecon, m_shaderRebounce;
	Shared_Primitive m_shapeQuad;
	StaticBuffer m_quadIndirectBuffer;
	GLuint m_bounceSize = 16;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // RADIANCE_HINTS_H