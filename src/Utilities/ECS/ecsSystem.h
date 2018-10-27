#pragma once
#ifndef ECSSYSTEM_H
#define ECSSYSTEM_H

#include "Utilities\ECS\ecsComponent.h"


/** An interface for ECS Systems. */
class BaseECSSystem {
public:
	// Public enumerations
	// Component flag types
	enum {
		FLAG_REQUIRED = 0,
		FLAG_OPTIONAL = 1
	};


	// Public (de)Constructors
	~BaseECSSystem() = default;
	BaseECSSystem() = default;


	// Public Methods
	/** Returns the component types supported by this system.
	@return		the component types supported by this system. */
	inline const std::vector<uint32_t> & getComponentTypes() { 
		return componentTypes; 
	};
	/** Returns the component flags requested by this system.
	@return		the component flags requested by this system. */
	inline const std::vector<uint32_t> & getComponentFlags() {
		return componentFlags; 
	};
	/** Returns whether or not this system is valid (has at least 1 non-optional component type)
	@return		true if the system is valid, false otherwise. */
	inline const bool isValid() const {
		for (size_t i = 0; i < componentFlags.size(); ++i) 
			if ((componentFlags[i] & BaseECSSystem::FLAG_OPTIONAL) == 0)
				return true;		
		return false;
	}


	// Public Interface
	/** Tick this system by deltaTime, passing in all the components matching this system's requirements.
	@param	deltaTime		the amount of time which passed since last update
	@param	components		the components to update. */
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) = 0;


protected:
	// Protected Methods
	/** Add a component type to be used by this system. 
	@param	componentType	the type of component to use
	@param	componentFlag	flag indicating required/optional */
	inline void addComponentType(const uint32_t & componentType, const uint32_t & componentFlag = FLAG_REQUIRED) {
		componentTypes.push_back(componentType);
		componentFlags.push_back(componentFlag);
	}

	
private:
	//private attributes
	std::vector<uint32_t> componentTypes;
	std::vector<uint32_t> componentFlags;
};

/** An ordered list of systems to be updated. */
class ECSSystemList {
public:
	// Public Methods
	/** Adds a system to the list.
	@param	system	the system to add. */
	inline const bool addSystem(BaseECSSystem * system)	{
		if (!system->isValid())
			return false;
		m_systems.push_back(system);
		return true;
	}
	/** Removes a system from the list.
	@param	system	the system to remove. */
	inline const bool removeSystem(BaseECSSystem * system) {
		for (size_t i = 0; i < m_systems.size(); ++i)
			if (system == m_systems[i]) {
				m_systems.erase(m_systems.begin() + i);
				return true;
			}		
		return false;
	}
	/** Get the number of systems in the list.
	@return			the size of the list. */
	inline const size_t size() {
		return m_systems.size();
	}
	/** Retrieve a specific system at a given index.
	@param	index	the index to fetch the system from. */
	inline BaseECSSystem * operator[](const size_t & index) {
		return m_systems[index];
	}


private:
	// Private attributes
	std::vector<BaseECSSystem *> m_systems;
};

#endif // ECSSYSTEM_H