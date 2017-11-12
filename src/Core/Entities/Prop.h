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

#include "Entities\Geometry.h"
#include "Utilities\Transform.h"
#include "Assets\Asset_Model.h"
#include <string>
#include <shared_mutex>

using namespace glm;
using namespace std;

struct Transform_Buffer {
	int useBones; vec3 padding1;
	mat4 mMatrix;
	mat4 transforms[NUM_MAX_BONES];
};

class Prop : public Geometry
{
public:
	/*************
	----Common----
	*************/

	DELTA_CORE_API ~Prop();
	DELTA_CORE_API Prop();
	DELTA_CORE_API Prop(const string &relativePath);
	DELTA_CORE_API Prop(const Prop &other);
	DELTA_CORE_API void operator= (const Prop &other);
	DELTA_CORE_API virtual void registerSelf();
	DELTA_CORE_API virtual void unregisterSelf();


	/*************************
	----Geometry Functions----
	*************************/

	static int GetGeometryType() { return GEOMETRY_TYPE_PROP; }
	DELTA_CORE_API virtual bool shouldRender(const mat4 &PVMatrix);
	DELTA_CORE_API virtual void geometryPass() const;

	
	/*************************
	----Variable Functions----
	*************************/

	void setPosition(const vec3 &position) { lock_guard<shared_mutex> write_guard(data_mutex);  worldState.position = position; }
	void setOrientation(const quat &orientation) { lock_guard<shared_mutex> write_guard(data_mutex); worldState.orientation = orientation; }
	void setScale(const vec3 &scale) { lock_guard<shared_mutex> write_guard(data_mutex); worldState.scale = scale; }
	vec3 getPosition() const { shared_lock<shared_mutex> read_guard(data_mutex); return worldState.position; }
	quat getOrientation() const { shared_lock<shared_mutex> read_guard(data_mutex); return worldState.orientation; }
	vec3 getScale() const { shared_lock<shared_mutex> read_guard(data_mutex); return worldState.scale; }
	mat4 getModelMatrix() const { shared_lock<shared_mutex> read_guard(data_mutex); return worldState.modelMatrix; }
	mat4 getInverseModelMatrix() const { shared_lock<shared_mutex> read_guard(data_mutex); return worldState.inverseModelMatrix; }
	shared_mutex & getDataMutex() const { return data_mutex; };
	DELTA_CORE_API void Update();


	/****************
	----Variables----
	****************/
	mutable shared_mutex data_mutex;
	Transform worldState;
	Shared_Asset_Model assetModel;
	GLuint uboID;
	Transform_Buffer uboData;
};

#endif // PROP