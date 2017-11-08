#pragma once
#define GLM_ENABLE_EXPERIMENTAL 
#include "glm\glm.hpp"
#include "glm\common.hpp"
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

	Transform()
	{
		position = vec3(0.0f);
		orientation = quat(1, 0, 0, 0);
		scale = vec3(1.0f);
		Update();
	}

	void Update()
	{
		modelMatrix =	glm::translate( mat4(1.0f), position ) * 
						glm::mat4_cast( orientation ) *
						glm::scale( mat4(1.0f), scale );
		inverseModelMatrix = glm::inverse(modelMatrix);
	}
};