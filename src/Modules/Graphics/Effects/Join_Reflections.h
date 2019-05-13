#pragma once
#ifndef JOIN_REFLECTIONS_H
#define JOIN_REFLECTIONS_H

#include "Modules/Graphics/Effects/GFX_Core_Effect.h"
#include "Assets/Shader.h"
#include "Assets/Texture.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/FBO.h"
#include "Engine.h"


/** A core-rendering technique for writing the final scene reflections back into the lighting. */
class Join_Reflections : public GFX_Core_Effect {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~Join_Reflections() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	Join_Reflections(Engine * engine, FBO_Base * geometryFBO, FBO_Base * lightingFBO, FBO_Base * reflectionFBO) 
	: m_engine(engine), m_geometryFBO(geometryFBO), m_lightingFBO(lightingFBO), m_reflectionFBO(reflectionFBO) {
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


	// Interface Implementations.
	inline virtual void applyEffect(const float & deltaTime) override {
		if (!m_shapeQuad->existsYet() || !m_shader->existsYet())
			return;
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		m_lightingFBO->bindForWriting();
		m_geometryFBO->bindForReading(0);
		m_reflectionFBO->bindForReading(5);
		m_brdfMap->bind(6);
		m_shader->bind();
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);		
		glDisable(GL_BLEND);
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	FBO_Base * m_geometryFBO = nullptr, * m_lightingFBO = nullptr, * m_reflectionFBO = nullptr;
	Shared_Shader m_shader;
	Shared_Texture m_brdfMap;
	Shared_Primitive m_shapeQuad;
	StaticBuffer m_quadIndirectBuffer;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // JOIN_REFLECTIONS_H