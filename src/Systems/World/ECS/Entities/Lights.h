#pragma once
#ifndef LIGHTS_H
#define LIGHTS_H

#include "Systems\World\ECS\Entities\Entity.h"
#include "Systems\World\ECS\Components\Component.h"
#include "Utilities\Transform.h"


/** A spot light entity. */
class SpotLight_Entity : public Entity
{
protected:
	// (de)Constructors
	/** Virtual Destructor. */
	virtual ~SpotLight_Entity() {};
	/** Constructor. */
	SpotLight_Entity() {
		m_commandMap["Change_Light_Color"] = [&](const ECS_Command & payload) {changeColor(payload); };
		m_commandMap["Change_Light_Intensity"] = [&](const ECS_Command & payload) {changeIntensity(payload); };
		m_commandMap["Change_Light_Radius"] = [&](const ECS_Command & payload) {changeRadius(payload); };
		m_commandMap["Change_Light_Cutoff"] = [&](const ECS_Command & payload) {changeCutoff(payload); };
		m_commandMap["Change_Transform"] = [&](const ECS_Command & payload) {changeTransform(payload); };
	}


	// Protected Attributes
	Component * m_light;
	friend class Creator_SpotLight;


private:
	// Private Functions
	// Forward Commands
	void changeColor(const ECS_Command & payload) {
		if (payload.isType<glm::vec3>())
			m_light->sendCommand("Set_Light_Color", payload.toType<glm::vec3>());
	}
	void changeIntensity(const ECS_Command & payload) {
		if (payload.isType<float>())
			m_light->sendCommand("Set_Light_Intensity", payload.toType<float>());
	}
	void changeRadius(const ECS_Command & payload) {
		if (payload.isType<float>())
			m_light->sendCommand("Set_Light_Radius", payload.toType<float>());
	}
	void changeCutoff(const ECS_Command & payload) {
		if (payload.isType<float>())
			m_light->sendCommand("Set_Light_Cutoff", payload.toType<float>());
	}
	void changeTransform(const ECS_Command & payload) {
		if (payload.isType<Transform>())
			m_light->sendCommand("Set_Transform", payload.toType<Transform>());
	}
};

/**
 * Creates a Spot Light entity, composed of only a spot light component.
 **/
class Creator_SpotLight : public EntityCreator
{
public:
	/** Constructor.
	 * @param	componentFactory	pointer to the component factory to allow creation of specific components */
	Creator_SpotLight(Component_Factory * componentFactory) : EntityCreator(componentFactory) {}
	

	virtual void destroy(Entity * e) {
		SpotLight_Entity * entity = (SpotLight_Entity*)e;
		unMakeComponent(entity->m_light);
		delete entity;
	}
	virtual Entity* create() {
		SpotLight_Entity *entity = new SpotLight_Entity();
		entity->m_light = makeComponent("Light_Spot");
		return entity;
	}
};

/** A cheap spot light entity. */
class SpotLight_Cheap_Entity : public Entity
{
protected:
	// (de)Constructors
	/** Virtual Destructor. */
	virtual ~SpotLight_Cheap_Entity() {};
	/** Constructor. */
	SpotLight_Cheap_Entity() {
		m_commandMap["Change_Light_Color"] = [&](const ECS_Command & payload) {changeColor(payload); };
		m_commandMap["Change_Light_Intensity"] = [&](const ECS_Command & payload) {changeIntensity(payload); };
		m_commandMap["Change_Light_Radius"] = [&](const ECS_Command & payload) {changeRadius(payload); };
		m_commandMap["Change_Light_Cutoff"] = [&](const ECS_Command & payload) {changeCutoff(payload); };
		m_commandMap["Change_Transform"] = [&](const ECS_Command & payload) {changeTransform(payload); };
	}


	// Protected Attributes
	Component * m_light;
	friend class Creator_SpotLight_Cheap;


private:
	// Private Functions
	// Forward Commands
	void changeColor(const ECS_Command & payload) {
		if (payload.isType<glm::vec3>())
			m_light->sendCommand("Set_Light_Color", payload.toType<glm::vec3>());
	}
	void changeIntensity(const ECS_Command & payload) {
		if (payload.isType<float>())
			m_light->sendCommand("Set_Light_Intensity", payload.toType<float>());
	}
	void changeRadius(const ECS_Command & payload) {
		if (payload.isType<float>())
			m_light->sendCommand("Set_Light_Radius", payload.toType<float>());
	}
	void changeCutoff(const ECS_Command & payload) {
		if (payload.isType<float>())
			m_light->sendCommand("Set_Light_Cutoff", payload.toType<float>());
	}
	void changeTransform(const ECS_Command & payload) {
		if (payload.isType<Transform>())
			m_light->sendCommand("Set_Transform", payload.toType<Transform>());
	}
};

/**
 * Creates a Cheap Spot Light entity, composed of only a cheap spot light component.
 **/
class Creator_SpotLight_Cheap : public EntityCreator
{
public:
	/** Constructor.
	 * @param	componentFactory	pointer to the component factory to allow creation of specific components */
	Creator_SpotLight_Cheap(Component_Factory * componentFactory) : EntityCreator(componentFactory) {}


