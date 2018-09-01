#pragma once
#ifndef SKYBOX_H
#define SKYBOX_H

#include "Modules\Graphics\Resources\Effects\Effect_Base.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Cubemap.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\FBO.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Engine.h"


/** A post-processing technique for writing the frame time to the screen. */
class Skybox : public Effect_Base {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~Skybox() {
		m_shapeQuad->removeCallback(this);
	}
	/** Constructor. */
	Skybox(Engine * engine, FBO_Base * geometryFBO, FBO_Base * lightingFBO, FBO_Base * reflectionFBO
	) : m_geometryFBO(geometryFBO), m_lightingFBO(lightingFBO), m_reflectionFBO(reflectionFBO) {
		// Asset Loading
		m_cubemapSky = Asset_Cubemap::Create(engine, "sky\\");
		m_shaderSky = Asset_Shader::Create(engine, "Effects\\Skybox");
		m_shaderSkyReflect = Asset_Shader::Create(engine, "Effects\\Skybox Reflection");
		m_shapeQuad = Asset_Primitive::Create(engine, "quad");

		// Asset-Finished Callbacks
		m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4);
		m_shapeQuad->addCallback(this, [&]() mutable {
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_quadIndirectBuffer.write(0, sizeof(GLuint) * 4, quadData);
		});
	}


	// Interface Implementations.
	virtual void applyEffect(const float & deltaTime) override {
		if (!m_shapeQuad->existsYet() || !m_shaderSky->existsYet() || !m_shaderSkyReflect->existsYet())
			return;
		glDisable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glEnable(GL_DEPTH_TEST);
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		m_geometryFBO->bindForReading();
		m_cubemapSky->bind(4);

		// Render skybox to reflection buffer
		// Uses the stencil buffer, last thing to write to it should be the reflector pass
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_EQUAL, 0, 0xFF);
		m_shaderSkyReflect->bind();
		m_reflectionFBO->bindForWriting();
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		glDisable(GL_STENCIL_TEST);

		// Render skybox to lighting buffer
		m_shaderSky->bind();
		m_lightingFBO->bindForWriting();
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		glEnable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glDisable(GL_DEPTH_TEST);
	}


private:
	// Private Attributes
	FBO_Base * m_geometryFBO, *m_lightingFBO, *m_reflectionFBO;
	Shared_Asset_Shader m_shaderSky, m_shaderSkyReflect;
	Shared_Asset_Cubemap m_cubemapSky;
	Shared_Asset_Primitive m_shapeQuad;
	StaticBuffer m_quadIndirectBuffer;
};

#endif // FRAMETIME_COUNTER_H