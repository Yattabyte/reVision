#pragma once
#ifndef TRANSFORMATION
#define TRANSFORMATION
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
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
 **/
struct DT_ENGINE_API Transform 
{
	// (de)Constructors
	/** Constructs a transformation object with any of the supplied parameters.
	 * @param position		the desired position
	 * @param orientation	the desired orientation
	 * @param scale			the desired scale */
	Transform(const vec3 &position = vec3(0.0f), const quat &orientation = quat(1, 0, 0, 0), const vec3 &scale = vec3(1.0f)) {
		m_position = position;
		m_orientation = orientation;
		m_scale = scale;
		update();
	}
	/** Constructs a transformation object with only orientation.
	* @param orientation	the desired orientation	*/
	Transform(const quat &orientation) {
		m_position = vec3(0.0f);
		m_orientation = orientation;
		m_scale = vec3(1.0f);
		update();
	}


	// Public Methods
	/** Recalculates the transformation matrix (and inverse) using this transformations current data. */
	void update() {
		m_modelMatrix = glm::translate( mat4(1.0f), m_position ) * 
						glm::mat4_cast( m_orientation ) *
						glm::scale( mat4(1.0f), m_scale );
		m_inverseModelMatrix = glm::inverse(m_modelMatrix);
	}


	// Public Attributes
	// Input Variables
	vec3 m_position;
	quat m_orientation;
	vec3 m_scale;
	// Derived Variables
	mat4x4 m_modelMatrix;
	mat4x4 m_inverseModelMatrix;
};

#endif // TRANSFORMATION