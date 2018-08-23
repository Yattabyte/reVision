#pragma once
#ifndef PBR_REFLECTION_H
#define PBR_REFLECTION_H

#include "Modules\Graphics\Resources\Effects\Effect_Base.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Utilities\GL\FBO.h"
#include "Engine.h"


/** A post-processing technique for writing the final scene reflections back into the lighting. */
class PBR_Reflection : public Effect_Base {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~PBR_Reflection() {
		if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
	}
	/** Constructor. */
	PBR_Reflection(Engine * engine, FBO_Base * lightingFBO, FBO_Base * reflectionFBO) {
		// Default Parameters
		m_engine = engine;
		m_lightingFBO = lightingFBO;
		m_reflectionFBO = reflectionFBO;

		// Asset Loading
		m_shader = Asset_Shader::Create(m_engine, "Effects\\Copy Texture");
		m_shapeQuad = Asset_Primitive::Create(m_engine, "quad");

		// Primitive Construction
		m_quadVAOLoaded = false;
		m_quadVAO = Asset_Primitive::Generate_VAO();
		m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, 0);
		m_shapeQuad->addCallback(this, [&]() mutable {
			m_quadVAOLoaded = true;
			m_shapeQuad->updateVAO(m_quadVAO);
			const GLuint quadData[4] = { m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_quadIndirectBuffer.write(0, sizeof(GLuint) * 4, quadData);
		});
	}


	// Interface Implementations.
	virtual void applyEffect(const float & deltaTime) {
		if (!m_shader->existsYet() || !m_quadVAOLoaded)
			return;
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		m_lightingFBO->bindForWriting();
		m_reflectionFBO->bindForReading(0);
		m_shader->bind();
		glBindVertexArray(m_quadVAO);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);		
		glDisable(GL_BLEND);
	}


private:
	// Private Attributes
	Engine * m_engine;
	Shared_Asset_Shader m_shader;
	Shared_Asset_Primitive m_shapeQuad;
	GLuint m_quadVAO;
	bool m_quadVAOLoaded;
	StaticBuffer m_quadIndirectBuffer;
	FBO_Base * m_lightingFBO, * m_reflectionFBO;
};

#endif // PBR_REFLECTION_H