	virtual void destroy(Entity * e) {
		SpotLight_Cheap_Entity * entity = (SpotLight_Cheap_Entity*)e;
		unMakeComponent(entity->m_light);
		delete entity;
	}
	virtual Entity* create() {
		SpotLight_Cheap_Entity *entity = new SpotLight_Cheap_Entity();
		entity->m_light = makeComponent("Light_Spot_Cheap");
		return entity;
	}
};

/** A point light entity. */
class PointLight_Entity : public Entity
{
protected:
	// (de)Constructors
	/** Virtual Destructor. */
	virtual ~PointLight_Entity() {};
	/** Constructor. */
	PointLight_Entity() {
		m_commandMap["Change_Light_Color"] = [&](const ECS_Command & payload) {changeColor(payload); };
		m_commandMap["Change_Light_Intensity"] = [&](const ECS_Command & payload) {changeIntensity(payload); };
		m_commandMap["Change_Light_Radius"] = [&](const ECS_Command & payload) {changeRadius(payload); };
		m_commandMap["Change_Transform"] = [&](const ECS_Command & payload) {changeTransform(payload); };
	}


	// Protected Attributes
	Component * m_light;
	friend class Creator_PointLight;


private:
	// Private Functions
	// Forward Commands
	void changeColor(const ECS_Command & payload) {
		if (payload.isType<glm::vec3>())
			m_light->sendCommand("Set_Light_Color", payload.toType<glm::vec3>());
	}
	void changeIntensity(const ECS_Command & payload) {
		if (payload.isType<float>())
			m_light->sendCommand("Set_Light_Intensity", payload.toType<float>());
	}
	void changeRadius(const ECS_Command & payload) {
		if (payload.isType<float>())
			m_light->sendCommand("Set_Light_Radius", payload.toType<float>());
	}
	void changeTransform(const ECS_Command & payload) {
		if (payload.isType<Transform>())
			m_light->sendCommand("Set_Transform", payload.toType<Transform>());
	}
};

/**
 * Creates a Point Light entity, composed of only a point light component.
 **/
class Creator_PointLight : public EntityCreator
{
public:
	/** Constructor.
	 * @param	componentFactory	pointer to the component factory to allow creation of specific components */
	Creator_PointLight(Component_Factory * componentFactory) : EntityCreator(componentFactory) {}


	virtual void destroy(Entity * e) {
		PointLight_Entity * entity = (PointLight_Entity*)e;
		unMakeComponent(entity->m_light);
		delete entity;
	}
	virtual Entity* create() {
		PointLight_Entity *entity = new PointLight_Entity();
		entity->m_light = makeComponent("Light_Point");
		return entity;
	}
};

/** A cheap point light entity. */
class PointLight_Cheap_Entity : public Entity
{
protected:
	// (de)Constructors
	/** Virtual Destructor. */
	virtual ~PointLight_Cheap_Entity() {};
	/** Constructor. */
	PointLight_Cheap_Entity() {
		m_commandMap["Change_Light_Color"] = [&](const ECS_Command & payload) {changeColor(payload); };
		m_commandMap["Change_Light_Intensity"] = [&](const ECS_Command & payload) {changeIntensity(payload); };
		m_commandMap["Change_Light_Radius"] = [&](const ECS_Command & payload) {changeRadius(payload); };
		m_commandMap["Change_Transform"] = [&](const ECS_Command & payload) {changeTransform(payload); };
	}


	// Protected Attributes
	Component * m_light;
	friend class Creator_PointLight_Cheap;


private:
	// Private Functions
	// Forward Commands
	void changeColor(const ECS_Command & payload) {
		if (payload.isType<glm::vec3>())
			m_light->sendCommand("Set_Light_Color", payload.toType<glm::vec3>());
	}
	void changeIntensity(const ECS_Command & payload) {
		if (payload.isType<float>())
			m_light->sendCommand("Set_Light_Intensity", payload.toType<float>());
	}
	void changeRadius(const ECS_Command & payload) {
		if (payload.isType<float>())
			m_light->sendCommand("Set_Light_Radius", payload.toType<float>());
	}
	void changeTransform(const ECS_Command & payload) {
		if (payload.isType<Transform>())
			m_light->sendCommand("Set_Transform", payload.toType<Transform>());
	}
};

/**
 * Creates a Cheap Point Light entity, composed of only a cheap point light component.
 **/
class Creator_PointLight_Cheap : public EntityCreator
{
public:
	/** Constructor.
	 * @param	componentFactory	pointer to the component factory to allow creation of specific components */
	Creator_PointLight_Cheap(Component_Factory * componentFactory) : EntityCreator(componentFactory) {}


	virtual void destroy(Entity * e) {
		PointLight_Cheap_Entity * entity = (PointLight_Cheap_Entity*)e;
		unMakeComponent(entity->m_light);
		delete entity;
	}
	virtual Entity* create() {
		PointLight_Cheap_Entity *entity = new PointLight_Cheap_Entity();
		entity->m_light = makeComponent("Light_Point_Cheap");
		return entity;
	}
};

#endif // LIGHTS_H