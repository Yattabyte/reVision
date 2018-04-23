#include "Systems\World\ECS\Components\Light_Point_Component.h"
#include "Systems\World\ECS\Components\Geometry_Component.h"
#include "Utilities\EnginePackage.h"
#include "Systems\World\World.h"
#include "Systems\World\ECS\ECSmessage.h"
#include "Utilities\Frustum.h"
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
	uboData->ShadowSize = m_shadowMapper->getSize(SHADOW_REGULAR).x;
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
			m_camera[0].setFarPlane(m_squaredRadius);
			m_camera[1].setFarPlane(m_squaredRadius);
			m_camera[0].update();
			m_camera[1].update();
			// Calculate view matrix
			const mat4 trans = glm::translate(mat4(1.0f), m_lightPos);
			const mat4 scl = glm::scale(mat4(1.0f), vec3(m_squaredRadius));
			m_lightVMatrix = glm::translate(mat4(1.0f), -m_lightPos);
			uboData->lightV = m_lightVMatrix;
			uboData->mMatrix = trans * scl;
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
			break;
		}
	}
}

bool Light_Point_Component::isVisible(const float & radius, const vec3 & eyePosition, const mat4 & PMatrix, const mat4 &VMatrix) const
{
	const float distance = glm::distance(m_lightPos, eyePosition);
	return radius + m_radius > distance;
}

void Light_Point_Component::shadowPass()
{
	glUniform1i(0, getBufferIndex());
	
	for (int x = 0; x < 2; ++x) {
		const size_t size = m_camera[x].getVisibilityToken().specificSize("Anim_Model");
		if (size) {
			glUniform1f(1, (float(x) * 2.0f) - 1.0f); // update p_dir

			// Draw render lists
			m_visGeoUBO[x].bindBufferBase(GL_UNIFORM_BUFFER, 3);
			m_indirectGeo[x].bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glMultiDrawArraysIndirect(GL_TRIANGLES, 0, size, 0);

			m_shadowUpdateTime = glfwGetTime();
		}
	}
}

float Light_Point_Component::getImportance(const vec3 & position) const
{
	return m_radius / glm::length(position - m_lightPos);
}

#include "Systems\World\ECS\Components\Anim_Model_Component.h"
void Light_Point_Component::update()
{
	// Update cameras to face the right direction
	const vec2 &size = m_shadowMapper->getSize(SHADOW_REGULAR);
	for (int x = 0; x < 2; ++x) {
		m_camera[x].setPosition(m_lightPos);		
		m_camera[x].update();

		// Update render list
		const Visibility_Token vis_token = m_camera[x].getVisibilityToken();
		const size_t size = vis_token.specificSize("Anim_Model");
		if (size) {
			// Clear out the shadows which we will update next shadow pass
			m_shadowMapper->clearShadow(SHADOW_REGULAR, m_shadowSpots[x]);
			struct DrawData {
				GLuint count;
				GLuint instanceCount = 1;
				GLuint first;
				GLuint baseInstance = 0;
				DrawData(const GLuint & c = 0, const GLuint & f = 0) : count(c), first(f) {}
			};
			vector<ivec4> geoArray(size);
			vector<DrawData> drawData(size);
			unsigned int count = 0;
			for each (const auto &component in vis_token.getTypeList<Anim_Model_Component>("Anim_Model")) {
				geoArray[count] = ivec4(component->getBufferIndex());
				const ivec2 drawInfo = component->getDrawInfo();
				drawData[count++] = DrawData(drawInfo.y, drawInfo.x);
			}
			m_visGeoUBO[x].write(0, sizeof(ivec4)*geoArray.size(), geoArray.data());
			m_indirectGeo[x].write(0, sizeof(DrawData) * size, drawData.data());
		}
	}
}