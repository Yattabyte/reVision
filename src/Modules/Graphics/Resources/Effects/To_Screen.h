#pragma once
#ifndef TO_SCREEN_H
#define TO_SCREEN_H

#include "Modules\Graphics\Resources\Effects\Effect_Base.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Engine.h"


/** The final post-processing technique which outputs the last bound texture to the screen. */
class To_Screen : public Effect_Base {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~To_Screen() {
		if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
		glDeleteVertexArrays(1, &m_quadVAO);
	}
	/** Constructor. */
	To_Screen(Engine * engine) {
		// Default Parameters
		m_engine = engine;

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
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_quadIndirectBuffer.write(0, sizeof(GLuint) * 4, quadData);
		});
	}


	// Interface Implementations.
	virtual void applyEffect(const float & deltaTime) {
		if (!m_shader->existsYet() || !m_quadVAOLoaded)
			return;
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		m_shader->bind();
		glBindVertexArray(m_quadVAO);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
	}


private:
	// Private Attributes
	Engine * m_engine;
	Shared_Asset_Shader m_shader;
	Shared_Asset_Primitive m_shapeQuad;
	GLuint m_quadVAO;
	bool m_quadVAOLoaded;
	StaticBuffer m_quadIndirectBuffer;
};

#endif // TO_SCREEN_H