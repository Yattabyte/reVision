#pragma once
#ifndef ECS_SYSTEM_H
#define ECS_SYSTEM_H

#include "Modules/ECS/ecsComponent.h"
#include <memory>


/** An interface for an ecs system. */
class ecsBaseSystem {
public:
	// Public enumerations
	/** Component flag types. */
	const enum RequirementsFlag {
		FLAG_REQUIRED = 0,
		FLAG_OPTIONAL = 1
	};


	// Public (de)Constructors
	inline virtual ~ecsBaseSystem() = default;
	inline ecsBaseSystem() = default;


	// Public Methods
	/** Returns the component types supported by this system.
	@return		the component types supported by this system. */
	inline const std::vector<ComponentID>& getComponentTypes() {
		return componentTypes;
	};
	/** Returns the component flags requested by this system.
	@return		the component flags requested by this system. */
	inline const std::vector<RequirementsFlag>& getComponentFlags() {
		return componentFlags;
	};
	/** Returns whether or not this system is valid (has at least 1 non-optional component type)
	@return		true if the system is valid, false otherwise. */
	inline const bool isValid() const {
		for each (const auto& flag in componentFlags)
			if ((flag & FLAG_OPTIONAL) == 0)
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
		componentTypes.push_back(componentType);
		componentFlags.push_back(componentFlag);
	}


private:
	//private attributes
	std::vector<ComponentID> componentTypes;
	std::vector<RequirementsFlag> componentFlags;
};

/** An ordered list of systems to be updated. */
class ecsSystemList {
public:
	// Public (de)Constructors
	inline ~ecsSystemList() = default;
	inline explicit ecsSystemList() = default;
	inline ecsSystemList(const std::vector<std::shared_ptr<ecsBaseSystem>>& systems)
		: m_systems(systems) {}


	// Public Methods
	/** Generate a system with a specifc type and input arguments.
	@param	<T>		the system class type.
	@param	...Args	variadic arguments to forward to the system constructor. */
	template <typename T, class...Args>
	inline const bool makeSystem(Args ...args) {
		const auto& system = std::make_shared<T>(args...);
		if (!system->isValid())
			return false;
		m_systems.push_back(system);
		return true;
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
	inline auto operator[](const size_t& index) {
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
		return  m_systems.cend();
	}


private:
	// Private attributes
	std::vector<std::shared_ptr<ecsBaseSystem>> m_systems;
};

#endif // ECS_SYSTEM_H