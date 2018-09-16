#pragma once
#ifndef JOIN_REFLECTIONS_H
#define JOIN_REFLECTIONS_H

#include "Modules\Graphics\Effects\Effect_Base.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Texture.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Utilities\GL\FBO.h"
#include "Engine.h"


/** A post-processing technique for writing the final scene reflections back into the lighting. */
class Join_Reflections : public Effect_Base {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~Join_Reflections() {
		if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
		m_brdfMap->removeCallback(this);
		m_shader->removeCallback(this);
	}
	/** Constructor. */
	Join_Reflections(Engine * engine, FBO_Base * geometryFBO, FBO_Base * lightingFBO, FBO_Base * reflectionFBO) 
	: m_engine(engine), m_geometryFBO(geometryFBO), m_lightingFBO(lightingFBO), m_reflectionFBO(reflectionFBO) {
		// Asset Loading
		m_shader = Asset_Shader::Create(m_engine, "Effects\\Join Reflections");
		m_brdfMap = Asset_Texture::Create(engine, "brdfLUT.png", GL_TEXTURE_2D, false, false);
		m_shapeQuad = Asset_Primitive::Create(m_engine, "quad");

		// Asset-Finished callbacks
		m_shapeQuad->addCallback(this, [&]() mutable {
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData, 0);
		});
		m_brdfMap->addCallback(this, [&] {
			glMakeTextureHandleResidentARB(m_brdfMap->m_glTexHandle);
			if (m_shader->existsYet())
				m_shader->setUniform(0, m_brdfMap->m_glTexHandle);
		});
		m_shader->addCallback(this, [&] {
			if (m_brdfMap->existsYet())
				m_shader->setUniform(0, m_brdfMap->m_glTexHandle);
		});
	}


	// Interface Implementations.
	virtual void applyEffect(const float & deltaTime) override {
		if (!m_shapeQuad->existsYet() || !m_shader->existsYet())
			return;
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		m_lightingFBO->bindForWriting();
		m_geometryFBO->bindForReading(0);
		m_reflectionFBO->bindForReading(5);
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
	Shared_Asset_Shader m_shader;
	Shared_Asset_Texture m_brdfMap;
	Shared_Asset_Primitive m_shapeQuad;
	StaticBuffer m_quadIndirectBuffer;
};

#endif // JOIN_REFLECTIONS_H