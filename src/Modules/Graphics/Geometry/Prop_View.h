#pragma once
#ifndef PROP_VIEW_H
#define PROP_VIEW_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Geometry/components.h"
#include "Modules/World/ECS/TransformComponent.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/GL_ArrayBuffer.h"
#include "Engine.h"
#include "glm/gtx/component_wise.hpp"
#include <vector>

#define NUM_MAX_BONES 100


/** A core rendering technique for rendering props from a given viewing perspective. */
class Prop_View : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Destructor. */
	inline ~Prop_View() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	inline Prop_View(Engine * engine)
		: m_engine(engine), Graphics_Technique(GEOMETRY) {
		// Asset Loading
		m_shaderCull = Shared_Shader(m_engine, "Core\\Props\\culling");
		m_shaderGeometry = Shared_Shader(m_engine, "Core\\Props\\geometry");
		m_shapeCube = Shared_Primitive(m_engine, "cube");
		m_modelsVAO = &m_engine->getManager_Models().getVAO();

		// Add New Component Types
		auto & world = m_engine->getModule_World();
		world.addNotifyOnComponentType(Prop_Component::ID, m_aliveIndicator, [&](BaseECSComponent * c) {
			auto * component = (Prop_Component*)c;
			component->m_propBufferIndex = m_propBuffer.newElement();
		});
		world.addNotifyOnComponentType(Skeleton_Component::ID, m_aliveIndicator, [&](BaseECSComponent * c) {
			auto * component = (Skeleton_Component*)c;
			component->m_skeleBufferIndex = m_skeletonBuffer.newElement();
		});

		// World-Changed Callback
		world.addLevelListener(m_aliveIndicator, [&](const World_Module::WorldState & state) {
			if (state == World_Module::unloaded)
				clear();
		});
	}


	// Public Interface Implementations
	inline virtual void beginFrame(const float & deltaTime) override {
		m_propBuffer.beginWriting();
		m_skeletonBuffer.beginWriting();
		m_bufferPropIndex.beginWriting();
		m_bufferCulling.beginWriting();
		m_bufferRender.beginWriting();
		m_bufferSkeletonIndex.beginWriting();

		// Apply skeletal animation
		auto & world = m_engine->getModule_World();
		world.updateSystem(
			deltaTime,
			{ Skeleton_Component::ID },
			{ BaseECSSystem::FLAG_REQUIRED },
			[&](const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) {
			animateComponents(deltaTime, components);
		});
		// Synchronize technique related components
		world.updateSystem(
			deltaTime,
			{ Prop_Component::ID, Skeleton_Component::ID, Transform_Component::ID },
			{ BaseECSSystem::FLAG_REQUIRED, BaseECSSystem::FLAG_OPTIONAL, BaseECSSystem::FLAG_OPTIONAL },
			[&](const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) {
			syncComponents(deltaTime, components);
		});
	}
	inline virtual void endFrame(const float & deltaTime) override {
		m_propBuffer.endWriting();
		m_skeletonBuffer.endWriting();
		m_bufferPropIndex.endWriting();
		m_bufferCulling.endWriting();
		m_bufferRender.endWriting();
		m_bufferSkeletonIndex.endWriting();
	}
	inline virtual void renderTechnique(const float & deltaTime) override {
		// Exit Early
		if (!m_enabled || !m_shapeCube->existsYet() || !m_shaderCull->existsYet() || !m_shaderGeometry->existsYet())
			return;

		// Populate render-lists
		m_engine->getModule_World().updateSystem(
			deltaTime,
			{ Prop_Component::ID, Skeleton_Component::ID },
			{ BaseECSSystem::FLAG_REQUIRED, BaseECSSystem::FLAG_OPTIONAL },
			[&](const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) {
			updateVisibility(deltaTime, components);
		});

		// Apply occlusion culling and render props
		renderGeometry(deltaTime);
	}	


	// Public Methods
	/** Retrieve the prop buffer.
	@return		the prop buffer. */
	inline auto & getPropBuffer() {
		return m_propBuffer;
	}
	/** Retrieve the skeleton buffer.
	@return		the skeleton buffer. */
	inline auto & getSkeletonBuffer() {
		return m_skeletonBuffer;
	}
	/***/
	template <typename T> inline static T valueMix(const T &t1, const T &t2, const float &f) { return glm::mix(t1, t2, f); }
	/***/
	template <> inline static glm::quat valueMix(const glm::quat &t1, const glm::quat &t2, const float &f) { return glm::slerp(t1, t2, f); }


