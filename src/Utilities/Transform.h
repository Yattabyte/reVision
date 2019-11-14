#pragma once
#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H
#define GLM_ENABLE_EXPERIMENTAL

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/matrix_access.hpp"
#include "glm/gtx/matrix_decompose.hpp"


/** A 3D transformation object.
Takes in position, orientation, and scaling attributes, and calculates a transformation matrix. */
struct Transform {
	// (De)Constructors
	/** Default Destructor. */
	inline ~Transform() = default;
	/** Default Constructor. */
	inline Transform() = default;
	/** Constructs a transformation object with any of the supplied parameters.
	@param position			the desired position
	@param orientation		the desired orientation
	@param scale			the desired scale */
	inline Transform(const glm::vec3& position, const glm::quat& orientation, const glm::vec3& scale) noexcept
		: m_position(position), m_orientation(orientation), m_scale(scale) {
		update();
	}
	/** Constructs a transformation object with only orientation.
	* @param orientation	the desired orientation	*/
	inline explicit Transform(const glm::quat& orientation) noexcept
		: Transform(glm::vec3(0.0f), orientation, glm::vec3(1.0f)) {
	}


	// Public Methods
	/** Recalculates the transformation matrix (and inverse) using this transformations current data. */
	inline void update() noexcept {
		m_modelMatrix = glm::translate(glm::mat4(1.0f), m_position) *
			glm::mat4_cast(m_orientation) *
			glm::scale(glm::mat4(1.0f), m_scale);
		m_inverseModelMatrix = glm::inverse(m_modelMatrix);
	}
	inline Transform inverse() noexcept {
		Transform n(*this);
		n.m_modelMatrix = m_inverseModelMatrix;
		n.m_inverseModelMatrix = m_modelMatrix;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(n.m_modelMatrix, n.m_scale, n.m_orientation, n.m_position, skew, perspective);
		return n;
	}
	inline bool operator==(const Transform& other) const noexcept {
		return (m_position == other.m_position && m_orientation == other.m_orientation && m_scale == other.m_scale);
	}
	inline bool operator!=(const Transform& other) const noexcept {
		return !((*this) == other);
	}
	inline Transform& operator*=(const Transform& o) noexcept {
		m_position += o.m_position;
		m_orientation *= o.m_orientation;
		m_scale *= o.m_scale;
		m_modelMatrix = m_modelMatrix * o.m_modelMatrix;
		m_inverseModelMatrix = glm::inverse(m_modelMatrix);
		return *this;
	}
	inline Transform operator*(const Transform& o) const noexcept {
		Transform n(m_position + o.m_position, m_orientation * o.m_orientation, m_scale * o.m_scale);
		n.m_modelMatrix = m_modelMatrix * o.m_modelMatrix;
		n.m_inverseModelMatrix = glm::inverse(n.m_modelMatrix);
		return n;
	}


	// Public Attributes
	//// Input Variables
	glm::vec3 m_position = glm::vec3(0.0f);
	glm::quat m_orientation = glm::quat(1, 0, 0, 0);
	glm::vec3 m_scale = glm::vec3(1.0f);
	//// Derived Variables
	glm::mat4 m_modelMatrix = glm::mat4(1.0f);
	glm::mat4 m_inverseModelMatrix = glm::mat4(1.0f);
};

#endif // TRANSFORMATION_H