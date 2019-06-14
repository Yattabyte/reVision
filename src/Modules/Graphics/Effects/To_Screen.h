#pragma once
#ifndef TO_SCREEN_H
#define TO_SCREEN_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Engine.h"


/** The final post-processing technique which outputs the last bound texture to the screen. */
class To_Screen : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline ~To_Screen() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	inline To_Screen(Engine * engine)
		: m_engine(engine) {
		// Asset Loading
		m_shader = Shared_Shader(m_engine, "Effects\\Copy Texture");
		m_shapeQuad = Shared_Primitive(m_engine, "quad");

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData, 0);
		});
	}


	// Public Interface Implementations.
	inline virtual void applyTechnique(const float & deltaTime) override {
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
	Shared_Shader m_shader;
	Shared_Primitive m_shapeQuad;
	StaticBuffer m_quadIndirectBuffer;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // TO_SCREEN_H