#pragma once
#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H
#define GLM_ENABLE_EXPERIMENTAL 

#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\quaternion.hpp"
#include "glm\gtx\quaternion.hpp"
#include "glm\gtc\matrix_access.hpp"


/** A 3D transformation object. 
Takes in position, orientation, and scaling attributes, and calculates a transformation matrix. */
struct Transform {
	// (de)Constructors
	/** Default Constructor. */
	Transform() = default;
	/** Constructs a transformation object with any of the supplied parameters.
	@param position			the desired position
	@param orientation		the desired orientation
	@param scale			the desired scale */
	Transform(const glm::vec3 & position, const glm::quat & orientation, const glm::vec3 & scale) {
		m_position = position;
		m_orientation = orientation;
		m_scale = scale;
		update();
	}
	/** Constructs a transformation object with only orientation.
	* @param orientation	the desired orientation	*/
	Transform(const glm::quat &orientation) {
		m_position = glm::vec3(0.0f);
		m_orientation = orientation;
		m_scale = glm::vec3(1.0f);
		update();
	}


	// Public Methods
	/** Recalculates the transformation matrix (and inverse) using this transformations current data. */
	void update() {
		m_modelMatrix = glm::translate( glm::mat4(1.0f), m_position ) * 
						glm::mat4_cast( m_orientation ) *
						glm::scale( glm::mat4(1.0f), m_scale );
		m_inverseModelMatrix = glm::inverse(m_modelMatrix);
	}


	// Public Attributes
	// Input Variables
	glm::vec3 m_position = glm::vec3(0.0f);
	glm::quat m_orientation = glm::quat(1, 0, 0, 0);
	glm::vec3 m_scale = glm::vec3(1.0f);
	// Derived Variables
	glm::mat4 m_modelMatrix = glm::mat4(1.0f);
	glm::mat4 m_inverseModelMatrix = glm::mat4(1.0f);
};

#endif // TRANSFORMATION_H