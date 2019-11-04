#pragma once
#ifndef ECS_COMPONENT_TYPES_H
#define ECS_COMPONENT_TYPES_H

#include "Assets/Mesh.h"
#include "Assets/Model.h"
#include "Assets/Collider.h"
#include "Modules/ECS/ecsComponent.h"
#include "Modules/Graphics/Common/Viewport.h"
#include "Modules/Graphics/Common/Camera.h"
#include "Utilities/Transform.h"
#include "Utilities/GL/GL_Vector.h"
#include "glm/glm.hpp"
#include <memory>


constexpr static const char selectedComponentName[] = "Selected_Component";
struct Selected_Component final : public ecsComponent<Selected_Component, selectedComponentName> {
};

constexpr static const char transformName[] = "Transform_Component";
struct Transform_Component final : public ecsComponent<Transform_Component, transformName> {
	Transform m_localTransform, m_worldTransform;
};

constexpr static const char playerSpawnName[] = "PlayerSpawn_Component";
struct PlayerSpawn_Component final : public ecsComponent<PlayerSpawn_Component, playerSpawnName> {
};

constexpr static const char player3DName[] = "Player3D_Component";
struct Player3D_Component final : public ecsComponent<Player3D_Component, player3DName> {
	glm::vec3 m_rotation = glm::vec3(0.0f);
};

constexpr static const char cameraName[] = "Camera_Component";
struct Camera_Component final : public ecsComponent<Camera_Component, cameraName> {
	Camera m_camera;
	float m_updateTime = 0.0f;

	inline std::vector<char> serialize() {
		std::vector<char> data(sizeof(Camera));
		std::memcpy(&data[0], &m_camera, sizeof(Camera));
		return data;
	}
	inline void deserialize(const char* data) {
		std::memcpy(&m_camera, &data[0], sizeof(Camera));
	}
};

constexpr static const char boundingSphereName[] = "BoundingSphere_Component";
struct [[deprecated]] BoundingSphere_Component final : public ecsComponent<BoundingSphere_Component, boundingSphereName>{
	glm::vec3 m_positionOffset = glm::vec3(0.0f);
	float m_radius = 1.0f;
	enum CameraCollision {
		OUTSIDE, INSIDE
	} m_cameraCollision = OUTSIDE;
};

constexpr static const char boundingBoxName[] = "BoundingBox_Component";
struct [[deprecated]] BoundingBox_Component final : public ecsComponent<BoundingBox_Component, boundingBoxName>{
	glm::vec3 m_positionOffset = glm::vec3(0.0f), m_extent = glm::vec3(0), m_min = glm::vec3(0.0f), m_max = glm::vec3(0.0f);
	enum CameraCollision {
		OUTSIDE, INSIDE
	} m_cameraCollision = OUTSIDE;
};

constexpr static const char propName[] = "Prop_Component";
struct Prop_Component final : public ecsComponent<Prop_Component, propName> {
	// Serialized Attributes
	std::string m_modelName;
	unsigned int m_skin = 0u;

	// Derived Attributes
	Shared_Model m_model;
	float m_radius = 1.0f;
	glm::vec3 m_position = glm::vec3(0.0f);
	bool m_uploadModel = false, m_uploadMaterial = false;
	size_t m_offset = 0ull, m_count = 0ull;
	GLuint m_materialID = 0u;

	inline std::vector<char> serialize() {
		const size_t propSize = sizeof(unsigned int) + (m_modelName.size() * sizeof(char)) + // need to store size + chars
			sizeof(unsigned int) + sizeof(float) + sizeof(glm::vec3);
		std::vector<char> data(propSize);
		void* ptr = &data[0];

		const auto nameCount = (int)m_modelName.size();
		std::memcpy(ptr, &nameCount, sizeof(int));
		ptr = static_cast<char*>(ptr) + sizeof(int);
		std::memcpy(ptr, m_modelName.data(), m_modelName.size());
		ptr = static_cast<char*>(ptr) + m_modelName.size();

		std::memcpy(ptr, &m_skin, sizeof(unsigned int));
		ptr = static_cast<char*>(ptr) + sizeof(unsigned int);
		std::memcpy(ptr, &m_radius, sizeof(float));
		ptr = static_cast<char*>(ptr) + sizeof(float);
		std::memcpy(ptr, &m_position, sizeof(glm::vec3));
		ptr = static_cast<char*>(ptr) + sizeof(glm::vec3);

		return data;
	}
	inline void deserialize(const char* data) {
		// Want a pointer that I can increment, promise to not change underlying data
		auto ptr = const_cast<char*>(&data[0]);

		int nameCount(0ull);
		std::memcpy(&nameCount, ptr, sizeof(int));
		ptr = static_cast<char*>(ptr) + sizeof(int);
		char* modelName = new char[size_t(nameCount) + 1ull];
		std::fill(&modelName[0], &modelName[nameCount + 1], '\0');
		std::memcpy(&modelName[0], ptr, nameCount);
		ptr = static_cast<char*>(ptr) + (size_t)nameCount;
		m_modelName = std::string(modelName);
		delete[] modelName;

		std::memcpy(&m_skin, ptr, sizeof(unsigned int));
		ptr = static_cast<char*>(ptr) + sizeof(unsigned int);
		std::memcpy(&m_radius, ptr, sizeof(float));
		ptr = static_cast<char*>(ptr) + sizeof(float);
		std::memcpy(&m_position, ptr, sizeof(glm::vec3));
		ptr = static_cast<char*>(ptr) + sizeof(glm::vec3);
	}
};

