#pragma once
#ifndef ECS_COMPONENT_TYPES_H
#define ECS_COMPONENT_TYPES_H

#include "Assets/Mesh.h"
#include "Assets/Model.h"
#include "Assets/Collider.h"
#include "Modules/ECS/ecsComponent.h"
#include "Modules/Graphics/Common/Camera.h"
#include "Utilities/Transform.h"
#include "Utilities/IO/Serializer.h"
#include "glm/glm.hpp"
#include <btBulletDynamicsCommon.h>


constexpr static const char selectedComponentName[] = "Selected_Component";
struct Selected_Component final : public ecsComponent<Selected_Component, selectedComponentName> {
};

constexpr static const char transformName[] = "Transform_Component";
struct Transform_Component final : public ecsComponent<Transform_Component, transformName> {
	Transform m_localTransform, m_worldTransform;

	inline std::vector<char> serialize() noexcept {
		return Serializer::Serialize_Set(std::pair("m_localTransform", m_localTransform), std::pair("m_worldTransform", m_worldTransform));
	}
	inline void deserialize(const std::vector<char>& data) noexcept {
		Serializer::Deserialize_Set(data, std::pair("m_localTransform", &m_localTransform), std::pair("m_worldTransform", &m_worldTransform));
	}
};

constexpr static const char playerSpawnName[] = "PlayerSpawn_Component";
struct PlayerSpawn_Component final : public ecsComponent<PlayerSpawn_Component, playerSpawnName> {
};

constexpr static const char player3DName[] = "Player3D_Component";
struct Player3D_Component final : public ecsComponent<Player3D_Component, player3DName> {
	glm::vec3 m_rotation = glm::vec3(0.0f);

	inline std::vector<char> serialize() noexcept {
		return Serializer::Serialize_Set(std::pair("m_rotation", m_rotation));
	}
	inline void deserialize(const std::vector<char>& data) noexcept {
		Serializer::Deserialize_Set(data, std::pair("m_rotation", &m_rotation));
	}
};

constexpr static const char cameraName[] = "Camera_Component";
struct Camera_Component final : public ecsComponent<Camera_Component, cameraName> {
	Camera m_camera;
	float m_updateTime = 0.0f;

	inline std::vector<char> serialize() noexcept {
		return Serializer::Serialize_Set(std::pair("m_camera", m_camera));
	}
	inline void deserialize(const std::vector<char>& data) noexcept {
		Serializer::Deserialize_Set(data, std::pair("m_camera", &m_camera));
	}
};

constexpr static const char boundingSphereName[] = "BoundingSphere_Component";
struct [[deprecated]] BoundingSphere_Component final : public ecsComponent<BoundingSphere_Component, boundingSphereName>{
	glm::vec3 m_positionOffset = glm::vec3(0.0f);
	float m_radius = 1.0f;
	enum class CameraCollision {
		OUTSIDE, INSIDE
	} m_cameraCollision = CameraCollision::OUTSIDE;

	inline std::vector<char> serialize() noexcept {
		return Serializer::Serialize_Set(
			std::pair("m_positionOffset", m_positionOffset),
			std::pair("m_radius", m_radius),
			std::pair("m_cameraCollision", m_cameraCollision)
		);
	}
	inline void deserialize(const std::vector<char>& data) noexcept {
		Serializer::Deserialize_Set(data,
			std::pair("m_positionOffset", &m_positionOffset),
			std::pair("m_radius", &m_radius),
			std::pair("m_cameraCollision", &m_cameraCollision)
		);
	}
};

constexpr static const char boundingBoxName[] = "BoundingBox_Component";
struct [[deprecated]] BoundingBox_Component final : public ecsComponent<BoundingBox_Component, boundingBoxName>{
	glm::vec3 m_positionOffset = glm::vec3(0.0f), m_extent = glm::vec3(0), m_min = glm::vec3(0.0f), m_max = glm::vec3(0.0f);
	enum class CameraCollision {
		OUTSIDE, INSIDE
	} m_cameraCollision = CameraCollision::OUTSIDE;

	inline std::vector<char> serialize() noexcept {
		return Serializer::Serialize_Set(
			std::pair("m_positionOffset", m_positionOffset),
			std::pair("m_extent", m_extent),
			std::pair("m_min", m_min),
			std::pair("m_max", m_max),
			std::pair("m_cameraCollision", m_cameraCollision)
		);
	}
	inline void deserialize(const std::vector<char>& data) noexcept {
		Serializer::Deserialize_Set(data,
			std::pair("m_positionOffset", &m_positionOffset),
			std::pair("m_extent", &m_extent),
			std::pair("m_min", &m_min),
			std::pair("m_max", &m_max),
			std::pair("m_cameraCollision", &m_cameraCollision)
		);
	}
};

