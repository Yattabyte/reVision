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
#include "Utilities/GL/GL_ArrayBuffer.h"
#include "glm/glm.hpp"
#include <memory>


constexpr static const char selectedComponentName[] = "Selected_Component";
struct Selected_Component final : public ecsComponent<Selected_Component, selectedComponentName> {
};

constexpr static const char transformName[] = "Transform_Component";
struct Transform_Component final : public ecsComponent<Transform_Component, transformName> {
	// Default Serialization
	Transform m_localTransform, m_worldTransform;
};

constexpr static const char playerSpawnName[] = "PlayerSpawn_Component";
struct PlayerSpawn_Component final : public ecsComponent<PlayerSpawn_Component, playerSpawnName> {
	// Default Serialization
};

constexpr static const char player3DName[] = "Player3D_Component";
struct Player3D_Component final : public ecsComponent<Player3D_Component, player3DName> {
	// Default Serialization
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

constexpr static const char cameraArrayName[] = "CameraArray_Component";
struct CameraArray_Component final : public ecsComponent<CameraArray_Component, cameraArrayName> {
	std::vector<Camera> m_cameras;
	std::vector<float> m_updateTimes;

	inline std::vector<char> serialize() {
		/**@todo copy out cameras*/
		return {};
	}
	inline void deserialize(const char* data) {
		m_cameras = std::vector<Camera>();
		/**@todo copy in cameras*/
	}
};

constexpr static const char boundingSphereName[] = "BoundingSphere_Component";
struct BoundingSphere_Component final : public ecsComponent<BoundingSphere_Component, boundingSphereName> {
	// Default Serialization
	glm::vec3 m_positionOffset = glm::vec3(0.0f);
	float m_radius = 1.0f;
	enum CameraCollision {
		OUTSIDE, INSIDE
	} m_cameraCollision = OUTSIDE;
};

constexpr static const char boundingBoxName[] = "BoundingBox_Component";
struct BoundingBox_Component final : public ecsComponent<BoundingBox_Component, boundingBoxName> {
	// Default Serialization
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
		void * ptr = &data[0];

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
		char * modelName = new char[size_t(nameCount) + 1ull];
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
		void * ptr = &data[0];

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
		char * modelName = new char[size_t(nameCount) + 1ull];
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

	inline std::vector<char> serialize() {
		return {};
	}
	inline void deserialize(const char* data) {
	}
};

constexpr static const char lightColorName[] = "LightColor_Component";
struct LightColor_Component final : public ecsComponent<LightColor_Component, lightColorName> {
	// Default Serialization
	glm::vec3 m_color = glm::vec3(1.0f);
	float m_intensity = 1.0f;
};

constexpr static const char lightRadiusName[] = "LightRadius_Component";
struct LightRadius_Component final : public ecsComponent<LightRadius_Component, lightRadiusName> {
	// Default Serialization
	float m_radius = 1.0f;
};

constexpr static const char lightCutoffName[] = "LightCutoff_Component";
struct LightCutoff_Component final : public ecsComponent<LightCutoff_Component, lightCutoffName> {
	// Default Serialization
	float m_cutoff = 45.0f;
};

constexpr static const char lightDirectionalName[] = "LightDirectional_Component";
struct LightDirectional_Component final : public ecsComponent<LightDirectional_Component, lightDirectionalName> {
#define NUM_CASCADES 4
	// Default Serialization
	glm::mat4 m_pvMatrices[NUM_CASCADES] = { glm::mat4(1.0f),glm::mat4(1.0f), glm::mat4(1.0f), glm::mat4(1.0f) };
	float m_cascadeEnds[NUM_CASCADES] = { 1.0f, 1.0f, 1.0f, 1.0f };
};

constexpr static const char lightPointName[] = "LightPoint_Component";
struct LightPoint_Component final : public ecsComponent<LightPoint_Component, lightPointName> {
	// Default Serialization
};

constexpr static const char lightSpotName[] = "LightSpot_Component";
struct LightSpot_Component final : public ecsComponent<LightSpot_Component, lightSpotName> {
	// Default Serialization
};

constexpr static const char reflectorName[] = "Reflector_Component";
struct Reflector_Component final : public ecsComponent<Reflector_Component, reflectorName> {
	float m_updateTime = 0.0f;
	int m_cubeSpot = -1;

	inline std::vector<char> serialize() {
		return {};
	}
	inline void deserialize(const char* data) {
	}
};

constexpr static const char colliderName[] = "Collider_Component";
struct Collider_Component final : public ecsComponent<Collider_Component, colliderName> {	
	float m_restitution = 1.0f;
	float m_friction = 1.0f;
	btScalar m_mass = btScalar(0);
	Shared_Collider m_collider;
	btDefaultMotionState * m_motionState = nullptr;
	btRigidBody * m_rigidBody = nullptr;
	btConvexHullShape * m_shape = nullptr;
	Transform m_worldTransform;

	inline std::vector<char> serialize() {
		/**@todo*/
		return {};
	}
	inline void deserialize(const char* data) {
		/**@todo*/
	}
};

#endif // ECS_COMPONENT_TYPES_H