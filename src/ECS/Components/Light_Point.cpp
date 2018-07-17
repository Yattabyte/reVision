#include "ECS\Components\Light_Point.h"
#include "ECS\Components\Geometry.h"
#include "Systems\World\World.h"
#include "ECS\ECSmessage.h"
#include "Systems\Graphics\Graphics.h"
#include "Systems\Graphics\Resources\Lights\Point.h"
#include "Engine.h"
#include "GLFW\glfw3.h"
#include <math.h>


Light_Point_C::~Light_Point_C()
{
	m_pointTech->unregisterShadowCaster(m_shadowSpot);
	m_world->unregisterViewer(&m_camera);
	m_engine->getSubSystem<System_Graphics>("Graphics")->m_lightBuffers.m_lightPointSSBO.removeElement(&m_uboIndex);
}

Light_Point_C::Light_Point_C(Engine * engine, const glm::vec3 & color, const float & intensity, const float & radius, const Transform & transform)
{
	// Default Parameters
	m_engine = engine;
	m_radius = 0;
	m_squaredRadius = 0;
	m_lightPos = glm::vec3(0.0f);
	m_lightVMatrix = glm::mat4(1.0f);
	m_visSize[0] = 0;
	m_visSize[1] = 0;

	// Acquire and update buffers
	auto graphics = m_engine->getSubSystem<System_Graphics>("Graphics");
	m_pointTech = graphics->getBaseTech<Point_Tech>("Point_Tech");
	m_uboBuffer = graphics->m_lightBuffers.m_lightPointSSBO.addElement(&m_uboIndex);
	m_pointTech->registerShadowCaster(m_shadowSpot);
	m_world = m_engine->getSubSystem<System_World>("World");
	m_world->registerViewer(&m_camera);
	m_camera.setHorizontalFOV(90.0f);
	m_camera.setDimensions(m_pointTech->getSize());
	Point_Struct * uboData = &reinterpret_cast<Point_Struct*>(m_uboBuffer->pointer)[m_uboIndex];
	uboData->Shadow_Spot = m_shadowSpot;
	uboData->ShadowSize_Recip = 1.0f / m_pointTech->getSize().x;

	// Register Commands
	m_commandMap["Set_Light_Color"] = [&](const ECS_Command & payload) {
		if (payload.isType<glm::vec3>()) setColor(payload.toType<glm::vec3>());
	};
	m_commandMap["Set_Light_Intensity"] = [&](const ECS_Command & payload) {
		if (payload.isType<float>()) setIntensity(payload.toType<float>());
	}; 
	m_commandMap["Set_Light_Radius"] = [&](const ECS_Command & payload) {
		if (payload.isType<float>()) setRadius(payload.toType<float>());
	};
	m_commandMap["Set_Transform"] = [&](const ECS_Command & payload) {
		if (payload.isType<Transform>()) setTransform(payload.toType<Transform>());
	};

	// Update with passed parameters
	setColor(color);
	setIntensity(intensity);
	setRadius(radius);
	setTransform(transform);
}

void Light_Point_C::updateViews()
{
	Point_Struct * uboData = &reinterpret_cast<Point_Struct*>(m_uboBuffer->pointer)[m_uboIndex];

	// Calculate view matrixs
	const glm::mat4 trans = glm::translate(glm::mat4(1.0f), m_lightPos);
	const glm::mat4 scl = glm::scale(glm::mat4(1.0f), glm::vec3(m_squaredRadius));
	m_lightVMatrix = glm::translate(glm::mat4(1.0f), -m_lightPos);
	uboData->lightV = m_lightVMatrix;
	uboData->mMatrix = trans * scl;

	m_camera.update();
	glm::mat4 rotMats[6];
	const glm::mat4 pMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.5f, m_squaredRadius);
	rotMats[0] = pMatrix * glm::lookAt(m_lightPos, m_lightPos + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
	rotMats[1] = pMatrix * glm::lookAt(m_lightPos, m_lightPos + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
	rotMats[2] = pMatrix * glm::lookAt(m_lightPos, m_lightPos + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
	rotMats[3] = pMatrix * glm::lookAt(m_lightPos, m_lightPos + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
	rotMats[4] = pMatrix * glm::lookAt(m_lightPos, m_lightPos + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
	rotMats[5] = pMatrix * glm::lookAt(m_lightPos, m_lightPos + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));

	for (int x = 0; x < 6; ++x) {
		uboData->lightPV[x] = rotMats[x];
		uboData->inversePV[x] = glm::inverse(rotMats[x]);
	}
}

void Light_Point_C::setColor(const glm::vec3 & color)
{
	(&reinterpret_cast<Point_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->LightColor = color;
}

void Light_Point_C::setIntensity(const float & intensity)
{
	(&reinterpret_cast<Point_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->LightIntensity = intensity;
}

void Light_Point_C::setRadius(const float & radius)
{
	m_radius = radius;
	m_squaredRadius = radius * radius;
	(&reinterpret_cast<Point_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->LightRadius = radius;
	m_camera.setFarPlane(m_squaredRadius);
	updateViews();
}

void Light_Point_C::setTransform(const Transform & transform)
{
	(&reinterpret_cast<Point_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->LightPosition = transform.m_position;
	m_lightPos = transform.m_position;
	m_camera.setPosition(transform.m_position);
	updateViews();
}

bool Light_Point_C::isVisible(const float & radius, const glm::vec3 & eyePosition) const
{
	const float distance = glm::distance(m_lightPos, eyePosition);
	return radius + m_radius > distance;
}

void Light_Point_C::occlusionPass(const unsigned int & type)
{
	if (m_visSize[type]) {
		glUniform1i(0, getBufferIndex());
		const auto &visBuffers = m_camera.getVisibilityBuffers();
		visBuffers.m_buffer_Index[type].bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		visBuffers.m_buffer_Culling[type].bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		visBuffers.m_buffer_Render[type].bindBufferBase(GL_SHADER_STORAGE_BUFFER, 7);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_visSize[type], 0);
	}	
}

void Light_Point_C::shadowPass(const unsigned int & type)
{
	if (m_visSize[type]) {
		// Clear out the shadows
		m_pointTech->clearShadow(type, m_shadowSpot);
		glUniform1i(0, getBufferIndex());

		// Draw render lists
		glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
		const auto &visBuffers = m_camera.getVisibilityBuffers();
		visBuffers.m_buffer_Index[type].bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		visBuffers.m_buffer_Render[type].bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_visSize[type], 0);
		m_shadowUpdateTime = glfwGetTime();
	}
}

float Light_Point_C::getImportance(const glm::vec3 & position) const
{
	return m_radius / glm::length(position - m_lightPos);
}

#include "Systems\Graphics\Resources\Geometry Techniques\Model_Technique.h"
#include "Systems\Graphics\Resources\Geometry Techniques\Model_Static_Technique.h"
void Light_Point_C::update(const unsigned int & type)
{
	// Update render lists
	const char * string_type;
	switch (type) {
		case CAM_GEOMETRY_DYNAMIC:
			Model_Technique::writeCameraBuffers(m_camera, 6);
			string_type = "Anim_Model";
			break;
		case CAM_GEOMETRY_STATIC:
			Model_Static_Technique::writeCameraBuffers(m_camera, 6);
			string_type = "Static_Model";
			break;
	}
	m_visSize[type] = m_camera.getVisibilityToken().specificSize(string_type);
}