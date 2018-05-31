#pragma once
#ifndef SUN
#define SUN
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\World\ECS\Entities\Entity.h"
#include "Utilities\Transform.h"


/** A sun entity. */
class DT_ENGINE_API Sun_Entity : public Entity
{
protected:
	// (de)Constructors
	/** Virtual Destructor. */
	virtual ~Sun_Entity() {};
	/** Constructor. */
	Sun_Entity() {
		m_commandMap["Change_Light_Color"] = [&](const ECS_Command & payload) {changeColor(payload); };
		m_commandMap["Change_Light_Intensity"] = [&](const ECS_Command & payload) {changeIntensity(payload); };
		m_commandMap["Change_Transform"] = [&](const ECS_Command & payload) {changeTransform(payload); };
	}


	// Protected Attributes
	Component * m_light;
	friend class Creator_Sun;


private:
	// Private Functions
	// Forward Commands
	void changeColor(const ECS_Command & payload) {
		if (payload.isType<vec3>())
			m_light->sendCommand("Set_Light_Color", payload.toType<vec3>());
	}
	void changeIntensity(const ECS_Command & payload) {
		if (payload.isType<float>())
			m_light->sendCommand("Set_Light_Intensity", payload.toType<float>());
	}
	void changeTransform(const ECS_Command & payload) {
		if (payload.isType<Transform>())
			m_light->sendCommand("Set_Transform", payload.toType<Transform>());
	}
};

/**
 * Creates a Sun entity, composed of only a directional light component.
 **/
class DT_ENGINE_API Creator_Sun : public EntityCreator
{
public:
	/** Constructor.
	 * @param	componentFactory	pointer to the component factory to allow creation of specific components */
	Creator_Sun(Component_Factory * componentFactory) : EntityCreator(componentFactory) {}


	virtual void destroy(Entity * e) {
		Sun_Entity * entity = (Sun_Entity*)e;
		unMakeComponent(entity->m_light);
		delete entity;
	}
	virtual Entity* create() {
		Sun_Entity *entity = new Sun_Entity();
		entity->m_light = makeComponent("Light_Directional");
		return entity;
	}
};

#endif // SUN