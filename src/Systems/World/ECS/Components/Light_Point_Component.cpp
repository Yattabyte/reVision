#include "Systems\World\ECS\Components\Light_Point_Component.h"
#include "Systems\World\ECS\Components\Geometry_Component.h"
#include "Utilities\EnginePackage.h"
#include "Systems\World\World.h"
#include "Systems\World\ECS\ECSmessage.h"
#include "Utilities\Transform.h"
#include "Systems\Graphics\Graphics.h"
#include "GLFW\glfw3.h"
#include <math.h>


Light_Point_Component::~Light_Point_Component()
{
	m_shadowMapper->unregisterShadowCaster(SHADOW_REGULAR, m_shadowSpots[0]);
	m_shadowMapper->unregisterShadowCaster(SHADOW_REGULAR, m_shadowSpots[1]);
	m_world->unregisterViewer(&m_camera[0]);
	m_world->unregisterViewer(&m_camera[1]);
}

Light_Point_Component::Light_Point_Component(const ECShandle & id, const ECShandle & pid, EnginePackage *enginePackage) : Lighting_Component(id, pid)
{
	m_enginePackage = enginePackage;
	m_squaredRadius = 0;
	m_lightPos = vec3(0.0f);
	m_lightVMatrix = mat4(1.0f);

	auto graphics = m_enginePackage->getSubSystem<System_Graphics>("Graphics");
	m_uboBuffer = graphics->m_lightPointSSBO.addElement(&m_uboIndex);
	m_shadowMapper = &graphics->m_shadowFBO;
	m_shadowMapper->registerShadowCaster(SHADOW_REGULAR, m_shadowSpots[0]);
	m_shadowMapper->registerShadowCaster(SHADOW_REGULAR, m_shadowSpots[1]);

	m_world = m_enginePackage->getSubSystem<System_World>("World");
	for (int x = 0; x < 2; ++x) {
		m_camera[x].setHorizontalFOV(180);
		m_camera[x].setDimensions(m_shadowMapper->getSize(SHADOW_REGULAR));
		m_camera[x].setOrientation(glm::rotate(quat(1, 0, 0, 0), glm::radians(180.0f * x), vec3(0, 1, 0)));
		m_world->registerViewer(&m_camera[x]);
	}
	
	// Write data to our index spot
	Point_Struct * uboData = &reinterpret_cast<Point_Struct*>(m_uboBuffer->pointer)[m_uboIndex];
	uboData->Shadow_Spot1 = m_shadowSpots[0];
	uboData->Shadow_Spot2 = m_shadowSpots[1];
	uboData->ShadowSize_Recip = 1.0f / m_shadowMapper->getSize(SHADOW_REGULAR).x;
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
			uboData->p_far = m_squaredRadius;
			uboData->LightRadius = payload;
			// Calculate view matrix
			const mat4 trans = glm::translate(mat4(1.0f), m_lightPos);
			const mat4 scl = glm::scale(mat4(1.0f), vec3(m_squaredRadius));
			m_lightVMatrix = glm::translate(mat4(1.0f), -m_lightPos);
			uboData->lightV = m_lightVMatrix;
			uboData->mMatrix = trans * scl;
			for (int x = 0; x < 2; ++x) {
				m_camera[x].setFarPlane(m_squaredRadius);
				m_camera[x].update();
			}
			break;
		}
		case SET_POSITION: {
			if (!message.IsOfType<vec3>()) break;
			const auto &payload = message.GetPayload<vec3>();
			uboData->LightPosition = payload;
			m_lightPos = payload;
			// Calculate view matrix
			const mat4 trans = glm::translate(mat4(1.0f), m_lightPos);
			const mat4 scl = glm::scale(mat4(1.0f), vec3(m_squaredRadius));
			m_lightVMatrix = glm::translate(mat4(1.0f), -m_lightPos);
			uboData->lightV = m_lightVMatrix;
			uboData->mMatrix = trans * scl;
			for (int x = 0; x < 2; ++x) 
				m_camera[x].setPosition(payload);			
			break;
		}
		case SET_TRANSFORM: {
			if (!message.IsOfType<Transform>()) break;
			const auto &payload = message.GetPayload<Transform>();
			uboData->LightPosition = payload.m_position;
			m_lightPos = payload.m_position;
			// Calculate view matrix
			const mat4 trans = glm::translate(mat4(1.0f), m_lightPos);
			const mat4 scl = glm::scale(mat4(1.0f), vec3(m_squaredRadius));
			m_lightVMatrix = glm::translate(mat4(1.0f), -m_lightPos);
			uboData->lightV = m_lightVMatrix;
			uboData->mMatrix = trans * scl;
			for (int x = 0; x < 2; ++x)
				m_camera[x].setPosition(payload.m_position);
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
	glUniform1i(0, getBufferIndex());
	for (int x = 0; x < 2; ++x) {
		const size_t size = m_camera[x].getVisibilityToken().specificSize("Anim_Model");
		if (size) {
			glUniform1f(1, (float(x) * 2.0f) - 1.0f); // update p_dir
			m_camera[x].getVisibleIndexBuffer().bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
			m_camera[x].getRenderBuffer().bindBufferBase(GL_SHADER_STORAGE_BUFFER, 7);
			glDrawArraysInstanced(GL_TRIANGLES, 0, 36, size);
		}
	}
}

void Light_Point_Component::shadowPass()
{
	glUniform1i(0, getBufferIndex());

	// Clear out the shadows
	for (int x = 0; x < 2; ++x)
		m_shadowMapper->clearShadow(SHADOW_REGULAR, m_shadowSpots[x]);

	for (int x = 0; x < 2; ++x) {
		const size_t size = m_camera[x].getVisibilityToken().specificSize("Anim_Model");
		if (size) {
			glUniform1f(1, (float(x) * 2.0f) - 1.0f); // update p_dir

			// Draw render lists
			glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
			m_camera[x].getVisibleIndexBuffer().bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
			m_camera[x].getRenderBuffer().bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glMultiDrawArraysIndirect(GL_TRIANGLES, 0, size, 0);

			m_shadowUpdateTime = glfwGetTime();
		}
	}
}

float Light_Point_Component::getImportance(const vec3 & position) const
{
	return m_radius / glm::length(position - m_lightPos);
}

#include "Systems\Graphics\Resources\Geometry Techniques\Model_Techniques.h"
void Light_Point_Component::update()
{
	// Update cameras to face the right direction
	for (int x = 0; x < 2; ++x) {
		// Update render list
		Model_Technique::writeCameraBuffers(m_camera[x]);
	}
}