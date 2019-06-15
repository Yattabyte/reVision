#pragma once
#ifndef GRAPHICS_GEOMETRY_COMPONENTS_H
#define GRAPHICS_GEOMETRY_COMPONENTS_H

#include "Assets/Mesh.h"
#include "Assets/Model.h"
#include "Modules/World/ECS/ecsComponent.h"
#include "glm/glm.hpp"


/** A static, animation-less prop component. */
struct Prop_Component : public ECSComponent<Prop_Component> {
	Shared_Model m_model;
	unsigned int m_skin = 0u;
	float m_radius = 1.0f;
	glm::vec3 m_position = glm::vec3(0.0f);
	glm::mat4 mMatrix;
	glm::mat4 bBoxMatrix;
	std::shared_ptr<size_t> m_propBufferIndex = 0ull;
};

/** A skeleton component is used in combination with prop components to allow for skeletal animation.
@note		is useless by itself. */
struct Skeleton_Component : public ECSComponent<Skeleton_Component> {
	Shared_Mesh m_mesh;
	int m_animation = -1;
	bool m_playAnim = true;
	float m_animTime = 0, m_animStart = 0;
	std::vector<glm::mat4> m_transforms;
	std::shared_ptr<size_t> m_skeleBufferIndex = 0ull;
};

#endif // GRAPHICS_GEOMETRY_COMPONENTS_H
