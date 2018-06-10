#pragma once
#ifndef REFLECTOR_H
#define REFLECTOR_H

#include "Systems\World\ECS\Entities\Entity.h"


/** A sun entity. */
class Reflector_Entity : public Entity
{
protected:
	// (de)Constructors
	/** Virtual Destructor. */
	virtual ~Reflector_Entity() {};
	/** Constructor. */
	Reflector_Entity() {
		m_commandMap["Change_Reflector_Radius"] = [&](const ECS_Command & payload) {changeRadius(payload); };
		m_commandMap["Change_Transform"] = [&](const ECS_Command & payload) {changeTransform(payload); };
	}


	// Protected Attributes
	Component * m_light;
	friend class Creator_Reflector;


private:
	// Private Functions
	// Forward Commands
	void changeRadius(const ECS_Command & payload) {
		if (payload.isType<float>())
			m_light->sendCommand("Set_Reflector_Radius", payload.toType<float>());
	}
	void changeTransform(const ECS_Command & payload) {
		if (payload.isType<Transform>())
			m_light->sendCommand("Set_Transform", payload.toType<Transform>());
	}
};

/**
 * Creates a Reflector entity, stores reflection information about what it sees around it
 **/
class Creator_Reflector : public EntityCreator
{
public:
	/** Constructor.
	 * @param	componentFactory	pointer to the component factory to allow creation of specific components */
	Creator_Reflector(Component_Factory * componentFactory) : EntityCreator(componentFactory) {}


	virtual void destroy(Entity * e) {
		Reflector_Entity * entity = (Reflector_Entity*)e;
		unMakeComponent(entity->m_light);
		delete entity;
	}
	virtual Entity* create() {
		Reflector_Entity *entity = new Reflector_Entity();
		entity->m_light = makeComponent("Reflector");
		return entity;
	}
};

#endif // REFLECTOR_H