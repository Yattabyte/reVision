#pragma once
#ifndef SKYBOX_S_H
#define SKYBOX_S_H

#include "ECS\Systems\ecsSystem.h"
#include "glm\gtx\component_wise.hpp"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Engine.h"
#include <vector>

/* Component Types Used */
#include "ECS\Components\Skybox_C.h"


/** A system responsible for rendering skybox components. */
class Skybox_System : public BaseECSSystem {
public: 
	// (de)Constructors
	~Skybox_System() {
		if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
	}
	Skybox_System(
		Engine * engine, FBO_Base * geometryFBO, FBO_Base * lightingFBO, FBO_Base * reflectionFBO
	) : BaseECSSystem(), m_geometryFBO(geometryFBO), m_lightingFBO(lightingFBO), m_reflectionFBO(reflectionFBO) {
		// Declare component types used
		addComponentType(Skybox_Component::ID);

		// System Loading
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


	// Interface Implementation
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		if (!m_shapeQuad->existsYet() || !m_shaderSky->existsYet())
			return;
		if (!components.size())
			return;
		Skybox_Component * skyboxComponent = (Skybox_Component*)components[0][0];
		if (!skyboxComponent->m_texture->existsYet())
			return;

		glDisable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glEnable(GL_DEPTH_TEST);
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		m_geometryFBO->bindForReading();
		skyboxComponent->m_texture->bind(4);

		// Render skybox to reflection buffer
		m_shaderSkyReflect->bind();
		m_reflectionFBO->bindForWriting();
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Render skybox to lighting buffer
		m_shaderSky->bind();
		m_lightingFBO->bindForWriting();
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		glEnable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glDisable(GL_DEPTH_TEST);
	};
	

private:
	// Private Attributes
	FBO_Base * m_geometryFBO, * m_lightingFBO, * m_reflectionFBO;
	Shared_Asset_Shader	m_shaderSky, m_shaderSkyReflect;
	Shared_Asset_Primitive m_shapeQuad;
	StaticBuffer m_quadIndirectBuffer;
};

#endif // SKYBOX_S_H