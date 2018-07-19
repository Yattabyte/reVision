#pragma once
#ifndef ENTITY_H
#define ENTITY_H

#include "ECS\Components\Component.h"
#include "Utilities\MappedChar.h"


class Entity {
protected:
	// Protected (de)Constructors
	~Entity() {}
	Entity(const unsigned int & count, const char** types, Component ** components) {
		for (unsigned int x = 0; x < count; ++x)
			m_components[types[x]].push_back(components[x]);
	}


	// Protected Methods
	/** Add a component to the entity.
	 * @param	<Component_Type>	the class of component to add
	 * @param	component			the component to add */
	template<typename Component_Type>
	inline void addComponent(Component * component) {
		m_components[Component_Type::GetName()] = component;
	}
	/** Remove a component from the entity.
	 * @param	<Component_Type>	the class of component to remove
	 * @param	component			the component to remove */
	template<typename Component_Type>
	inline void removeComponent(Component * component) {
		const char * componentName = Component_Type::GetName();
		if (!component || !m_components.find(componentName))
			return;
		auto & vector = m_components[componentName];
		vector.erase(std::remove_if(begin(vector), end(vector), [&component](Component * entityComponent) {
			return (entityComponent == component);
		}), end(vector));
	}


	// Protected Attributes
	VectorMap<Component*> m_components;
	friend class ECS;
};

#endif // ENTITY_H