constexpr static const char skeletonName[] = "Skeleton_Component";
struct Skeleton_Component final : public ecsComponent<Skeleton_Component, skeletonName> {
	// Serialized Attributes
	std::string m_modelName;
	int m_animation = -1;
	bool m_playAnim = true;

	// Derived Attributes
	Shared_Mesh m_mesh;
	float m_animTime = 0, m_animStart = 0;
	std::vector<glm::mat4> m_transforms;

	inline std::vector<char> serialize() {
		const size_t propSize = sizeof(unsigned int) + (m_modelName.size() * sizeof(char)) + // need to store size + chars
			sizeof(int) + sizeof(bool);
		std::vector<char> data(propSize);
		void* ptr = &data[0];

		const auto nameCount = (int)m_modelName.size();
		std::memcpy(ptr, &nameCount, sizeof(unsigned int));
		ptr = static_cast<char*>(ptr) + sizeof(unsigned int);
		std::memcpy(ptr, m_modelName.data(), m_modelName.size());
		ptr = static_cast<char*>(ptr) + m_modelName.size();

		std::memcpy(ptr, &m_animation, sizeof(int));
		ptr = static_cast<char*>(ptr) + sizeof(int);
		std::memcpy(ptr, &m_playAnim, sizeof(bool));
		ptr = static_cast<char*>(ptr) + sizeof(bool);

		return data;
	}
	inline void deserialize(const char* data) {
		// Want a pointer that I can increment, promise to not change underlying data
		auto ptr = const_cast<char*>(&data[0]);

		int nameCount(0ull);
		std::memcpy(&nameCount, ptr, sizeof(int));
		ptr = static_cast<char*>(ptr) + sizeof(int);
		char* modelName = new char[size_t(nameCount) + 1ull];
		std::fill(&modelName[0], &modelName[nameCount + 1], '\0');
		std::memcpy(&modelName[0], ptr, nameCount);
		ptr = static_cast<char*>(ptr) + (size_t)nameCount;
		m_modelName = std::string(modelName);
		delete[] modelName;

		std::memcpy(&m_animation, ptr, sizeof(int));
		ptr = static_cast<char*>(ptr) + sizeof(int);
		std::memcpy(&m_playAnim, ptr, sizeof(bool));
		ptr = static_cast<char*>(ptr) + sizeof(bool);
	}
};

constexpr static const char shadowName[] = "Shadow_Component";
struct Shadow_Component final : public ecsComponent<Shadow_Component, shadowName> {
	int m_shadowSpot = -1;
	std::vector<Camera> m_cameras;
	std::vector<float> m_updateTimes;

	inline std::vector<char> serialize() {
		return {};
	}
	inline void deserialize(const char*) {
		m_cameras = {};
	}
};

constexpr static const char lightName[] = "Light_Component";
struct Light_Component final : public ecsComponent<Light_Component, lightName> {
	enum class Light_Type : int {
		DIRECTIONAL, POINT, SPOT
	} m_type = Light_Type::DIRECTIONAL;
	glm::vec3 m_color = glm::vec3(1.0f);
	float m_intensity = 1.0f;
	float m_radius = 1.0f;
	float m_cutoff = 45.0f;
};

constexpr static const char reflectorName[] = "Reflector_Component";
struct Reflector_Component final : public ecsComponent<Reflector_Component, reflectorName> {
	std::vector<Camera> m_cameras;
	std::vector<float> m_updateTimes;
	float m_updateTime = 0.0f;
	int m_cubeSpot = -1;

	inline std::vector<char> serialize() {
		return {};
	}
	inline void deserialize(const char*) {
		m_cameras = {};
	}
};

constexpr static const char colliderName[] = "Collider_Component";
struct Collider_Component final : public ecsComponent<Collider_Component, colliderName> {
	float m_restitution = 1.0f;
	float m_friction = 1.0f;
	btScalar m_mass = btScalar(0);
	Shared_Collider m_collider;
	btDefaultMotionState* m_motionState = nullptr;
	btRigidBody* m_rigidBody = nullptr;
	btConvexHullShape* m_shape = nullptr;
	Transform m_worldTransform;

	inline std::vector<char> serialize() {
		/**@todo*/
		return {};
	}
	inline void deserialize(const char*) {
		/**@todo*/
	}
};

#endif // ECS_COMPONENT_TYPES_H