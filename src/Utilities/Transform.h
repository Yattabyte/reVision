#pragma once
#ifndef TRANSFORMATION
#define TRANSFORMATION
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define GLM_ENABLE_EXPERIMENTAL 

#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\quaternion.hpp"
#include "glm\gtx\quaternion.hpp"
#include "glm\gtc\matrix_access.hpp"

using namespace glm;


/**
 * A 3D transformation object. 
 * Takes in position, orientation, and scaling attributes, and calculates a transformation matrix.
 * @todo Make member variables m_*name* and fix occurences related to it
 * @todo Give constructor better named parameters
 **/
struct DT_ENGINE_API Transform 
{
	// (de)Constructors
	/** Constructs a transformation object with any of the supplied parameters.
	 * @param p		the desired position
	 * @param ori	the desired orientation
	 * @param scl	the desired scale
	 */
	Transform(const vec3 &p = vec3(0.0f), const quat &ori = quat(1, 0, 0, 0), const vec3 &scl = vec3(1.0f)) {
		position = p;
		orientation = ori;
		scale = scl;
		update();
	}


	// Public Methods
	/** Recalculates the transformation matrix (and inverse) using this transformations current data. */
	void update() {
		modelMatrix = glm::translate( mat4(1.0f), position ) * 
					  glm::mat4_cast( orientation ) *
					  glm::scale( mat4(1.0f), scale );
		inverseModelMatrix = glm::inverse(modelMatrix);
	}


	// Public Attributes
	// Input Variables
	vec3 position;
	quat orientation;
	vec3 scale;
	// Derived Variables
	mat4x4 modelMatrix;
	mat4x4 inverseModelMatrix;
};

#endif // TRANSFORMATION