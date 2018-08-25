#pragma once
#ifndef SKELETONANIMATION_S_H
#define SKELETONANIMATION_S_H 

#include "ECS\Systems\ecsSystem.h"
#include <minmax.h>

/* Component Types Used */
#include "ECS\Components\Prop_C.h"
#include "ECS\Components\Skeleton_C.h"


/** A system responsible for animating props with skeleton components. */
class SkeletonAnimation_System : public BaseECSSystem {
public: 
	// (de)Constructors
	~SkeletonAnimation_System() {}
	SkeletonAnimation_System() : BaseECSSystem() {
		// Declare component types used
		addComponentType(Prop_Component::ID);
		addComponentType(Skeleton_Component::ID);
	}


	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) {
		for each (const auto & componentParam in components) {
			Prop_Component * propComponent = (Prop_Component*)componentParam[0];
			Skeleton_Component * skeletonComponent = (Skeleton_Component*)componentParam[1];

			if (!propComponent->m_model->existsYet())
				return;

			Skeleton_Buffer * uboData = skeletonComponent->m_data->data;
			std::shared_lock<std::shared_mutex> guard(propComponent->m_model->m_mutex);

			if (skeletonComponent->m_animation == -1 || skeletonComponent->m_transforms.size() == 0 || skeletonComponent->m_animation >= propComponent->m_model->m_animations.size())
				return;
			else {
				if (skeletonComponent->m_playAnim)
					skeletonComponent->m_animTime += deltaTime;
				const float TicksPerSecond = propComponent->m_model->m_animations[skeletonComponent->m_animation].ticksPerSecond != 0
					? propComponent->m_model->m_animations[skeletonComponent->m_animation].ticksPerSecond
					: 25.0f;
				const float TimeInTicks = skeletonComponent->m_animTime * TicksPerSecond;
				const float AnimationTime = fmod(TimeInTicks, propComponent->m_model->m_animations[skeletonComponent->m_animation].duration);
				skeletonComponent->m_animStart = skeletonComponent->m_animStart == -1 ? TimeInTicks : skeletonComponent->m_animStart;

				ReadNodeHeirarchy(skeletonComponent->m_transforms, AnimationTime, skeletonComponent->m_animation, propComponent->m_model->m_rootNode, propComponent->m_model, glm::mat4(1.0f));

				for (unsigned int i = 0, total = min(skeletonComponent->m_transforms.size(), NUM_MAX_BONES); i < total; i++)
					uboData->bones[i] = skeletonComponent->m_transforms[i].final;
			}
		}
	};


protected:
	// Protected functions
	inline void ReadNodeHeirarchy(std::vector<BoneTransform> &transforms, const float &animation_time, const int &animation_ID, const Node* parentNode, const Shared_Asset_Model &model, const glm::mat4& ParentTransform)
	{
		const std::string & NodeName = parentNode->name;
		const Animation &pAnimation = model->m_animations[animation_ID];
		const Node_Animation *pNodeAnim = FindNodeAnim(pAnimation, NodeName);
		glm::mat4 NodeTransformation = parentNode->transformation;

		// Interpolate scaling, rotation, and translation.
		// Generate their matrices and apply their transformations.
		if (pNodeAnim) {
			const glm::vec3 Scaling = InterpolateKeys<glm::vec3>(animation_time, pNodeAnim->scalingKeys);
			const glm::quat Rotation = InterpolateKeys<glm::quat>(animation_time, pNodeAnim->rotationKeys);
			const glm::vec3 Translation = InterpolateKeys<glm::vec3>(animation_time, pNodeAnim->positionKeys);

			NodeTransformation = glm::translate(glm::mat4(1.0f), Translation) * glm::mat4_cast(Rotation) * glm::scale(glm::mat4(1.0f), Scaling);
		}

		const glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;
		const glm::mat4 GlobalInverseTransform = glm::inverse(model->m_rootNode->transformation);
		const std::map<std::string, int> &BoneMap = model->m_boneMap;
		if (BoneMap.find(NodeName) != BoneMap.end()) {
			int BoneIndex = BoneMap.at(NodeName);
			transforms.at(BoneIndex).final = GlobalInverseTransform * GlobalTransformation * transforms.at(BoneIndex).offset;
		}

		for (unsigned int i = 0; i < parentNode->children.size(); ++i)
			ReadNodeHeirarchy(transforms, animation_time, animation_ID, parentNode->children[i], model, GlobalTransformation);
	}
	template <typename FROM, typename TO> inline TO convertType(const FROM &t) { return *((TO*)&t); }
	template <> inline glm::quat convertType(const aiQuaternion &t) { return glm::quat(t.w, t.x, t.y, t.z); }
	template <> inline glm::mat4 convertType(const aiMatrix4x4 &t) { return glm::mat4(t.a1, t.b1, t.c1, t.d1, t.a2, t.b2, t.c2, t.d2, t.a3, t.b3, t.c3, t.d3, t.a4, t.b4, t.c4, t.d4); }
	template <typename T> inline T valueMix(const T &t1, const T &t2, const float &f) { return glm::mix(t1, t2, f); }
	template <> inline glm::quat valueMix(const glm::quat &t1, const glm::quat &t2, const float &f) { return glm::slerp(t1, t2, f); }
	inline const Node_Animation * FindNodeAnim(const Animation & pAnimation, const std::string &NodeName)
	{
		for (unsigned int i = 0; i < pAnimation.numChannels; i++) {
			const Node_Animation * pNodeAnim = pAnimation.channels[i];
			if (pNodeAnim->nodeName == NodeName)
				return pNodeAnim;
		}
		return nullptr;
	}
	template <typename KeyType>
	inline int FindKey(const float &AnimationTime, const unsigned int &count, const std::vector<KeyType> & keys)
	{
		for (unsigned int i = 0; i < count; i++)
			if (AnimationTime < (float)(keys[i + 1]).time)
				return i;
		return 0;
	}
	template <typename ValueType>
	inline ValueType InterpolateKeys(const float &AnimationTime, const std::vector<Animation_Time_Key<ValueType>> & keys) {
		const unsigned int & keyCount = keys.size();
		assert(keyCount > 0);
		const ValueType &Result = keys[0].value;
		if (keyCount > 1) { // Ensure we have 2 values to interpolate between
			unsigned int Index = FindKey<Animation_Time_Key<ValueType>>(AnimationTime, keyCount - 1, keys);
			unsigned int NextIndex = (Index + 1) > keyCount ? 0 : (Index + 1);
			const Animation_Time_Key<ValueType> &Key = keys[Index];
			const Animation_Time_Key<ValueType> &NextKey = keys[NextIndex];
			const float DeltaTime = (float)(NextKey.time - Key.time);
			const float Factor = glm::clamp((AnimationTime - (float)Key.time) / DeltaTime, 0.0f, 1.0f);
			return valueMix(Key.value, NextKey.value, Factor);
		}
		return Result;
	}
};

#endif // SKELETONANIMATION_S_H