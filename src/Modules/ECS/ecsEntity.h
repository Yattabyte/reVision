#pragma once
#ifndef ECS_ENTITY_H
#define ECS_ENTITY_H

#include "Modules/ECS/ecsHandle.h"
#include "Modules/ECS/ecsComponent.h"
#include <map>
#include <string>
#include <vector>


// Definitions to make life easier
struct ecsEntity;
using EntityMap = std::map<EntityHandle, ecsEntity*>;

/** An entity in the ECS paradigm - holds components and hierarchical information. */
struct ecsEntity {
	// Public Attributes
	// A non-unique name used primarily for level editing and debugging
	std::string m_name = "Entity";
	// The index this entity is found at in the entity vector
	int m_entityIndex = -1;
	// The list of component raw-types held by this entity. First of pair is the component ID, second is id to the create function
	std::vector<std::tuple<ComponentID, int, ComponentHandle>> m_components = {};
	// An optional parent for this entity, used when forming larger mega-entities
	EntityHandle m_parent;
	// An optional set of children for this entity, whom this entity will be the parent of
	EntityMap m_children;
};

#endif // ECS_ENTITY_H