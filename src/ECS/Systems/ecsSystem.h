#pragma once
#ifndef ECSSYSTEM_H
#define ECSSYSTEM_H

#include "ECS\Components\ecsComponent.h"


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
	BaseECSSystem() {};


	// Public Methods
	/** Returns the component types supported by this system.
	@return		the component types supported by this system. */
	const std::vector<unsigned int> & getComponentTypes() { return componentTypes; };
	/** Returns the component flags requested by this system.
	@return		the component flags requested by this system. */
	const std::vector<unsigned int> & getComponentFlags() { return componentFlags; };
	/** Returns whether or not this system is valid (has at least 1 non-optional component type)
	@return		true if the system is valid, false otherwise. */
	const bool isValid() const;


	// Public Interface
	/** Tick this system by deltaTime, passing in all the components matching this system's requirements.
	@param	deltaTime		the amount of time which passed since last update
	@param	components		the components to update. */
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) {};


protected:
	// Protected Methods
	/** Add a component type to be used by this system. 
	@param	componentType	the type of component to use
	@param	componentFlag	flag indicating required/optional */
	void addComponentType(const unsigned int & componentType, const unsigned int & componentFlag = FLAG_REQUIRED) {
		componentTypes.push_back(componentType);
		componentFlags.push_back(componentFlag);
	}

	
private:
	//private attributes
	std::vector<unsigned int> componentTypes;
	std::vector<unsigned int> componentFlags;
};

/** An ordered list of systems to be updated. */
class ECSSystemList {
public:
	// Public Methods
	/** Adds a system to the list.
	@param	system	the system to add. */
	inline const bool addSystem(BaseECSSystem * system)
	{
		if (!system->isValid())
			return false;
		systems.push_back(system);
		return true;
	}
	/** Removes a system from the list.
	@param	system	the system to remove. */
	const bool removeSystem(BaseECSSystem * system);
	/** Get the number of systems in the list.
	@return			the size of the list. */
	inline const size_t size() {
		return systems.size();
	}
	/** Retrieve a specific system at a given index.
	@param	index	the index to fetch the system from. */
	inline BaseECSSystem * operator[](const unsigned int & index) {
		return systems[index];
	}


private:
	// Private attributes
	std::vector<BaseECSSystem *> systems;
};

#endif // ECSSYSTEM_H