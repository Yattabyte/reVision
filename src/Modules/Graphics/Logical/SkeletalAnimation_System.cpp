#include "Modules/Graphics/Logical/SkeletalAnimation_System.h"


Skeletal_Animation_System::Skeletal_Animation_System(Engine& engine) noexcept :
	m_engine(engine)
{
	// Declare component types used
	addComponentType(Skeleton_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
}

void Skeletal_Animation_System::updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept 
{
	for (const auto& componentParam : components) {
		auto* skeletonComponent = static_cast<Skeleton_Component*>(componentParam[0]);

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
}

void Skeletal_Animation_System::ReadNodeHeirarchy(std::vector<glm::mat4>& transforms, const float& AnimationTime, const int& animation_ID, const Node* parentNode, const Shared_Mesh& model, const glm::mat4& ParentTransform) noexcept 
{
	const std::string& NodeName = parentNode->name;
	const Animation& pAnimation = model->m_geometry.animations[animation_ID];
	const Node_Animation* pNodeAnim = FindNodeAnim(pAnimation, NodeName);
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
	const std::map<std::string, size_t>& BoneMap = model->m_geometry.boneMap;
	if (BoneMap.find(NodeName) != BoneMap.end()) {
		size_t BoneIndex = BoneMap.at(NodeName);
		transforms.at(BoneIndex) = GlobalInverseTransform * GlobalTransformation * model->m_geometry.boneTransforms.at(BoneIndex);
	}

	for (auto& childNode : parentNode->children)
		ReadNodeHeirarchy(transforms, AnimationTime, animation_ID, childNode, model, GlobalTransformation);
}