#pragma once
#ifndef SKELETALANIMATION_SYSTEM_H
#define SKELETALANIMATION_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Assets/Mesh.h"
#include "glm/glm.hpp"


// Forward Declarations
class Engine;
struct Node;

/** A system responsible for animating props with skeleton components. */
class Skeletal_Animation_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this skeletal animation system. */
	inline ~Skeletal_Animation_System() = default;
	/** Construct a skeletal animation system.
	@param	engine		reference to the engine to use. */
	explicit Skeletal_Animation_System(Engine& engine);


	// Public Interface Implementation
	void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) final;


protected:
	// Protected Methods
	/** Process animation nodes from a scene, updating a series of transformation matrix representing the bones in the skeleton.
	@param	transforms		matrix vector representing bones in the skeleton.
	@param	AnimationTime	the current time in the animation.
	@param	animation_ID	id for the current animation to process.
	@param	parentNode		parent node in the node hierarchy.
	@param	model			the model to process the animations from.
	@param	ParentTransform	parent transform in the node hierarchy. */
	static void ReadNodeHeirarchy(std::vector<glm::mat4>& transforms, const float& AnimationTime, const int& animation_ID, const Node& parentNode, const Shared_Mesh& model, const glm::mat4& ParentTransform);


private:
	// Private Attributes
	Engine& m_engine;
};

#endif // SKELETALANIMATION_SYSTEM_H