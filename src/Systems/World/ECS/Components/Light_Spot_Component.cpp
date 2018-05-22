#include "Systems\World\ECS\Components\Light_Spot_Component.h"
#include "Systems\World\ECS\Components\Geometry_Component.h"
#include "Systems\World\ECS\ECSmessage.h"
#include "Systems\World\World.h"
#include "Utilities\EnginePackage.h"
#include "Utilities\Transform.h"
#include "Systems\Graphics\Graphics.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Base Types\Spot.h"
#include "GLFW\glfw3.h"
#include <math.h>


Light_Spot_Component::~Light_Spot_Component()
{
	m_spotTech->unregisterShadowCaster(m_shadowSpot);
	m_world->unregisterViewer(&m_camera);
}

Light_Spot_Component::Light_Spot_Component(const ECShandle & id, const ECShandle & pid, EnginePackage * enginePackage) : Lighting_Component(id, pid)
{
	m_enginePackage = enginePackage;
	m_squaredRadius = 0;
	m_orientation = quat(1, 0, 0, 0);
	m_visSize = 0;

	auto graphics = m_enginePackage->getSubSystem<System_Graphics>("Graphics");
	m_spotTech = graphics->getBaseTech<Spot_Tech>("Spot_Tech");
	m_uboBuffer = graphics->m_lightBuffers.m_lightSpotSSBO.addElement(&m_uboIndex);
	m_spotTech->registerShadowCaster(m_shadowSpot);
	
	m_world = m_enginePackage->getSubSystem<System_World>("World");
	m_world->registerViewer(&m_camera);	

	// Write data to our index spot
	Spot_Struct * uboData = &reinterpret_cast<Spot_Struct*>(m_uboBuffer->pointer)[m_uboIndex];
	uboData->Shadow_Spot = m_shadowSpot;
	uboData->ShadowSize_Recip = 1.0f / m_spotTech->getSize().x;
	m_camera.setDimensions(m_spotTech->getSize());
}

