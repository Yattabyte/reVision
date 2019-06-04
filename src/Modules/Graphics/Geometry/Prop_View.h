#pragma once
#ifndef PROP_VIEW_H
#define PROP_VIEW_H

#include "Modules/Graphics/Graphics_Technique.h"
#include "Modules/Graphics/Geometry/components.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/FBO.h"
#include "Engine.h"
#include <vector>


/***/
class Prop_View : public Graphics_Technique, public BaseECSSystem {
public:
	// Public (de)Constructors
	/** Destructor. */
	inline ~Prop_View() {
		auto & world = m_engine->getModule_World();
		world.removeComponentType("Prop_Component");
		world.removeComponentType("Skeleton_Component");
	}
	/** Constructor. */
	inline Prop_View(Engine * engine, FBO_Base * geometryFBO)
		: m_engine(engine), m_geometryFBO(geometryFBO) {
		// Asset Loading
		m_shaderCull = Shared_Shader(m_engine, "Core\\Props\\culling");
		m_shaderGeometry = Shared_Shader(m_engine, "Core\\Props\\geometry");
		m_shapeCube = Shared_Primitive(engine, "cube");
		m_modelsVAO = &m_engine->getManager_Models().getVAO();

		// Declare component types used
		addComponentType(Prop_Component::ID);
		addComponentType(Skeleton_Component::ID, FLAG_OPTIONAL);

		// Error Reporting
		if (!isValid())
			engine->getManager_Messages().error("Invalid ECS System: Prop_View");

		// Add New Component Types
		auto & world = m_engine->getModule_World();
		world.addComponentType("Prop_Component", [&, engine](const ParamList & parameters) {
			auto directory = CastAny(parameters[0], std::string(""));
			auto material = CastAny(parameters[1], 0u);
			auto * component = new Prop_Component();
			component->m_data = m_propBuffer.newElement();
			component->m_model = Shared_Model(engine, directory);
			component->m_data->data->materialID = material;
			return std::make_pair(component->ID, component);
		});
		world.addComponentType("Skeleton_Component", [&, engine](const ParamList & parameters) {
			auto directory = CastAny(parameters[0], std::string(""));
			auto animation = CastAny(parameters[1], 0);
			auto * component = new Skeleton_Component();
			component->m_data = m_skeletonBuffer.newElement();
			component->m_mesh = Shared_Mesh(engine, "\\Models\\" + directory);
			component->m_animation = animation;
			for (int x = 0; x < NUM_MAX_BONES; ++x)
				component->m_data->data->bones[x] = glm::mat4(1.0f);
			return std::make_pair(component->ID, component);
		});
	}


	// Public Interface Implementations
	inline virtual void applyEffect(const float & deltaTime) override {
		// Exit Early
		if (!m_enabled || !m_shapeCube->existsYet() || !m_shaderCull->existsYet() || !m_shaderGeometry->existsYet())
			return;

		m_engine->getManager_Materials().bind();
		m_propBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		m_bufferPropIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);
		m_skeletonBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 5);
		m_bufferSkeletonIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);

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
		m_bufferCulling.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		m_bufferRender.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 7);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_propCount, 0);
		glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, 0);

		// Draw geometry using the populated render buffer
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		m_shaderGeometry->bind();
		glBindVertexArray(*m_modelsVAO);
		m_bufferRender.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_propCount, 0);
	}
	inline virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		// Accumulate draw parameter information per model
		std::vector<glm::ivec4> cullingDrawData, renderingDrawData;
		std::vector<GLuint> visibleIndices;
		std::vector<int> skeletonData;
		const glm::vec3 & eyePosition = m_engine->getModule_Graphics().getCameraBuffer()->EyePosition;
		for each (const auto & componentParam in components) {
			Prop_Component * propComponent = (Prop_Component*)componentParam[0];
			Skeleton_Component * skeletonComponent = (Skeleton_Component*)componentParam[1];
			if (!propComponent->m_model->existsYet())
				continue;

			const auto & offset = propComponent->m_model->m_offset;
			const auto & count = propComponent->m_model->m_count;
			const GLuint & index = propComponent->m_data->index;
			visibleIndices.push_back(index);
			// Flag for occlusion culling  if mesh complexity is high enough and if viewer is NOT within BSphere
			if ((count >= 100) && !(propComponent->m_radius > glm::distance(propComponent->m_position, eyePosition))) { // Allow
				cullingDrawData.push_back(glm::ivec4(36, 1, 0, 1));
				renderingDrawData.push_back(glm::ivec4(count, 0, offset, 1));
			}
			else { // Skip occlusion culling		
				cullingDrawData.push_back(glm::ivec4(36, 0, 0, 1));
				renderingDrawData.push_back(glm::ivec4(count, 1, offset, 1));
			}
			skeletonData.push_back(skeletonComponent ? skeletonComponent->m_data->index : -1); // get skeleton ID if this entity has one
		}

		// Update camera buffers
		const GLsizei size = (GLsizei)visibleIndices.size();
		m_propCount = size;
		m_bufferPropIndex.write(0, sizeof(GLuint) * size, visibleIndices.data());
		m_bufferCulling.write(0, sizeof(glm::ivec4) * size, cullingDrawData.data());
		m_bufferRender.write(0, sizeof(glm::ivec4) * size, renderingDrawData.data());
		m_bufferSkeletonIndex.write(0, sizeof(int) * size, skeletonData.data());
	}


	// Public Methods
	/***/
	inline auto & getPropBuffer() {
		return m_propBuffer;
	}
	/***/
	inline auto & getSkeletonBuffer() {
		return m_skeletonBuffer;
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	FBO_Base * m_geometryFBO = nullptr;
	const GLuint * m_modelsVAO = nullptr;
	Shared_Shader m_shaderCull, m_shaderGeometry;
	Shared_Primitive m_shapeCube;
	GLsizei m_propCount = 0;
	DynamicBuffer m_bufferPropIndex, m_bufferCulling, m_bufferRender, m_bufferSkeletonIndex;
	VectorBuffer<Prop_Component::GL_Buffer> m_propBuffer;
	VectorBuffer<Skeleton_Component::GL_Buffer> m_skeletonBuffer;
};

#endif // PROP_VIEW_H