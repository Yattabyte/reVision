#include "Systems\World\ECS\Components\Light_Point_Component.h"
#include "Systems\World\ECS\Components\Geometry_Component.h"
#include "Systems\World\World.h"
#include "Systems\World\ECS\ECSmessage.h"
#include "Systems\Graphics\Graphics.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Base Types\Point.h"
#include "Utilities\EnginePackage.h"
#include "Utilities\Transform.h"
#include "GLFW\glfw3.h"
#include <math.h>


Light_Point_Component::~Light_Point_Component()
{
	m_pointTech->unregisterShadowCaster(m_shadowSpot);
	m_world->unregisterViewer(&m_camera);
}

Light_Point_Component::Light_Point_Component(const ECShandle & id, const ECShandle & pid, EnginePackage *enginePackage) : Lighting_Component(id, pid)
{
	m_enginePackage = enginePackage;
	m_radius = 0;
	m_squaredRadius = 0;
	m_lightPos = vec3(0.0f);
	m_lightVMatrix = mat4(1.0f);
	m_visSize = 0;

	auto graphics = m_enginePackage->getSubSystem<System_Graphics>("Graphics");
	m_pointTech = graphics->getBaseTech<Point_Tech>("Point_Tech");
	m_uboBuffer = graphics->m_lightBuffers.m_lightPointSSBO.addElement(&m_uboIndex);
	m_pointTech->registerShadowCaster(m_shadowSpot);

	m_world = m_enginePackage->getSubSystem<System_World>("World");
	m_world->registerViewer(&m_camera);
	m_camera.setHorizontalFOV(90.0f);
	m_camera.setDimensions(m_pointTech->getSize());
	
	// Write data to our index spot
	Point_Struct * uboData = &reinterpret_cast<Point_Struct*>(m_uboBuffer->pointer)[m_uboIndex];
	uboData->Shadow_Spot = m_shadowSpot;
	uboData->ShadowSize_Recip = 1.0f / m_pointTech->getSize().x;
}

void Light_Point_Component::updateViews()
{
	Point_Struct * uboData = &reinterpret_cast<Point_Struct*>(m_uboBuffer->pointer)[m_uboIndex];

	// Calculate view matrixs
	const mat4 trans = glm::translate(mat4(1.0f), m_lightPos);
	const mat4 scl = glm::scale(mat4(1.0f), vec3(m_squaredRadius));
	m_lightVMatrix = glm::translate(mat4(1.0f), -m_lightPos);
	uboData->lightV = m_lightVMatrix;
	uboData->mMatrix = trans * scl;

	m_camera.update();
	mat4 rotMats[6];
	const mat4 pMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.5f, m_squaredRadius);
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

void Light_Point_Component::receiveMessage(const ECSmessage &message)
{
	if (Component::compareMSGSender(message)) return;
	Point_Struct * uboData = &reinterpret_cast<Point_Struct*>(m_uboBuffer->pointer)[m_uboIndex];

	switch (message.GetCommandID()) {
		case SET_LIGHT_COLOR: {
			if (!message.IsOfType<vec3>()) break;
			const auto &payload = message.GetPayload<vec3>();
			uboData->LightColor = payload;
			break;
		}
		case SET_LIGHT_INTENSITY: {
			if (!message.IsOfType<float>()) break;
			const auto &payload = message.GetPayload<float>();
			uboData->LightIntensity = payload;
			break;
		}
		case SET_LIGHT_RADIUS: {
			if (!message.IsOfType<float>()) break;
			const auto &payload = message.GetPayload<float>();
			m_radius = payload;
			m_squaredRadius = payload * payload;
			uboData->LightRadius = payload;
			m_camera.setFarPlane(m_squaredRadius);
			updateViews();
			break;
		}
		case SET_POSITION: {
			if (!message.IsOfType<vec3>()) break;
			const auto &payload = message.GetPayload<vec3>();
			uboData->LightPosition = payload;
			m_lightPos = payload;		
			m_camera.setPosition(payload);
			updateViews();
			break;
		}
		case SET_TRANSFORM: {
			if (!message.IsOfType<Transform>()) break;
			const auto &payload = message.GetPayload<Transform>();
			uboData->LightPosition = payload.m_position;
			m_lightPos = payload.m_position;
			m_camera.setPosition(payload.m_position);
			updateViews();
			break;
		}
	}
}

bool Light_Point_Component::isVisible(const float & radius, const vec3 & eyePosition) const
{
	const float distance = glm::distance(m_lightPos, eyePosition);
	return radius + m_radius > distance;
}

void Light_Point_Component::occlusionPass()
{
	m_visSize = m_camera.getVisibilityToken().specificSize("Anim_Model");
	if (m_visSize) {
		glUniform1i(0, getBufferIndex());
		m_camera.getVisibleIndexBuffer().bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		m_camera.getCullingBuffer().bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		m_camera.getRenderBuffer().bindBufferBase(GL_SHADER_STORAGE_BUFFER, 7);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_visSize, 0);
	}	
}

void Light_Point_Component::shadowPass()
{
	if (m_visSize) {
		// Clear out the shadows
		m_pointTech->clearShadow(m_shadowSpot);
		glUniform1i(0, getBufferIndex());

		// Draw render lists
		glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
		m_camera.getVisibleIndexBuffer().bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		m_camera.getRenderBuffer().bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_visSize, 0);
		m_shadowUpdateTime = glfwGetTime();
	}
}

float Light_Point_Component::getImportance(const vec3 & position) const
{
	return m_radius / glm::length(position - m_lightPos);
}

#include "Systems\Graphics\Resources\Geometry Techniques\Model_Techniques.h"
void Light_Point_Component::update()
{
	// Update render lists
	Model_Technique::writeCameraBuffers(m_camera, 6);
}