void Light_Spot_Component::receiveMessage(const ECSmessage &message)
{
	if (Component::compareMSGSender(message)) return;
	Spot_Struct * uboData = &reinterpret_cast<Spot_Struct*>(m_uboBuffer->pointer)[m_uboIndex];

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
			m_camera.setFarPlane(m_squaredRadius);
			uboData->LightRadius = payload;

			// Recalculate perspective matrix
			m_camera.update();
			const mat4 lightP = m_camera.getCameraBuffer().pMatrix;
			const mat4 lightPV = lightP * m_lightVMatrix;
			uboData->lightPV = lightPV;
			uboData->inversePV = glm::inverse(lightPV);
			m_camera.setMatrices(lightP, m_lightVMatrix);
			break;
		}
		case SET_LIGHT_CUTOFF: {
			if (!message.IsOfType<float>()) break;
			const auto &payload = message.GetPayload<float>();
			uboData->LightCutoff = cosf(glm::radians(payload));
			m_camera.setHorizontalFOV(payload * 2.0f);

			// Recalculate perspective matrix
			m_camera.update();
			const mat4 lightP = m_camera.getCameraBuffer().pMatrix;
			const mat4 lightPV = lightP * m_lightVMatrix;
			uboData->lightPV = lightPV;
			uboData->inversePV = glm::inverse(lightPV);
			m_camera.setMatrices(lightP, m_lightVMatrix);
			break;
		}
		case SET_ORIENTATION: {
			if (!message.IsOfType<quat>()) break;
			const auto &payload = message.GetPayload<quat>();
			m_orientation = payload;			

			// Recalculate view matrix
			const mat4 trans = glm::translate(mat4(1.0f), m_lightPos);
			const mat4 rot = glm::mat4_cast(m_orientation);
			const mat4 scl = glm::scale(mat4(1.0f), vec3(m_squaredRadius));
			const mat4 final = glm::inverse(trans * rot * glm::rotate(mat4(1.0f), glm::radians(-90.0f), vec3(0, 1, 0)));
			m_lightVMatrix = final;
			uboData->lightV = final;
			uboData->LightDirection = (rot * vec4(1, 0, 0, 0)).xyz;
			uboData->mMatrix = (trans * rot) * scl;

			// Recalculate perspective matrix
			m_camera.update();
			const mat4 lightP = m_camera.getCameraBuffer().pMatrix;
			const mat4 lightPV = lightP * m_lightVMatrix;
			uboData->lightPV = lightPV;
			uboData->inversePV = glm::inverse(lightPV);
			m_camera.setMatrices(lightP, m_lightVMatrix);
			break;
		}
		case SET_POSITION: {
			if (!message.IsOfType<vec3>()) break;
			const auto &payload = message.GetPayload<vec3>();
			m_lightPos = payload;
			uboData->LightPosition = payload;
			m_camera.setPosition(payload);

			// Recalculate view matrix
			const mat4 trans = glm::translate(mat4(1.0f), m_lightPos);
			const mat4 rot = glm::mat4_cast(m_orientation);
			const mat4 scl = glm::scale(mat4(1.0f), vec3(m_squaredRadius));
			const mat4 final = glm::inverse(trans * rot * glm::rotate(mat4(1.0f), glm::radians(-90.0f), vec3(0, 1, 0)));
			m_lightVMatrix = final;
			uboData->lightV = final;
			uboData->LightDirection = (rot * vec4(1, 0, 0, 0)).xyz;
			uboData->mMatrix = (trans * rot) * scl;

			// Recalculate perspective matrix
			m_camera.update();
			const mat4 lightP = m_camera.getCameraBuffer().pMatrix;
			const mat4 lightPV = lightP * m_lightVMatrix;
			uboData->lightPV = lightPV;
			uboData->inversePV = glm::inverse(lightPV);
			m_camera.setMatrices(lightP, m_lightVMatrix);
			break;
		}
		case SET_TRANSFORM: {
			if (!message.IsOfType<Transform>()) break;
			const auto &payload = message.GetPayload<Transform>();
			uboData->LightPosition = payload.m_position;
			m_lightPos = payload.m_position;

			// Recalculate view matrix
			const mat4 trans = glm::translate(mat4(1.0f), m_lightPos);
			const mat4 rot = glm::mat4_cast(m_orientation);
			const mat4 scl = glm::scale(mat4(1.0f), vec3(m_squaredRadius));
			const mat4 final = glm::inverse(trans * rot * glm::rotate(mat4(1.0f), glm::radians(-90.0f), vec3(0, 1, 0)));
			m_lightVMatrix = final;
			uboData->lightV = final;
			uboData->LightDirection = (rot * vec4(1, 0, 0, 0)).xyz;
			uboData->mMatrix = (trans * rot) * scl;

			// Recalculate perspective matrix
			m_camera.update();
			const mat4 lightP = m_camera.getCameraBuffer().pMatrix;
			const mat4 lightPV = lightP * m_lightVMatrix;
			uboData->lightPV = lightPV;
			uboData->inversePV = glm::inverse(lightPV);
			m_camera.setMatrices(lightP, m_lightVMatrix);
			break;
		}
	}
}

bool Light_Spot_Component::isVisible(const float & radius, const vec3 & eyePosition) const
{
	const float distance = glm::distance(m_lightPos, eyePosition);
	return radius + m_radius > distance;
}

void Light_Spot_Component::occlusionPass()
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

void Light_Spot_Component::shadowPass()
{
	if (m_visSize) {
		// Clear out the shadows
		m_spotTech->clearShadow(m_shadowSpot);
		glUniform1i(0, getBufferIndex());

		// Draw render lists
		glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
		m_camera.getVisibleIndexBuffer().bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		m_camera.getRenderBuffer().bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_visSize, 0);

		m_shadowUpdateTime = glfwGetTime();
	}
}

float Light_Spot_Component::getImportance(const vec3 & position) const
{
	return m_radius / glm::length(position - m_lightPos);
}

#include "Systems\Graphics\Resources\Geometry Techniques\Model_Techniques.h"
void Light_Spot_Component::update()
{
	// Update render lists
	Model_Technique::writeCameraBuffers(m_camera);
}