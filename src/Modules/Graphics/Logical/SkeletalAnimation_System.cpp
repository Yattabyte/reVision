#include "Modules/Graphics/Logical/SkeletalAnimation_System.h"
#include "Modules/ECS/component_types.h"


Skeletal_Animation_System::Skeletal_Animation_System(Engine& engine) noexcept :
	m_engine(engine)
{
	// Declare component types used
	addComponentType(Skeleton_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
}

void Skeletal_Animation_System::updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) 
{
	for (const auto& componentParam : components) {
		auto* skeletonComponent = static_cast<Skeleton_Component*>(componentParam[0]);

		// Animate if the mesh has finished loading
		if (skeletonComponent->m_mesh->ready()) {
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
}

/** Search for a node in the animation system matching the name specified.
	@param	pAnimation		the animation system to search through.
	@param	NodeName		the name of the node to find.
	@return					pointer to the node matching the name specified if found, nullptr otherwise. */
inline static constexpr auto FindNodeAnim = [](const Animation& pAnimation, const std::string& NodeName) noexcept -> const Node_Animation* {
	for (unsigned int i = 0; i < pAnimation.numChannels; i++) {
		auto& pNodeAnim = pAnimation.channels[i];
		if (pNodeAnim.nodeName == NodeName)
			return &pNodeAnim;
	}
	return nullptr;
};

/** Search for a key-frame appropriate for the current animation time.
@param	AnimationTime	the current time in the animation.
@param	count			the number of key frames.
@param	keyVector		array of key frames.
@return					an appropriate key-frame, 0 otherwise. */
inline static constexpr auto FindKey = [](const float& AnimationTime, const size_t& count, const auto& keyVector) noexcept {
	for (size_t i = 0; i < count; i++)
		if (AnimationTime < (float)(keyVector[i + 1]).time)
			return i;
	return 0ULL;
};

/** Interpolate between this key-frame, and the next one, based on the animation time.
@param	AnimationTime	the current time in the animation.
@param	keyVector		array of key frames.
@return					a new key-frame value. */
inline static constexpr auto InterpolateKeys = [](const float& AnimationTime, const auto& keyVector) noexcept {
	const size_t& keyCount = keyVector.size();
	assert(keyCount > 0);
	const auto& Result = keyVector[0].value;
	if (keyCount > 1) { // Ensure we have 2 values to interpolate between
		const size_t Index = FindKey(AnimationTime, keyCount - 1, keyVector);
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

void Skeletal_Animation_System::ReadNodeHeirarchy(std::vector<glm::mat4>& transforms, const float& AnimationTime, const int& animation_ID, const Node& parentNode, const Shared_Mesh& model, const glm::mat4& ParentTransform) 
{
	const std::string& NodeName = parentNode.name;
	const Animation& pAnimation = model->m_geometry.animations[animation_ID];
	glm::mat4 NodeTransformation = parentNode.transformation;

	// Interpolate scaling, rotation, and translation.
	// Generate their matrices and apply their transformations.
	if (const auto* pNodeAnim = FindNodeAnim(pAnimation, NodeName)) {
		const glm::vec3 Scaling = InterpolateKeys(AnimationTime, pNodeAnim->scalingKeys);
		const glm::quat Rotation = InterpolateKeys(AnimationTime, pNodeAnim->rotationKeys);
		const glm::vec3 Translation = InterpolateKeys(AnimationTime, pNodeAnim->positionKeys);

		NodeTransformation = glm::translate(glm::mat4(1.0f), Translation) * glm::mat4_cast(Rotation) * glm::scale(glm::mat4(1.0f), Scaling);
	}

	const glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;
	const glm::mat4 GlobalInverseTransform = glm::inverse(model->m_geometry.rootNode.transformation);
	const std::map<std::string, size_t>& BoneMap = model->m_geometry.boneMap;
	if (BoneMap.find(NodeName) != BoneMap.end()) {
		const size_t BoneIndex = BoneMap.at(NodeName);
		transforms.at(BoneIndex) = GlobalInverseTransform * GlobalTransformation * model->m_geometry.boneTransforms.at(BoneIndex);
	}

	for (const auto& childNode : parentNode.children)
		ReadNodeHeirarchy(transforms, AnimationTime, animation_ID, childNode, model, GlobalTransformation);
}