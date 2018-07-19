#pragma once
#ifndef COMPONENT_H
#define COMPONENT_H

#include "ECS\ECSmessage.h"
#include "Utilities\MappedChar.h"
#include "glm\glm.hpp"
#include <utility>
#include <functional>


class Engine;
class Component_Factory;

/**
 * A base class which is extend-able to create a specific component type.
 * Created by the component factory.
 **/
class Component
{
public:
	/** Returns the name of this component class. */
	virtual const char * getName() const {	return "Component";	}
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
	/** Constructor. */
	Component(Engine * engine) : m_engine(engine) {};


	// Protected Attributes
	Engine * m_engine;
	MappedChar<std::function<void(const ECS_Command&)>> m_commandMap;
	friend class Component_Factory;
};
#endif // COMPONENT_H