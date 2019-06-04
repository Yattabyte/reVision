#pragma once
#ifndef GRAPHICS_GEOMETRY_COMPONENTS_H
#define GRAPHICS_GEOMETRY_COMPONENTS_H

#include "Assets/Mesh.h"
#include "Assets/Model.h"
#include "Modules/Graphics/Common/CameraBuffer.h"
#include "Modules/Graphics/Common/FBO_EnvMap.h"
#include "Modules/World/ECS/ecsComponent.h"
#include "Utilities/Transform.h"
#include "Utilities/GL/VectorBuffer.h"
#include "glm/glm.hpp"


/** A static, animation-less prop component. */
struct Prop_Component : public ECSComponent<Prop_Component> {
	/** OpenGL buffer for basic props. */
	struct GL_Buffer {
		GLuint materialID; glm::vec3 padding1;
		glm::mat4 mMatrix;
		glm::mat4 bBoxMatrix;
	};

	Shared_Model m_model;
	VB_Element<GL_Buffer> * m_data = nullptr;
	float m_radius = 1.0f;
	glm::vec3 m_position = glm::vec3(0.0f);
};

/** A skeleton component is used in combination with prop components to allow for skeletal animation.
@note		is useless by itself. */
struct Skeleton_Component : public ECSComponent<Skeleton_Component> {
	/** OpenGL buffer for animated props. */
	struct GL_Buffer {
	#define NUM_MAX_BONES 100
		glm::mat4 bones[NUM_MAX_BONES];
	};

	int m_animation = -1;
	bool m_playAnim = true;
	float m_animTime = 0, m_animStart = 0;
	Shared_Mesh m_mesh;
	std::vector<glm::mat4> m_transforms;
	VB_Element<GL_Buffer> * m_data = nullptr;
};

#endif // GRAPHICS_GEOMETRY_COMPONENTS_H
