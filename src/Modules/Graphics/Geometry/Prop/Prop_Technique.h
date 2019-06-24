#pragma once
#ifndef PROP_TECHNIQUE_H
#define PROP_TECHNIQUE_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Geometry/components.h"
#include "Modules/Graphics/Geometry/Prop/PropVisibility_System.h"
#include "Modules/Graphics/Geometry/Prop/PropSync_System.h"
#include "Modules/World/ECS/components.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Engine.h"
#include <vector>


/** A core rendering technique for rendering props from a given viewing perspective. */
class Prop_Technique : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Destructor. */
	inline ~Prop_Technique() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	inline Prop_Technique(Engine * engine, ECSSystemList & auxilliarySystems)
		: m_engine(engine), Graphics_Technique(GEOMETRY) {
		// Auxilliary Systems
		m_visibility = std::make_shared<Prop_Visibility>();
		m_buffers = std::make_shared<Prop_Buffers>();
		auxilliarySystems.addSystem(new PropVisibility_System(m_visibility));
		auxilliarySystems.addSystem(new PropSync_System(m_buffers));

		// Asset Loading
		m_shaderCull = Shared_Shader(m_engine, "Core\\Props\\culling");
		m_shaderGeometry = Shared_Shader(m_engine, "Core\\Props\\geometry");
		m_shapeCube = Shared_Primitive(m_engine, "cube");
		m_modelsVAO = &m_engine->getManager_Models().getVAO();

		// Add New Component Types
		auto & world = m_engine->getModule_World();
		world.addNotifyOnComponentType(Prop_Component::ID, m_aliveIndicator, [&](BaseECSComponent * c) {
			((Prop_Component*)c)->m_modelBufferIndex = m_buffers->m_modelBuffer.newElement();
		});
		world.addNotifyOnComponentType(Skeleton_Component::ID, m_aliveIndicator, [&](BaseECSComponent * c) {
			((Skeleton_Component*)c)->m_skeleBufferIndex = m_buffers->m_skeletonBuffer.newElement();
		});

		// World-Changed Callback
		world.addLevelListener(m_aliveIndicator, [&](const World_Module::WorldState & state) {
			if (state == World_Module::unloaded)
				clear();
		});
	}


	// Public Interface Implementations
	inline virtual void beginFrame(const float & deltaTime) override {
		m_buffers->m_modelBuffer.beginWriting();
		m_buffers->m_skeletonBuffer.beginWriting();
		m_visibility->bufferPropIndex.beginWriting();
		m_visibility->bufferCulling.beginWriting();
		m_visibility->bufferRender.beginWriting();
		m_visibility->bufferSkeletonIndex.beginWriting();
	}
	inline virtual void endFrame(const float & deltaTime) override {
		m_buffers->m_modelBuffer.endWriting();
		m_buffers->m_skeletonBuffer.endWriting();
		m_visibility->bufferPropIndex.endWriting();
		m_visibility->bufferCulling.endWriting();
		m_visibility->bufferRender.endWriting();
		m_visibility->bufferSkeletonIndex.endWriting();
	}
	inline virtual void renderTechnique(const float & deltaTime, const std::shared_ptr<Viewport> & viewport) override {
		// Exit Early
		if (!m_enabled || !m_shapeCube->existsYet() || !m_shaderCull->existsYet() || !m_shaderGeometry->existsYet())
			return;

		// Apply occlusion culling and render props
		renderGeometry(deltaTime, viewport);
	}	


	// Public Methods
	/** Retrieve the prop buffer.
	@return		the prop buffer. */
	inline auto & getPropBuffer() {
		return m_buffers->m_modelBuffer;
	}
	/** Retrieve the skeleton buffer.
	@return		the skeleton buffer. */
	inline auto & getSkeletonBuffer() {
		return m_buffers->m_skeletonBuffer;
	}


private:
	// Private Methods
	/***/
	inline void renderGeometry(const float & deltaTime, const std::shared_ptr<Viewport> & viewport) {
		if (m_visibility->visProps) {
			m_engine->getManager_Materials().bind();
			m_buffers->m_modelBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
			m_visibility->bufferPropIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);
			m_buffers->m_skeletonBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 5);
			m_visibility->bufferSkeletonIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);

			// Draw bounding boxes for each model, filling render buffer on successful rasterization
			glDisable(GL_BLEND);
			glEnable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			glDepthFunc(GL_LEQUAL);
			glDepthMask(GL_FALSE);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			m_shaderCull->bind();
			viewport->m_gfxFBOS->bindForWriting("GEOMETRY");
			glBindVertexArray(m_shapeCube->m_vaoID);
			m_visibility->bufferCulling.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			m_visibility->bufferRender.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 7);
			glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_visibility->visProps, 0);
			glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, 0);

			// Draw geometry using the populated render buffer
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glDepthMask(GL_TRUE);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			m_shaderGeometry->bind();
			glBindVertexArray(*m_modelsVAO);
			m_visibility->bufferRender.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_visibility->visProps, 0);
		}
	}
	/** Clear out the props queued up for rendering. */
	inline void clear() {
		m_visibility->visProps = 0;
		m_buffers->m_modelBuffer.clear();
		m_buffers->m_skeletonBuffer.clear();
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	const GLuint * m_modelsVAO = nullptr;
	Shared_Shader m_shaderCull, m_shaderGeometry;
	Shared_Primitive m_shapeCube;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);

	// Shared Attributes
	std::shared_ptr<Prop_Visibility> m_visibility;
	std::shared_ptr<Prop_Buffers> m_buffers;
};

#endif // PROP_TECHNIQUE_H