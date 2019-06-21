#pragma once
#ifndef JOIN_REFLECTIONS_H
#define JOIN_REFLECTIONS_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Assets/Shader.h"
#include "Assets/Texture.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/FBO.h"
#include "Engine.h"


/** A core-rendering technique for writing the final scene reflections back into the lighting. */
class Join_Reflections : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline ~Join_Reflections() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	inline Join_Reflections(Engine * engine)
		: m_engine(engine), Graphics_Technique(SECONDARY_LIGHTING) {
		// Asset Loading
		m_shader = Shared_Shader(m_engine, "Effects\\Join Reflections");
		m_brdfMap = Shared_Texture(engine, "brdfLUT.png", GL_TEXTURE_2D, false, false);
		m_shapeQuad = Shared_Primitive(m_engine, "quad");

		// Asset-Finished callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData, 0);
		});
	}


	// Public Interface Implementations.
	inline virtual void renderTechnique(const float & deltaTime) override {
		if (!m_enabled || !m_shapeQuad->existsYet() || !m_shader->existsYet())
			return;
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		m_gfxFBOS->bindForWriting("LIGHTING");
		m_gfxFBOS->bindForReading("GEOMETRY", 0);
		m_gfxFBOS->bindForReading("REFLECTION", 5);
		m_brdfMap->bind(6);
		m_shader->bind();
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);		
		glDisable(GL_BLEND);
		glBindTextureUnit(0, m_gfxFBOS->getTexID("LIGHTING", 0));
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Shader m_shader;
	Shared_Texture m_brdfMap;
	Shared_Primitive m_shapeQuad;
	StaticBuffer m_quadIndirectBuffer;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // JOIN_REFLECTIONS_H