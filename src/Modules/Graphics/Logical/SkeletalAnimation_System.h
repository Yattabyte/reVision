#pragma once
#ifndef SKELETALANIMATION_SYSTEM_H
#define SKELETALANIMATION_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Engine.h"


/** A system responsible for animating props with skeleton components. */
class Skeletal_Animation_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this skeletal animation system. */
	inline ~Skeletal_Animation_System() = default;
	/** Construct a skeletal animation system.
	@param	engine		reference to the engine to use. */
	explicit Skeletal_Animation_System(Engine& engine) noexcept;


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept override final;;


protected:
	// Protected functions
	/** Search for a node in the animation system matching the name specified.
	@param	pAnimation		the animation system to search through.
	@param	NodeName		the name of the node to find.
	@return					pointer to the node matching the name specified if found, nullptr otherwise. */
	inline static constexpr auto FindNodeAnim = [](const Animation& pAnimation, const std::string& NodeName) -> const Node_Animation* {
		for (unsigned int i = 0; i < pAnimation.numChannels; i++) {
			const Node_Animation* pNodeAnim = pAnimation.channels[i];
			if (pNodeAnim->nodeName == NodeName)
				return pNodeAnim;
		}
		return nullptr;
	};
	/** Search for a key-frame appropriate for the current animation time.
	@param	AnimationTime	the current time in the animation.
	@param	count			the number of key frames.
	@param	keyVector		array of key frames.
	@return					an appropriate key-frame, 0 otherwise. */
	inline static constexpr auto FindKey = [](const float& AnimationTime, const size_t& count, const auto& keyVector) -> const size_t {
		for (size_t i = 0; i < count; i++)
			if (AnimationTime < (float)(keyVector[i + 1]).time)
				return i;
		return size_t(0);
	};
	/** Interpolate between this key-frame, and the next one, based on the animation time.
	@param	AnimationTime	the current time in the animation.
	@param	keyVector		array of key frames.
	@return					a new key-frame value. */
	inline static constexpr auto InterpolateKeys = [](const float& AnimationTime, const auto& keyVector) {
		const size_t& keyCount = keyVector.size();
		assert(keyCount > 0);
		const auto& Result = keyVector[0].value;
		if (keyCount > 1) { // Ensure we have 2 values to interpolate between
			const size_t Index = Skeletal_Animation_System::FindKey(AnimationTime, keyCount - 1, keyVector);
			const size_t NextIndex = (Index + 1) > keyCount ? 0 : (Index + 1);
			const auto& Key = keyVector[Index];
			const auto& NextKey = keyVector[NextIndex];
			const float DeltaTime = (float)(NextKey.time - Key.time);
			const float Factor = glm::clamp((AnimationTime - (float)Key.time) / DeltaTime, 0.0f, 1.0f);
			if constexpr (std::is_same<decltype(Key.value), glm::quat>::value)
				return glm::slerp(Key.value, NextKey.value, Factor); 
			return glm::mix(Key.value, NextKey.value, Factor);
		}
		return Result;
	};
	/** Process animation nodes from a scene, updating a series of transformation matrix representing the bones in the skeleton.
	@param	transforms		matrix vector representing bones in the skeleton.
	@param	AnimationTime	the current time in the animation.
	@param	animation_ID	id for the current animation to process.
	@param	parentNode		parent node in the node hierarchy.
	@param	model			the model to process the animations from.
	@param	ParentTransform	parent transform in the node hierarchy. */
	static void ReadNodeHeirarchy(std::vector<glm::mat4>& transforms, const float& AnimationTime, const int& animation_ID, const Node* parentNode, const Shared_Mesh& model, const glm::mat4& ParentTransform) noexcept;


private:
	// Private Attributes
	Engine& m_engine;
};

#endif // SKELETALANIMATION_SYSTEM_H