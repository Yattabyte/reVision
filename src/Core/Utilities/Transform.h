/*
	Transformation

	- A 3D transformation
*/

#pragma once
#ifndef TRANSFORMATION
#define TRANSFORMATION
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#define GLM_ENABLE_EXPERIMENTAL 
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\quaternion.hpp"
#include "glm\gtx\quaternion.hpp"
#include "glm\gtc\matrix_access.hpp"

using namespace glm;

struct Transform
{
	// Raw Variables
	vec3 position;
	quat orientation;
	vec3 scale;

	// Derived Variables
	mat4x4 modelMatrix;
	mat4x4 inverseModelMatrix;

	DELTA_CORE_API Transform(const vec3 &p = vec3(0.0f), const quat &ori = quat(1, 0, 0, 0), const vec3 &scl = vec3(1.0f))
	{
		position = p;
		orientation = ori;
		scale = scl;
		Update();
	}

	DELTA_CORE_API void Update()
	{
		modelMatrix =	glm::translate( mat4(1.0f), position ) * 
						glm::mat4_cast( orientation ) *
						glm::scale( mat4(1.0f), scale );
		inverseModelMatrix = glm::inverse(modelMatrix);
	}
};

#endif // TRANSFORMATION