#include "ECS\Components\Reflector.h"
#include "ECS\ECSmessage.h"
#include "Systems\Graphics\Graphics.h"
#include "Systems\Graphics\Resources\GFX_DEFINES.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\Reflections.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\Reflectors\IBL_Parallax.h"
#include "Engine.h"
#include "glm\gtc\matrix_transform.hpp"
#include <minmax.h>


Reflector_C::~Reflector_C()
{
	auto graphics = m_engine->getSubSystem<System_Graphics>("Graphics");
	graphics->getLightingTech<Reflections>("Reflections")->getReflectorTech<IBL_Parallax_Tech>("IBL_Parallax_Tech")->removeElement(m_uboIndex);
	graphics->m_reflectionSSBO.removeElement(&m_uboIndex);
}

Reflector_C::Reflector_C(Engine * engine, const ArgumentList & argumentList)
	: Reflector_C(
		engine,
		*(Transform*)argumentList.dataPointers[0]
	) {}

Reflector_C::Reflector_C(Engine * engine, const Transform & transform)
	: Component(engine)
{
	// Default Parameters
	m_position = glm::vec3(0.0f);
	m_scale = glm::vec3(1.0f);

	// Update Cameras
	glm::quat quats[6];
	quats[0] = glm::lookAt(glm::vec3(0), glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
	quats[1] = glm::lookAt(glm::vec3(0), glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
	quats[2] = glm::lookAt(glm::vec3(0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
	quats[3] = glm::lookAt(glm::vec3(0), glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
	quats[4] = glm::lookAt(glm::vec3(0), glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
	quats[5] = glm::lookAt(glm::vec3(0), glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
	for (int x = 0; x < 6; ++x) {
		m_cameras[x].setFarPlane(1000.0f);
		m_cameras[x].setOrientation(quats[x]);
		m_cameras[x].update();
	}

	// Acquire and update buffers
	auto graphics = m_engine->getSubSystem<System_Graphics>("Graphics");
	m_uboBuffer = graphics->m_reflectionSSBO.addElement(&m_uboIndex);
	(&reinterpret_cast<Reflection_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->CubeSpot = m_uboIndex;
	graphics->getLightingTech<Reflections>("Reflections")->getReflectorTech<IBL_Parallax_Tech>("IBL_Parallax_Tech")->addElement();

	// Register Commands
	m_commandMap["Set_Transform"] = [&](const ECS_Command & payload) {
		if (payload.isType<Transform>()) setTransform(payload.toType<Transform>());
	};

	// Update with passed parameters
	setTransform(transform);
}

void Reflector_C::setTransform(const Transform & transform)
{
	Reflection_Struct * uboData = &reinterpret_cast<Reflection_Struct*>(m_uboBuffer->pointer)[m_uboIndex];
	uboData->mMatrix = transform.m_modelMatrix;
	uboData->rotMatrix = glm::inverse(glm::toMat4(transform.m_orientation));
	uboData->BoxCamPos.xyz = transform.m_position;
	uboData->BoxScale.xyz = transform.m_scale;
	m_position = transform.m_position;
	m_scale = transform.m_scale;
	const float largest = pow(max(max(m_scale.x, m_scale.y), m_scale.z), 2.0f);

	for (int x = 0; x < 6; ++x) {
		m_cameras[x].setPosition(m_position);
		m_cameras[x].setFarPlane(largest);
		m_cameras[x].update();
	}
}

bool Reflector_C::isVisible(const float & radius, const glm::vec3 & eyePosition) const
{
	const float distance = glm::distance(m_position, eyePosition);
	return radius + glm::length(m_scale) > distance;
}

const unsigned int Reflector_C::getBufferIndex() const
{
	return m_uboIndex;
}

void Reflector_C::bindCamera(const unsigned int & index) const
{
	m_cameras[index].bind();
}
