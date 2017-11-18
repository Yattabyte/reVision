/*
	Prop

	- A renderable mesh with a position, orientation, and scale
	- Is of type "Geometry", and is intended to be registered in some form of geometry manager
*/

#pragma once
#ifndef PROP
#define PROP
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif
#define GEOMETRY_TYPE_PROP 0

#include "Entities\Entity.h"
#include "Utilities\Transform.h"
#include <string>
#include <shared_mutex>

using namespace glm;
using namespace std;

class PropCreator;
class Prop : protected Entity
{
public:
	/*************
	----Common----
	*************/


	/*************************
	----Geometry Functions----
	*************************/

	static int GetGeometryType() { return GEOMETRY_TYPE_PROP; }
	DELTA_CORE_API virtual bool shouldRender(const mat4 &PVMatrix);
	DELTA_CORE_API virtual void geometryPass() const;

	
	/*************************
	----Variable Functions----
	*************************/

/*	void setPosition(const vec3 &position) { lock_guard<shared_mutex> write_guard(data_mutex);  worldState.position = position; }
	void setOrientation(const quat &orientation) { lock_guard<shared_mutex> write_guard(data_mutex); worldState.orientation = orientation; }
	void setScale(const vec3 &scale) { lock_guard<shared_mutex> write_guard(data_mutex); worldState.scale = scale; }
	vec3 getPosition() const { shared_lock<shared_mutex> read_guard(data_mutex); return worldState.position; }
	quat getOrientation() const { shared_lock<shared_mutex> read_guard(data_mutex); return worldState.orientation; }
	vec3 getScale() const { shared_lock<shared_mutex> read_guard(data_mutex); return worldState.scale; }
	mat4 getModelMatrix() const { shared_lock<shared_mutex> read_guard(data_mutex); return worldState.modelMatrix; }
	mat4 getInverseModelMatrix() const { shared_lock<shared_mutex> read_guard(data_mutex); return worldState.inverseModelMatrix; }
	shared_mutex & getDataMutex() const { return data_mutex; };*/
	DELTA_CORE_API void Update();


	/****************
	----Variables----
	****************/
	//mutable shared_mutex data_mutex;
	//Transform worldState;
protected:
	DELTA_CORE_API ~Prop();
	DELTA_CORE_API Prop(const ECSHandle &id) : Entity(id) {}
	friend class PropCreator;
};

class PropCreator : public EntityCreator
{
public:
	DELTA_CORE_API virtual Entity* Create(const ECSHandle &id) {
		Prop *prop = new Prop(id);
		prop->addComponent("Anim_Model");
		return prop;
	}
};

#endif // PROP