private:
	// Private Methods
	inline void animateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) {
		for each (const auto & componentParam in components) {
			Skeleton_Component * skeletonComponent = (Skeleton_Component*)componentParam[0];
			if (!skeletonComponent->m_mesh->existsYet())
				return;

			if (skeletonComponent->m_animation == -1 || skeletonComponent->m_mesh->m_geometry.boneTransforms.size() == 0 || skeletonComponent->m_animation >= skeletonComponent->m_mesh->m_geometry.animations.size())
				return;
			else {
				skeletonComponent->m_transforms.resize(skeletonComponent->m_mesh->m_geometry.boneTransforms.size());
				if (skeletonComponent->m_playAnim)
					skeletonComponent->m_animTime += deltaTime;
				const float TicksPerSecond = skeletonComponent->m_mesh->m_geometry.animations[skeletonComponent->m_animation].ticksPerSecond != 0.00
					? (float)(skeletonComponent->m_mesh->m_geometry.animations[skeletonComponent->m_animation].ticksPerSecond)
					: 25.0f;
				const float TimeInTicks = skeletonComponent->m_animTime * TicksPerSecond;
				const float AnimationTime = fmodf(TimeInTicks, float(skeletonComponent->m_mesh->m_geometry.animations[skeletonComponent->m_animation].duration));
				skeletonComponent->m_animStart = skeletonComponent->m_animStart == -1 ? (float)TimeInTicks : skeletonComponent->m_animStart;

				ReadNodeHeirarchy(skeletonComponent->m_transforms, AnimationTime, skeletonComponent->m_animation, skeletonComponent->m_mesh->m_geometry.rootNode, skeletonComponent->m_mesh, glm::mat4(1.0f));
			}
		}
	}
	inline void syncComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) {
		for each (const auto & componentParam in components) {
			Prop_Component * propComponent = (Prop_Component*)componentParam[0];
			Skeleton_Component * skeletonComponent = (Skeleton_Component*)componentParam[1];
			Transform_Component * transformComponent = (Transform_Component*)componentParam[2];
			const auto & index = propComponent->m_propBufferIndex;
			if (!propComponent->m_model->existsYet())	continue;

			// Sync Transform Attributes
			if (transformComponent) {
				const auto & position = transformComponent->m_transform.m_position;
				const auto & orientation = transformComponent->m_transform.m_orientation;
				const auto & scale = transformComponent->m_transform.m_scale;
				const auto matRot = glm::mat4_cast(orientation);
				propComponent->mMatrix = transformComponent->m_transform.m_modelMatrix;

				// Update bounding sphere
				const glm::vec3 bboxMax_World = (propComponent->m_model->m_bboxMax * scale) + position;
				const glm::vec3 bboxMin_World = (propComponent->m_model->m_bboxMin * scale) + position;
				const glm::vec3 bboxCenter = (bboxMax_World + bboxMin_World) / 2.0f;
				const glm::vec3 bboxScale = (bboxMax_World - bboxMin_World) / 2.0f;
				glm::mat4 matTrans = glm::translate(glm::mat4(1.0f), bboxCenter);
				glm::mat4 matScale = glm::scale(glm::mat4(1.0f), bboxScale);
				glm::mat4 matFinal = (matTrans * matRot * matScale);
				propComponent->bBoxMatrix = matFinal;
				propComponent->m_radius = glm::compMax(propComponent->m_model->m_radius * scale);
				propComponent->m_position = propComponent->m_model->m_bboxCenter + position;

			}

			// Sync Animation Attributes
			if (skeletonComponent) {
				propComponent->m_static = false;
				auto & bones = m_skeletonBuffer[skeletonComponent->m_skeleBufferIndex].bones;
				for (size_t i = 0, total = std::min(skeletonComponent->m_transforms.size(), size_t(NUM_MAX_BONES)); i < total; ++i)
					bones[i] = skeletonComponent->m_transforms[i];
			}

			// Sync Prop Attributes
			m_propBuffer[index].materialID = propComponent->m_skin;
			m_propBuffer[index].isStatic = propComponent->m_static;
			m_propBuffer[index].mMatrix = propComponent->mMatrix;
			m_propBuffer[index].bBoxMatrix = propComponent->bBoxMatrix;			
		}
	}
	inline void updateVisibility(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) {
		// Accumulate draw parameter information per model
		std::vector<glm::ivec4> cullingDrawData, renderingDrawData;
		std::vector<GLuint> visibleIndices;
		std::vector<int> skeletonData;
		const glm::vec3 & eyePosition = (*m_cameraBuffer)->EyePosition;
		for each (const auto & componentParam in components) {
			Prop_Component * propComponent = (Prop_Component*)componentParam[0];
			Skeleton_Component * skeletonComponent = (Skeleton_Component*)componentParam[1];
			const auto & offset = propComponent->m_model->m_offset;
			const auto & count = propComponent->m_model->m_count;
			const auto & index = propComponent->m_propBufferIndex;
			if (!propComponent->m_model->existsYet())	continue;			

			// Flag for occlusion culling if mesh complexity is high enough and if viewer is NOT within BSphere
			visibleIndices.push_back((GLuint)*index);
			if ((count >= 100) && propComponent->m_radius < glm::distance(propComponent->m_position, eyePosition)) {
				// Allow
				cullingDrawData.push_back(glm::ivec4(36, 1, 0, 1));
				renderingDrawData.push_back(glm::ivec4(count, 0, offset, 1));
			}
			else {
				// Skip occlusion culling		
				cullingDrawData.push_back(glm::ivec4(36, 0, 0, 1));
				renderingDrawData.push_back(glm::ivec4(count, 1, offset, 1));
			}
			skeletonData.push_back(skeletonComponent ? (GLint)*skeletonComponent->m_skeleBufferIndex : -1); // get skeleton ID if this entity has one
		}

		// Update camera buffers
		m_propCount = (GLsizei)visibleIndices.size();
		m_bufferPropIndex.write(0, sizeof(GLuint) * m_propCount, visibleIndices.data());
		m_bufferCulling.write(0, sizeof(glm::ivec4) * m_propCount, cullingDrawData.data());
		m_bufferRender.write(0, sizeof(glm::ivec4) * m_propCount, renderingDrawData.data());
		m_bufferSkeletonIndex.write(0, sizeof(int) * m_propCount, skeletonData.data());
	}
	/***/
	inline void renderGeometry(const float & deltaTime) {
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
		m_gfxFBOS->bindForWriting("GEOMETRY");
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
	/** Clear out the props queued up for rendering. */
	inline void clear() {
		m_propCount = 0;
		m_propBuffer.clear();
		m_skeletonBuffer.clear();
	}


protected:
	// Protected Methods
	/** Search for a node in the animation system matching the name specified.
	@param	pAnimation		the animation system to search through.
	@param	NodeName		the name of the node to find.
	@return					pointer to the node matching the name specified if found, nullptr otherwise. */
	inline static constexpr auto FindNodeAnim = [](const Animation & pAnimation, const std::string & NodeName) -> const Node_Animation* {
		for (unsigned int i = 0; i < pAnimation.numChannels; i++) {
			const Node_Animation * pNodeAnim = pAnimation.channels[i];
			if (pNodeAnim->nodeName == NodeName)
				return pNodeAnim;
		}
		return nullptr;
	};
	/** Search for a keyframe appropriate for the current animation time.
	@param	AnimationTime	the current time in the animation.
	@param	count			the number of key frames.
	@param	keyVector		array of key frames.
	@return					an appropriate keyframe, 0 otherwise. */
	inline static constexpr auto FindKey = [](const float & AnimationTime, const size_t & count, const auto & keyVector) -> const size_t {
		for (size_t i = 0; i < count; i++)
			if (AnimationTime < (float)(keyVector[i + 1]).time)
				return i;
		return size_t(0);
	};
	/** Interpolate between this keyframe, and the next one, based on the animation time.
	@param	AnimationTime	the current time in the animation.
	@param	keyVector		array of key frames.
	@return					a new keyframe value. */
	inline static constexpr auto InterpolateKeys = [](const float &AnimationTime, const auto & keyVector) {
		const size_t & keyCount = keyVector.size();
		assert(keyCount > 0);
		const auto & Result = keyVector[0].value;
		if (keyCount > 1) { // Ensure we have 2 values to interpolate between
			const size_t Index = Prop_View::FindKey(AnimationTime, keyCount - 1, keyVector);
			const size_t NextIndex = (Index + 1) > keyCount ? 0 : (Index + 1);
			const auto & Key = keyVector[Index];
			const auto & NextKey = keyVector[NextIndex];
			const float DeltaTime = (float)(NextKey.time - Key.time);
			const float Factor = glm::clamp((AnimationTime - (float)Key.time) / DeltaTime, 0.0f, 1.0f);
			return Prop_View::valueMix(Key.value, NextKey.value, Factor);
		}
		return Result;
	};
	/** Process animation nodes from a scene, updating a series of transformation matrix representing the bones in the skeleton.
	@param	transforms		matrix vector representing bones in the skeleton.
	@param	AnimationTime	the current time in the animation.
	@param	animation_ID	id for the current animation to proccess.
	@param	parentNode		parent node in the node hierarchy.
	@param	model			the model to process the animations from.
	@param	ParentTransform	parent transform in the node hierarchy. */
	inline static void ReadNodeHeirarchy(std::vector<glm::mat4> & transforms, const float & AnimationTime, const int & animation_ID, const Node * parentNode, const Shared_Mesh & model, const glm::mat4 & ParentTransform) {
		const std::string & NodeName = parentNode->name;
		const Animation & pAnimation = model->m_geometry.animations[animation_ID];
		const Node_Animation * pNodeAnim = FindNodeAnim(pAnimation, NodeName);
		glm::mat4 NodeTransformation = parentNode->transformation;

		// Interpolate scaling, rotation, and translation.
		// Generate their matrices and apply their transformations.
		if (pNodeAnim) {
			const glm::vec3 Scaling = InterpolateKeys(AnimationTime, pNodeAnim->scalingKeys);
			const glm::quat Rotation = InterpolateKeys(AnimationTime, pNodeAnim->rotationKeys);
			const glm::vec3 Translation = InterpolateKeys(AnimationTime, pNodeAnim->positionKeys);

			NodeTransformation = glm::translate(glm::mat4(1.0f), Translation) * glm::mat4_cast(Rotation) * glm::scale(glm::mat4(1.0f), Scaling);
		}

		const glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;
		const glm::mat4 GlobalInverseTransform = glm::inverse(model->m_geometry.rootNode->transformation);
		const std::map<std::string, size_t> &BoneMap = model->m_geometry.boneMap;
		if (BoneMap.find(NodeName) != BoneMap.end()) {
			size_t BoneIndex = BoneMap.at(NodeName);
			transforms.at(BoneIndex) = GlobalInverseTransform * GlobalTransformation * model->m_geometry.boneTransforms.at(BoneIndex);
		}

		for (unsigned int i = 0; i < parentNode->children.size(); ++i)
			ReadNodeHeirarchy(transforms, AnimationTime, animation_ID, parentNode->children[i], model, GlobalTransformation);
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	const GLuint * m_modelsVAO = nullptr;
	Shared_Shader m_shaderCull, m_shaderGeometry;
	Shared_Primitive m_shapeCube;
	GLsizei m_propCount = 0;
	DynamicBuffer m_bufferPropIndex, m_bufferCulling, m_bufferRender, m_bufferSkeletonIndex;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);

	// Core Prop Data
	/** OpenGL buffer for props. */
	struct Prop_Buffer {
		GLuint materialID;
		GLuint isStatic; glm::vec2 padding1;
		glm::mat4 mMatrix;
		glm::mat4 bBoxMatrix;
	};
	/** OpenGL buffer for prop skeletons. */
	struct Skeleton_Buffer {
		glm::mat4 bones[NUM_MAX_BONES];
	};
	GL_ArrayBuffer<Prop_Buffer> m_propBuffer;
	GL_ArrayBuffer<Skeleton_Buffer> m_skeletonBuffer;
};

#endif // PROP_VIEW_H