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
	}
	/** Constructor. */
	To_Screen(Engine * engine)
	: m_engine(engine) {
		// Asset Loading
		m_shader = Asset_Shader::Create(m_engine, "Effects\\Copy Texture");
		m_shapeQuad = Asset_Primitive::Create(m_engine, "quad");

		// Asset-Finished Callbacks
		m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4);
		m_shapeQuad->addCallback(this, [&]() mutable {
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_quadIndirectBuffer.write(0, sizeof(GLuint) * 4, quadData);
		});
	}


	// Interface Implementations.
	virtual void applyEffect(const float & deltaTime) override {
		if (!m_shapeQuad->existsYet() || !m_shader->existsYet())
			return;
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		m_shader->bind();
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Asset_Shader m_shader;
	Shared_Asset_Primitive m_shapeQuad;
	StaticBuffer m_quadIndirectBuffer;
};

#endif // TO_SCREEN_H