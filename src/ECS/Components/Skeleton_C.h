#pragma once
#ifndef SKELETALANIMATION_C_H
#define SKELETALANIMATION_C_H

#include "ECS\Components\ecsComponent.h"
#include "Assets\Asset_Model.h"


/** OpenGL buffer for animated props.
Formatted to adhere to glsl specifications. */
struct Skeleton_Buffer {
#define NUM_MAX_BONES 100
	glm::mat4 bones[NUM_MAX_BONES];
};

/** A skeleton component is used in combination with prop components to allow for skeletal animation. 
@note		is useless by itself. */
struct Skeleton_Component : public ECSComponent<Skeleton_Component> {
	int m_animation = -1;
	bool m_playAnim = true;
	float m_animTime = 0, m_animStart = 0;
	Shared_Asset_Model m_model;
	std::vector<glm::mat4> m_transforms;
	VB_Element<Skeleton_Buffer> * m_data;
};

#endif // SKELETALANIMATION_C_H