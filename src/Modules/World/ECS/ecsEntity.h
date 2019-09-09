#pragma once
#ifndef ECSENTITY_H
#define ECSENTITY_H

#include <string>
#include <vector>

struct ecsHandle {
	char uuid[32] = { '\0' };
	/** Default Constructor. */
	ecsHandle() = default;
	/** E/xplicit Constructor. */
	explicit ecsHandle(const char id[32]) {
		std::memcpy(uuid, id, size_t(sizeof(char) * 32));
	}
	/** Copy Constructor. */
	ecsHandle(const ecsHandle& other) {
		std::memcpy(uuid, other.uuid, size_t(sizeof(char) * 32));
	}
	/** Move Constructor. */
	ecsHandle(ecsHandle&& other) noexcept {
		std::move(std::begin(other.uuid), std::end(other.uuid), uuid);
	}
	// Copy from another handle
	inline ecsHandle& operator=(const ecsHandle& other) noexcept {
		std::memcpy(uuid, other.uuid, size_t(sizeof(char) * 32));
		return *this;
	}
	// Compare against another handle
	inline bool operator==(const ecsHandle& other) const {
		return bool(std::strncmp(uuid, other.uuid, 32ull) == 0);
	}
};

/** An entity in the ECS paradigm - holds components and heirarchical information. */
struct ecsEntity {
	// A non-unique name used primarily for level editing and debugging
	std::string m_name = "Entity";
	// A universally unique identifier
	ecsHandle m_uuid;
	// The index this entity is found at in the entity vector
	int m_entityIndex = -1;
	// The list of component raw-types held by this entity. First of pair is the component ID, second is id to the create function
	std::vector<std::pair<int, int>> m_components = {};
	// An optional parent for this entity, used when forming larger mega-entities
	ecsEntity* m_parent = nullptr;
	// An optional set of children for this entity, whom this entity will be the parent of
	std::vector<ecsEntity*> m_children;
};

#endif // ECSENTITY_H