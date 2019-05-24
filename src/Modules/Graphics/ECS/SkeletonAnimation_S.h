#pragma once
#ifndef SKELETONANIMATION_S_H
#define SKELETONANIMATION_S_H 

#include "Modules/World/ecsSystem.h"

/* Component Types Used */
#include "Modules/Graphics/ECS/Prop_C.h"
#include "Modules/Graphics/ECS/Skeleton_C.h"


/** A system responsible for animating props with skeleton components. */
class SkeletonAnimation_System : public BaseECSSystem {
public: 
	// (de)Constructors
	~SkeletonAnimation_System() = default;
	SkeletonAnimation_System() : BaseECSSystem() {
		// Declare component types used
		addComponentType(Skeleton_Component::ID);
	}


	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			Skeleton_Component * skeletonComponent = (Skeleton_Component*)componentParam[0];
			if (!skeletonComponent->m_mesh->existsYet())
				return;

			Skeleton_Buffer * uboData = skeletonComponent->m_data->data;
			if (skeletonComponent->m_animation == -1 || skeletonComponent->m_mesh->m_geometry.boneTransforms.size() == 0 || skeletonComponent->m_animation >= skeletonComponent->m_mesh->m_geometry.animations.size())
				return;
			else {
				skeletonComponent->m_transforms.resize(skeletonComponent->m_mesh->m_geometry.boneTransforms.size());
				if (skeletonComponent->m_playAnim)
					skeletonComponent->m_animTime += deltaTime;
				const double TicksPerSecond = skeletonComponent->m_mesh->m_geometry.animations[skeletonComponent->m_animation].ticksPerSecond != 0
					? skeletonComponent->m_mesh->m_geometry.animations[skeletonComponent->m_animation].ticksPerSecond
					: 25.0f;
				const double TimeInTicks = skeletonComponent->m_animTime * TicksPerSecond;
				const double AnimationTime = fmod(TimeInTicks, skeletonComponent->m_mesh->m_geometry.animations[skeletonComponent->m_animation].duration);
				skeletonComponent->m_animStart = skeletonComponent->m_animStart == -1 ? (float)TimeInTicks : skeletonComponent->m_animStart;

				ReadNodeHeirarchy(skeletonComponent->m_transforms, AnimationTime, skeletonComponent->m_animation, skeletonComponent->m_mesh->m_geometry.rootNode, skeletonComponent->m_mesh, glm::mat4(1.0f));

				for (size_t i = 0, total = std::min(skeletonComponent->m_transforms.size(), size_t(NUM_MAX_BONES)); i < total; ++i)
					uboData->bones[i] = skeletonComponent->m_transforms[i];
			}
		}
	};

	
	// Public functions
	template <typename T> static inline T valueMix(const T &t1, const T &t2, const float &f) { return glm::mix(t1, t2, f); }
	template <> inline static glm::quat valueMix(const glm::quat &t1, const glm::quat &t2, const float &f) { return glm::slerp(t1, t2, f); }


protected:
	// Protected functions
	static constexpr auto FindNodeAnim = [](const Animation & pAnimation, const std::string & NodeName) -> const Node_Animation* {
		for (unsigned int i = 0; i < pAnimation.numChannels; i++) {
			const Node_Animation * pNodeAnim = pAnimation.channels[i];
			if (pNodeAnim->nodeName == NodeName)
				return pNodeAnim;
		}
		return nullptr;
	};
	static constexpr auto FindKey = [](const float & AnimationTime, const size_t & count, const auto & keyVector) -> const size_t {
		for (size_t i = 0; i < count; i++)
			if (AnimationTime < (float)(keyVector[i + 1]).time)
				return i;
		return size_t(0);
	};	
	static constexpr auto InterpolateKeys = [](const float &AnimationTime, const auto & keyVector) {
		const size_t & keyCount = keyVector.size();
		assert(keyCount > 0);
		const auto & Result = keyVector[0].value;
		if (keyCount > 1) { // Ensure we have 2 values to interpolate between
			const size_t Index = SkeletonAnimation_System::FindKey(AnimationTime, keyCount - 1, keyVector);
			const size_t NextIndex = (Index + 1) > keyCount ? 0 : (Index + 1);
			const auto & Key = keyVector[Index];
			const auto & NextKey = keyVector[NextIndex];
			const float DeltaTime = (float)(NextKey.time - Key.time);
			const float Factor = glm::clamp((AnimationTime - (float)Key.time) / DeltaTime, 0.0f, 1.0f);
			return SkeletonAnimation_System::valueMix(Key.value, NextKey.value, Factor);
		}
		return Result;
	};
	inline static void ReadNodeHeirarchy(std::vector<glm::mat4> & transforms, const double & animation_time, const int & animation_ID, const Node * parentNode, const Shared_Mesh & model, const glm::mat4 & ParentTransform) {
		const std::string & NodeName = parentNode->name;
		const Animation & pAnimation = model->m_geometry.animations[animation_ID];
		const Node_Animation * pNodeAnim = FindNodeAnim(pAnimation, NodeName);
		glm::mat4 NodeTransformation = parentNode->transformation;

		// Interpolate scaling, rotation, and translation.
		// Generate their matrices and apply their transformations.
		if (pNodeAnim) {
			const glm::vec3 Scaling = InterpolateKeys((float)animation_time, pNodeAnim->scalingKeys);
			const glm::quat Rotation = InterpolateKeys((float)animation_time, pNodeAnim->rotationKeys);
			const glm::vec3 Translation = InterpolateKeys((float)animation_time, pNodeAnim->positionKeys);

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
			ReadNodeHeirarchy(transforms, animation_time, animation_ID, parentNode->children[i], model, GlobalTransformation);
	}
};

#endif // SKELETONANIMATION_S_H