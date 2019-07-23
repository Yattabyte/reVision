#pragma once
#ifndef SKELETALANIMATION_SYSTEM_H
#define SKELETALANIMATION_SYSTEM_H 

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Engine.h"


/** A system responsible for animating props with skeleton components. */
class Skeletal_Animation : public BaseECSSystem {
public:
	// Public (de)Constructors
	/** Destroy the skeletal animation system. */
	inline ~Skeletal_Animation() = default;
	/** Construct a skeletal animation system. */
	inline Skeletal_Animation(Engine * engine) 
		: m_engine(engine) {
		// Declare component types used
		addComponentType(Skeleton_Component::ID, FLAG_REQUIRED);
	}


	// Public Interface Implementation	
	inline virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			Skeleton_Component * skeletonComponent = (Skeleton_Component*)componentParam[0];
			
			// Ensure skeleton has a mesh
			if (!skeletonComponent->m_mesh)
				skeletonComponent->m_mesh = Shared_Mesh(m_engine, "\\Models\\" + skeletonComponent->m_modelName);

			// Animate if the mesh has finished loading
			if (skeletonComponent->m_mesh->existsYet()) {
				// Animate if there exists an animation & bones
				if (skeletonComponent->m_animation != -1 && skeletonComponent->m_mesh->m_geometry.boneTransforms.size() > 0 && skeletonComponent->m_animation < skeletonComponent->m_mesh->m_geometry.animations.size()) {
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
	};


	// Public functions
	template <typename T> inline static T valueMix(const T &t1, const T &t2, const float &f) { return glm::mix(t1, t2, f); }
	template <> inline static glm::quat valueMix(const glm::quat &t1, const glm::quat &t2, const float &f) { return glm::slerp(t1, t2, f); }


protected:
	// Protected functions
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
			const size_t Index = Skeletal_Animation::FindKey(AnimationTime, keyCount - 1, keyVector);
			const size_t NextIndex = (Index + 1) > keyCount ? 0 : (Index + 1);
			const auto & Key = keyVector[Index];
			const auto & NextKey = keyVector[NextIndex];
			const float DeltaTime = (float)(NextKey.time - Key.time);
			const float Factor = glm::clamp((AnimationTime - (float)Key.time) / DeltaTime, 0.0f, 1.0f);
			return Skeletal_Animation::valueMix(Key.value, NextKey.value, Factor);
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


private:
	// Private Attributes
	Engine * m_engine = nullptr;
};

#endif // SKELETALANIMATION_SYSTEM_H