#pragma once
#ifndef COMPONENT_H
#define COMPONENT_H

#include "ECS\ECSmessage.h"
#include "Utilities\MappedChar.h"
#include "glm\glm.hpp"
#include <utility>
#include <functional>
#include <minmax.h>


class Engine;
class ECS;

/**
 * A base class which is extend-able to create a specific component type.
 * Created by the component factory.
 **/
class Component
{
public:
	/** Returns the name of this component class. */
	static const char * GetName() { return "Component"; }
	/** Sends a command to this component to execute.
	 * @param	command		the std::string command name
	 * @param	obj			any arguments needed */
	template <typename DATA_TYPE>
	void sendCommand(const char * command, const DATA_TYPE & obj) {
		// Run the command if it exists, and pass it the payload
		if (m_commandMap.find(command))
			m_commandMap[command](ECS_Command(obj));
	}
	/** Returns true if this component is loaded, false otherwise. */
	virtual bool isLoaded() const { return this ? true : false; }
	/** Tests if this object is within the viewing frustum of the camera.
	 * @brief				a test of general visibility (excluding obstruction of other objects).
	 * @param	radius		the radius of the camera
	 * @param	eyePosition	the viewing position of the camera
	 * @return				true if this object is within the viewing range of the camera, false otherwise */
	virtual bool isVisible(const float & radius, const glm::vec3 & eyePosition) const { return true; }
	/** Tests if this object can contain the 3D point specified.
	 * @param	point		the point to test */
	virtual bool containsPoint(const glm::vec3 & point) const { return false; }



protected:
	// (de)Constructors
	/** Virtual Destructor. */
	virtual ~Component() {};
	/** Constructor. 
	 * @param	engine	the engine */
	Component(Engine * engine) : m_engine(engine) {};


	// Protected Attributes
	Engine * m_engine;
	MappedChar<std::function<void(const ECS_Command&)>> m_commandMap;
	friend class ECS;
	friend class Component_Creator_Base;
};

struct ArgumentList {
	std::vector<const char*> dataTypes;
	std::vector<void*> dataPointers;
	~ArgumentList() {
		for each(void * dataPtr in dataPointers)
			delete dataPtr;
	}
	void pushData(const char * type, void  * const pointer) {
		dataTypes.push_back(type);
		dataPointers.push_back(pointer);
	}
};

struct Component_Creator_Base {

	virtual Component * create(Engine * engine, const ArgumentList & argumentList) { return nullptr; };
	static void destroy(Component * component) {
		// delete component; 
	}
};

template <typename type_C>
struct Component_Creator : public Component_Creator_Base {
	virtual Component * create(Engine * engine, const ArgumentList & argumentList) {
		// Check for a size mismatch
		const std::vector<const char*> & paramTypes = type_C::GetParamTypes();
		if (paramTypes.size() != argumentList.dataTypes.size() || paramTypes.size() != argumentList.dataPointers.size())
			return nullptr;

		// Ensure the argument list provided matches the component parameter types
		for (int paramIndex = 0; paramIndex < paramTypes.size(); ++paramIndex) {
			if (strcmp(paramTypes[paramIndex], argumentList.dataTypes[paramIndex]) != 0)
				return nullptr; // Something didn't match, return early
		}

		// Safe to create
		return new type_C(engine, argumentList);
	}
};


#endif // COMPONENT_H