#pragma once
#ifndef ECS_SYSTEM_H
#define ECS_SYSTEM_H

#include "Modules/ECS/ecsComponent.h"
#include <vector>


/** An interface for an ecsSystem. */
class ecsBaseSystem {
public:
	// Public enumerations
	/** Component flag types. */
	enum class RequirementsFlag : unsigned int {
		FLAG_REQUIRED = 0,
		FLAG_OPTIONAL = 1
	};


	// Public (De)Constructors
	/** Virtual system destructor. */
	inline virtual ~ecsBaseSystem() noexcept = default;
	/** Construct a base ecsSystem. */
	inline ecsBaseSystem() noexcept = default;
	/** Move an base system. */
	inline ecsBaseSystem(ecsBaseSystem&&) noexcept = default;
	/** Copy an base system. */
	inline ecsBaseSystem(const ecsBaseSystem&) noexcept = default;


	// Public Methods
	/** Retrieves the component types supported by this system.
	@return		the component types supported by this system. */
	std::vector<std::pair<ComponentID, RequirementsFlag>> getComponentTypes() const;
	/** Retrieves whether or not this system is valid (has at least 1 non-optional component type).
	@return		true if the system is valid, false otherwise. */
	bool isValid() const noexcept;


	// Public Interface
	/** Tick this system by deltaTime, passing in all the components matching this system's requirements.
	@param	deltaTime		the amount of time which passed since last update
	@param	components		the components to update. */
	virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) = 0;


protected:
	// Protected Methods
	/** Add a component type to be used by this system.
	@param	componentType	the type of component to use
	@param	componentFlag	flag indicating required/optional */
	void addComponentType(const ComponentID& componentType, const RequirementsFlag& componentFlag = RequirementsFlag::FLAG_REQUIRED);


private:
	// Private attributes
	std::vector<std::pair<ComponentID, RequirementsFlag>> m_componentTypes;
};

/** An ordered list of systems to be updated. */
class ecsSystemList {
public:
	// Public (De)Constructors
	inline explicit ecsSystemList() noexcept = default;


	// Public Methods
	/** Generate a system with a specific type and input arguments.
	@param	<T>		the system class type.
	@param	...Args	variadic arguments to forward to the system constructor. */
	template <typename T, class...Args>
	inline void makeSystem(Args & ...args) noexcept {
		if (const auto& system = std::make_shared<T>(args...); system->isValid())
			m_systems.push_back(system);
	}
	/** Adds a system to the list.
	@param	system	the system to add. */
	bool addSystem(const std::shared_ptr<ecsBaseSystem>& system);
	/** Removes a system from the list.
	@param	system	the system to remove.
	@return			true if successfully removed, false otherwise. */
	bool removeSystem(const std::shared_ptr<ecsBaseSystem>& system);
	/** Retrieve the number of systems in the list.
	@return			the size of the list. */
	size_t size() const noexcept;
	/** Retrieve a specific system at a given index.
	@param	index	the index to fetch the system from.
	@return			the system found at the index specified. */
	std::shared_ptr<ecsBaseSystem> operator[](const size_t& index) const noexcept;
	/** Retrieve an iterator to the beginning of this system list.
	@return			 an iterator to the beginning of this system list. */
	std::vector<std::shared_ptr<ecsBaseSystem>>::iterator begin() noexcept;
	/** Retrieve a const iterator to the beginning of this system list.
	@return			a const iterator to the beginning of this system list. */
	std::vector<std::shared_ptr<ecsBaseSystem>>::const_iterator begin() const noexcept;
	/** Retrieve an iterator to the end of this system list.
	@return			an iterator to the end of this system list. */
	std::vector<std::shared_ptr<ecsBaseSystem>>::iterator end() noexcept;
	/** Retrieve a const iterator to the end of this system list.
	@return			a const iterator to the end of this system list. */
	std::vector<std::shared_ptr<ecsBaseSystem>>::const_iterator end() const noexcept;


private:
	// Private attributes
	std::vector<std::shared_ptr<ecsBaseSystem>> m_systems;
};

#endif // ECS_SYSTEM_H