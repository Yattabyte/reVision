#pragma once
#ifndef PROPRENDERING_FX_H
#define PROPRENDERING_FX_H

#include "Modules/Graphics/Effects/GFX_Core_Effect.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Modules/Graphics/ECS/PropRendering_S.h"
#include "Utilities/GL/FBO.h"
#include "Engine.h"


/** A core-rendering technique which renders prop geometry to the scene. */
class PropRendering_Effect : public GFX_Core_Effect {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline ~PropRendering_Effect() = default;
	/** Constructor. */
	inline PropRendering_Effect(
		Engine * engine, FBO_Base * geometryFBO, Prop_RenderState * renderState, Shared_Shader & shaderCull, Shared_Shader & shaderGeometry
	) : m_engine(engine), m_geometryFBO(geometryFBO), m_renderState(renderState), m_shaderCull(shaderCull), m_shaderGeometry(shaderGeometry) {
		// Asset Loading
		m_shapeCube = Shared_Primitive(engine, "cube");
		m_modelsVAO = &m_engine->getManager_Models().getVAO();
	}


	// Public Interface Implementation	
	inline virtual void applyEffect(const float & deltaTime) override {
		// Exit Early
		if (!m_shapeCube->existsYet() || !m_shaderCull->existsYet() || !m_shaderGeometry->existsYet())
			return;

		m_engine->getManager_Materials().bind();
		m_propBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		m_renderState->m_bufferPropIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);
		m_skeletonBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 5);
		m_renderState->m_bufferSkeletonIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);

		// Draw bounding boxes for each model, filling render buffer on successful rasterization
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_FALSE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		m_shaderCull->bind();
		m_geometryFBO->bindForWriting();
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
		glCullFace(GL_BACK);
		m_shaderGeometry->bind();
		glBindVertexArray(*m_modelsVAO);
		m_renderState->m_bufferRender.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_renderState->m_propCount, 0);
	}


	// Public Attributes
	VectorBuffer<Prop_Component::GL_Buffer> m_propBuffer;
	VectorBuffer<Skeleton_Component::GL_Buffer> m_skeletonBuffer;


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	FBO_Base * m_geometryFBO = nullptr;
	Shared_Shader m_shaderCull, m_shaderGeometry;
	Shared_Primitive m_shapeCube;
	const GLuint * m_modelsVAO = nullptr;
	Prop_RenderState * m_renderState = nullptr;
};

#endif // PROPRENDERING_FX_H