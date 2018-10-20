#pragma once
#ifndef PROPSHADOWING_FX_H
#define PROPSHADOWING_FX_H

#include "Modules\Graphics\Effects\Effect_Base.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "ECS\Systems\PropShadowing_S.h"
#include "Utilities\GL\FBO.h"
#include "Engine.h"
#include "GLFW\glfw3.h"

/** A core rendering effect which renders prop geometry to the active shadow map. */
class PropShadowing_Effect : public Effect_Base {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~PropShadowing_Effect() = default;
	/** Constructor. */
	PropShadowing_Effect(
		Engine * engine, Shared_Asset_Shader & shaderCull, Shared_Asset_Shader & shaderShadow, GL_Vector * propBuffer, GL_Vector * skeletonBuffer, PropShadow_RenderState * renderState
	) : m_engine(engine), m_propBuffer(propBuffer), m_skeletonBuffer(skeletonBuffer), m_shaderCull(shaderCull), m_shaderShadow(shaderShadow), m_renderState(renderState) {
		// Asset Loading
		m_shapeCube = Asset_Primitive::Create(engine, "cube");
		m_modelsVAO = &m_engine->getModelManager().getVAO();
	}


	// Interface Implementation	
	inline virtual void applyEffect(const float & deltaTime) override {
		// Exit Early
		if (!m_shapeCube->existsYet())
			return;

		m_engine->getMaterialManager().bind();
		m_propBuffer->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		m_renderState->m_bufferPropIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);
		m_skeletonBuffer->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 5);
		m_renderState->m_bufferSkeletonIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);

		// Draw bounding boxes for each model, filling render buffer on successful rasterization
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_FALSE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		m_shaderCull->bind();
		glBindVertexArray(m_shapeCube->m_vaoID);
		m_renderState->m_bufferCulling.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		m_renderState->m_bufferRender.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 7);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_renderState->m_propCount, 0);
		glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, 0);

		// Draw geometry using the populated render buffer
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glFrontFace(GL_CW);
		m_shaderShadow->bind();
		glBindVertexArray(*m_modelsVAO);
		m_renderState->m_bufferRender.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_renderState->m_propCount, 0);

		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Asset_Shader	m_shaderCull, m_shaderShadow;
	Shared_Asset_Primitive m_shapeCube;
	const GLuint * m_modelsVAO = nullptr;
	GL_Vector * m_propBuffer = nullptr, * m_skeletonBuffer = nullptr;
	PropShadow_RenderState * m_renderState = nullptr;
};

#endif // PROPSHADOWING_FX_H