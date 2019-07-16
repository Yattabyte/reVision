#pragma once
#ifndef GRAPHICS_GEOMETRY_COMPONENTS_H
#define GRAPHICS_GEOMETRY_COMPONENTS_H

#include "Assets/Mesh.h"
#include "Assets/Model.h"
#include "Modules/World/ECS/ecsComponent.h"
#include "Utilities/GL/GL_ArrayBuffer.h"
#include "glm/glm.hpp"


struct Prop_Component : public ECSComponent<Prop_Component> {
	Shared_Model m_model;
	bool m_uploadModel = false, m_uploadMaterial = false;
	size_t m_offset = 0ull, m_count = 0ull;
	GLuint m_materialID = 0u;
	unsigned int m_skin = 0u;
	float m_radius = 1.0f;
	glm::vec3 m_position = glm::vec3(0.0f);
};

struct Skeleton_Component : public ECSComponent<Skeleton_Component> {
	Shared_Mesh m_mesh;
	int m_animation = -1;
	bool m_playAnim = true;
	float m_animTime = 0, m_animStart = 0;
	std::vector<glm::mat4> m_transforms;
};

#endif // GRAPHICS_GEOMETRY_COMPONENTS_H