constexpr static const char colliderName[] = "Collider_Component";
struct Collider_Component final : public ecsComponent<Collider_Component, colliderName> {
	// Serialized Attributes
	std::string m_modelName;
	float m_restitution = 1.0f;
	float m_friction = 1.0f;
	btScalar m_mass = 0;

	// Derived Attributes
	Transform m_worldTransform;
	Shared_Collider m_collider;
	std::shared_ptr<btDefaultMotionState> m_motionState = nullptr;
	std::shared_ptr<btRigidBody> m_rigidBody = nullptr;
	std::shared_ptr<btCollisionShape> m_shape = nullptr;

	inline std::vector<char> serialize() noexcept {
		return Serializer::Serialize_Set(
			std::pair("m_modelName", m_modelName),
			std::pair("m_restitution", m_restitution),
			std::pair("m_friction", m_friction),
			std::pair("m_mass", m_mass)
		);
	}
	inline void deserialize(const std::vector<char>& data) noexcept {
		Serializer::Deserialize_Set(data,
			std::pair("m_modelName", &m_modelName),
			std::pair("m_restitution", &m_restitution),
			std::pair("m_friction", &m_friction),
			std::pair("m_mass", &m_mass)
		);
	}
};

constexpr static const char propName[] = "Prop_Component";
struct Prop_Component final : public ecsComponent<Prop_Component, propName> {
	// Serialized Attributes
	std::string m_modelName;
	unsigned int m_skin = 0u;

	// Derived Attributes
	Shared_Model m_model;
	bool m_uploadModel = false, m_uploadMaterial = false;
	size_t m_offset = 0ull, m_count = 0ull;
	GLuint m_materialID = 0u;

	inline std::vector<char> serialize() noexcept {
		return Serializer::Serialize_Set(
			std::pair("m_modelName", m_modelName),
			std::pair("m_skin", m_skin)
		);
	}
	inline void deserialize(const std::vector<char>& data) noexcept {
		Serializer::Deserialize_Set(data,
			std::pair("m_modelName", &m_modelName),
			std::pair("m_skin", &m_skin)
		);
	}
};

constexpr static const char skeletonName[] = "Skeleton_Component";
struct Skeleton_Component final : public ecsComponent<Skeleton_Component, skeletonName> {
	// Serialized Attributes
	int m_animation = 0;
	bool m_playAnim = true;

	// Derived Attributes
	Shared_Mesh m_mesh;
	float m_animTime = 0, m_animStart = 0;
	std::vector<glm::mat4> m_transforms;

	inline std::vector<char> serialize() noexcept {
		return Serializer::Serialize_Set(
			std::pair("m_animation", m_animation),
			std::pair("m_playAnim", m_playAnim)
		);
	}
	inline void deserialize(const std::vector<char>& data) noexcept {
		Serializer::Deserialize_Set(data,
			std::pair("m_animation", &m_animation),
			std::pair("m_playAnim", &m_playAnim)
		);
	}
};

constexpr static const char shadowName[] = "Shadow_Component";
struct Shadow_Component final : public ecsComponent<Shadow_Component, shadowName> {
	int m_shadowSpot = -1;
	std::vector<Camera> m_cameras;
	std::vector<float> m_updateTimes;
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

	inline std::vector<char> serialize() noexcept {
		return Serializer::Serialize_Set(
			std::pair("m_type", m_type),
			std::pair("m_color", m_color),
			std::pair("m_intensity", m_intensity),
			std::pair("m_radius", m_radius),
			std::pair("m_cutoff", m_cutoff)
		);
	}
	inline void deserialize(const std::vector<char>& data) noexcept {
		Serializer::Deserialize_Set(data,
			std::pair("m_type", &m_type),
			std::pair("m_color", &m_color),
			std::pair("m_intensity", &m_intensity),
			std::pair("m_radius", &m_radius),
			std::pair("m_cutoff", &m_cutoff)
		);
	}
};

constexpr static const char reflectorName[] = "Reflector_Component";
struct Reflector_Component final : public ecsComponent<Reflector_Component, reflectorName> {
	std::vector<Camera> m_cameras;
	std::vector<float> m_updateTimes;
	float m_updateTime = 0.0f;
	int m_cubeSpot = -1;
};

#endif // ECS_COMPONENT_TYPES_H