#pragma once
#ifndef PROPS_H
#define PROPS_H

#include "Systems\World\ECS\Entities\Entity.h"
#include "Utilities\Transform.h"


/** A prop entity. */
class Prop_Entity : public Entity
{
public:
	/** Returns true if this component is loaded, false otherwise. */
	virtual bool isLoaded() const {
		return m_prop->isLoaded();
	}


protected:
	// (de)Constructors
	~Prop_Entity() {}
	Prop_Entity() {
		m_commandMap["Load_Model"] = [&](const ECS_Command & payload) {loadModel(payload); };
		m_commandMap["Change_Skin"] = [&](const ECS_Command & payload) {changeSkin(payload); };
		m_commandMap["Change_Animation"] = [&](const ECS_Command & payload) {changeAnimation(payload); };
		m_commandMap["Change_Transform"] = [&](const ECS_Command & payload) {changeTransform(payload); };
	}


	// Protected Attributes
	Component * m_prop;
	friend class Creator_Prop;


private:
	// Private Functions
	// Forward Commands
	void loadModel(const ECS_Command & payload) {
		if (payload.isType<string>())
			m_prop->sendCommand("Set_Model_Directory", payload.toType<string>());
	}
	void changeSkin(const ECS_Command & payload) {
		if (payload.isType<int>())
			m_prop->sendCommand("Set_Skin", payload.toType<int>());
	}
	void changeAnimation(const ECS_Command & payload) {
		if (payload.isType<int>())
			m_prop->sendCommand("Set_Animation", payload.toType<int>());
	}
	void changeTransform(const ECS_Command & payload) {
		if (payload.isType<Transform>())
			m_prop->sendCommand("Set_Transform", payload.toType<Transform>());
	}
};

/**
 * Creates a Prop entity, composed of an animated model component
 **/
class Creator_Prop : public EntityCreator
{
public:
	/** Constructor.
	 * @param	componentFactory	pointer to the component factory to allow creation of specific components */
	Creator_Prop(Component_Factory * componentFactory) : EntityCreator(componentFactory) {}
	
	
	virtual void destroy(Entity * e) {
		Prop_Entity * entity = (Prop_Entity*)e;
		unMakeComponent(entity->m_prop);
		delete entity;
	}
	virtual Entity* create() {
		Prop_Entity *entity = new Prop_Entity();
		entity->m_prop = makeComponent("Anim_Model");
		return entity;
	}
};

/** A static prop entity. */
class Prop_Static_Entity : public Entity 
{
public:
	/** Returns true if this component is loaded, false otherwise. */
	virtual bool isLoaded() const {
		return m_prop->isLoaded();
	}


protected:
	// (de)Constructors
	~Prop_Static_Entity() {}
	Prop_Static_Entity() {
		m_commandMap["Load_Model"] = [&](const ECS_Command & payload) {loadModel(payload);};
		m_commandMap["Change_Skin"] = [&](const ECS_Command & payload) {changeSkin(payload); };
		m_commandMap["Change_Transform"] = [&](const ECS_Command & payload) {changeTransform(payload); };
	}


	// Protected Attributes
	Component * m_prop;
	friend class Creator_Prop_Static;


private:
	// Private Functions
	// Forward Commands
	void loadModel(const ECS_Command & payload) {
		if (payload.isType<string>())
			m_prop->sendCommand("Set_Model_Directory", payload.toType<string>());
	}
	void changeSkin(const ECS_Command & payload) {
		if (payload.isType<int>())
			m_prop->sendCommand("Set_Skin", payload.toType<int>());
	}
	void changeTransform(const ECS_Command & payload) {
		if (payload.isType<Transform>())
			m_prop->sendCommand("Set_Transform", payload.toType<Transform>());
	}
};

/**
* Creates a Prop entity, composed of an animated model component
**/
class Creator_Prop_Static : public EntityCreator
{
public:
	/** Constructor.
	 * @param	componentFactory	pointer to the component factory to allow creation of specific components */
	Creator_Prop_Static(Component_Factory * componentFactory) : EntityCreator(componentFactory) {}


	virtual void destroy(Entity * e) {
		Prop_Static_Entity * entity = (Prop_Static_Entity*)e;
		unMakeComponent(entity->m_prop);
		delete entity;
	}
	virtual Entity* create() {
		Prop_Static_Entity *entity = new Prop_Static_Entity();
		entity->m_prop = makeComponent("Static_Model");
		return entity;
	}
};

#endif // PROPS_H