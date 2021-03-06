#include "Utilities/Transform.h"
#include "glm/gtx/matrix_decompose.hpp"


Transform::Transform(const glm::vec3& position, const glm::quat& orientation, const glm::vec3& scale) :
	m_position(position),
	m_orientation(orientation),
	m_scale(scale)
{
	update();
}

Transform::Transform(const glm::quat& orientation) :
	Transform(glm::vec3(0.0F), orientation, glm::vec3(1.0F))
{
}

void Transform::update()
{
	m_modelMatrix =
		glm::translate(glm::mat4(1.0F), m_position) *
		glm::mat4_cast(m_orientation) *
		glm::scale(glm::mat4(1.0F), m_scale);
	m_inverseModelMatrix = glm::inverse(m_modelMatrix);
}

Transform Transform::inverse()
{
	Transform n(*this);
	n.m_modelMatrix = m_inverseModelMatrix;
	n.m_inverseModelMatrix = m_modelMatrix;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(n.m_modelMatrix, n.m_scale, n.m_orientation, n.m_position, skew, perspective);
	return n;
}

bool Transform::operator==(const Transform& other) const noexcept
{
	return (m_position == other.m_position && m_orientation == other.m_orientation && m_scale == other.m_scale);
}

bool Transform::operator!=(const Transform& other) const noexcept
{
	return !((*this) == other);
}

Transform& Transform::operator*=(const Transform& other)
{
	m_position += other.m_position;
	m_orientation *= other.m_orientation;
	m_scale *= other.m_scale;
	m_modelMatrix = m_modelMatrix * other.m_modelMatrix;
	m_inverseModelMatrix = glm::inverse(m_modelMatrix);
	return *this;
}

Transform Transform::operator*(const Transform& other) const
{
	Transform n(m_position + other.m_position, m_orientation * other.m_orientation, m_scale * other.m_scale);
	n.m_modelMatrix = m_modelMatrix * other.m_modelMatrix;
	n.m_inverseModelMatrix = glm::inverse(n.m_modelMatrix);
	return n;
}