#pragma once
#ifndef ECS_SYSTEM_H
#define ECS_SYSTEM_H

#include "Modules/ECS/ecsComponent.h"
#include <memory>


/** An interface for an ecsSystem. */
class ecsBaseSystem {
public:
	// Public enumerations
	/** Component flag types. */
	const enum RequirementsFlag {
		FLAG_REQUIRED = 0,
		FLAG_OPTIONAL = 1
	};


	// Public (De)Constructors
	inline virtual ~ecsBaseSystem() = default;
	inline ecsBaseSystem() = default;


	// Public Methods
	/** Returns the component types supported by this system.
	@return		the component types supported by this system. */
	inline const std::vector<std::pair<ComponentID, RequirementsFlag>>& getComponentTypes() const {
		return m_componentTypes;
	};
	/** Returns whether or not this system is valid (has at least 1 non-optional component type)
	@return		true if the system is valid, false otherwise. */
	inline const bool isValid() const {
		for (const auto& [componentID, componentFlag] : m_componentTypes)
			if ((componentFlag & FLAG_OPTIONAL) == 0)
				return true;
		return false;
	}


	// Public Interface
	/** Tick this system by deltaTime, passing in all the components matching this system's requirements.
	@param	deltaTime		the amount of time which passed since last update
	@param	components		the components to update. */
	virtual void updateComponents(const float& deltaTime, const std::vector< std::vector<ecsBaseComponent*> >& components) = 0;


protected:
	// Protected Methods
	/** Add a component type to be used by this system.
	@param	componentType	the type of component to use
	@param	componentFlag	flag indicating required/optional */
	inline void addComponentType(const ComponentID& componentType, const RequirementsFlag& componentFlag = FLAG_REQUIRED) {
		m_componentTypes.push_back({ componentType, componentFlag });
	}


private:
	//private attributes
	std::vector<std::pair<ComponentID, RequirementsFlag>> m_componentTypes;
};

/** An ordered list of systems to be updated. */
class ecsSystemList {
public:
	// Public (De)Constructors
	inline explicit ecsSystemList() = default;


	// Public Methods
	/** Generate a system with a specific type and input arguments.
	@param	<T>		the system class type.
	@param	...Args	variadic arguments to forward to the system constructor. */
	template <typename T, class...Args>
	[[maybe_unused]] inline std::shared_ptr<T> makeSystem(Args ...args) {
		const auto& system = std::make_shared<T>(args...);
		if (!system->isValid())
			return {};
		m_systems.push_back(system);
		return system;
	}
	/** Adds a system to the list.
	@param	system	the system to add. */
	inline const bool addSystem(const std::shared_ptr<ecsBaseSystem>& system) {
		if (!system->isValid())
			return false;
		m_systems.push_back(system);
		return true;
	}
	/** Removes a system from the list.
	@param	system	the system to remove. */
	inline const bool removeSystem(const std::shared_ptr<ecsBaseSystem>& system) {
		for (size_t i = 0; i < m_systems.size(); ++i)
			if (system.get() == m_systems[i].get()) {
				m_systems.erase(m_systems.begin() + i);
				return true;
			}
		return false;
	}
	/** Get the number of systems in the list.
	@return			the size of the list. */
	inline const size_t size() const {
		return m_systems.size();
	}
	/** Retrieve a specific system at a given index.
	@param	index	the index to fetch the system from. */
	inline auto operator[](const size_t& index) const {
		return m_systems[index];
	}
	/** Retrieve an iterator to the beginning of this system list.
	@return			 an iterator to the beginning of this system list. */
	inline auto begin() {
		return m_systems.begin();
	}
	/** Retrieve a const iterator to the beginning of this system list.
	@return			a const iterator to the beginning of this system list. */
	inline auto begin() const {
		return m_systems.cbegin();
	}
	/** Retrieve an iterator to the end of this system list.
	@return			an iterator to the end of this system list. */
	inline auto end() {
		return m_systems.end();
	}
	/** Retrieve a const iterator to the end of this system list.
	@return			a const iterator to the end of this system list. */
	inline auto end() const {
		return m_systems.cend();
	}


private:
	// Private attributes
	std::vector<std::shared_ptr<ecsBaseSystem>> m_systems;
};

#endif // ECS_SYSTEM_H