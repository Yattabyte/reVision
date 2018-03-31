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
	glDeleteBuffers(1, &m_uboID);
	if (m_enginePackage) {
		if (m_shadowMapper) {
			m_shadowMapper->unregisterShadowCaster(SHADOW_REGULAR, m_uboData.Shadow_Spot1);
			m_shadowMapper->unregisterShadowCaster(SHADOW_REGULAR, m_uboData.Shadow_Spot2);
		}
		if (m_world) {
			m_world->unregisterViewer(&m_camera[0]);
			m_world->unregisterViewer(&m_camera[1]);
		}
	}
}

Light_Point_Component::Light_Point_Component(const ECShandle & id, const ECShandle & pid, EnginePackage *enginePackage) : Lighting_Component(id, pid)
{
	m_enginePackage = enginePackage;
	m_uboID = 0;
	m_squaredRadius = 0;
	glCreateBuffers(1, &m_uboID);
	glNamedBufferData(m_uboID, sizeof(LightPointBuffer), &m_uboData, GL_DYNAMIC_COPY);

	m_camera[0].setHorizontalFOV(180);
	m_camera[1].setHorizontalFOV(180);

	if (m_enginePackage->findSubSystem("Graphics")) {
		m_shadowMapper = &m_enginePackage->getSubSystem<System_Graphics>("Graphics")->m_shadowBuffer;
		m_shadowMapper->registerShadowCaster(SHADOW_REGULAR, m_uboData.Shadow_Spot1);
		m_shadowMapper->registerShadowCaster(SHADOW_REGULAR, m_uboData.Shadow_Spot2);
	}
	if (m_enginePackage->findSubSystem("World")) {
		m_world = m_enginePackage->getSubSystem<System_World>("World");
		m_world->registerViewer(&m_camera[0]);
		m_world->registerViewer(&m_camera[1]);
	}
}

void Light_Point_Component::receiveMessage(const ECSmessage &message)
{
	if (Component::compareMSGSender(message)) return;
	glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);

	switch (message.GetCommandID()) {
	case SET_LIGHT_COLOR: {
		if (!message.IsOfType<vec3>()) break;
		const auto &payload = message.GetPayload<vec3>();
		m_uboData.LightColor = payload;
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(LightPointBuffer, LightColor), sizeof(vec3), &m_uboData.LightColor);
		break;
	}
	case SET_LIGHT_INTENSITY: {
		if (!message.IsOfType<float>()) break;
		const auto &payload = message.GetPayload<float>();
		m_uboData.LightIntensity = payload;
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(LightPointBuffer, LightIntensity), sizeof(float), &m_uboData.LightIntensity);
		break;
	}
	case SET_LIGHT_RADIUS: {
		if (!message.IsOfType<float>()) break;
		const auto &payload = message.GetPayload<float>();
		m_squaredRadius = payload * payload;
		m_uboData.p_far = m_squaredRadius;
		m_uboData.LightRadius = payload;
		m_camera[0].setFarPlane(m_squaredRadius);
		m_camera[1].setFarPlane(m_squaredRadius);
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(LightPointBuffer, p_far), sizeof(float), &m_uboData.p_far);
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(LightPointBuffer, LightRadius), sizeof(float), &m_uboData.LightRadius);
		update();
		break;
	}
	case SET_POSITION: {
		if (!message.IsOfType<vec3>()) break;
		const auto &payload = message.GetPayload<vec3>();
		m_uboData.LightPosition = payload;
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(LightPointBuffer, LightPosition), sizeof(float), &m_uboData.LightPosition);
		update();
		break;
	}
	case SET_TRANSFORM: {
		if (!message.IsOfType<Transform>()) break;
		const auto &payload = message.GetPayload<Transform>();
		m_uboData.LightPosition = payload.m_position;
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(LightPointBuffer, LightPosition), sizeof(float), &m_uboData.LightPosition);
		update();
		break;
	}
	}

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Light_Point_Component::directPass(const int & vertex_count)
{
	glBindBufferBase(GL_UNIFORM_BUFFER, 6, m_uboID);

	glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
	m_uboData.LightStencil = 1;
	glBufferSubData(GL_UNIFORM_BUFFER, offsetof(LightPointBuffer, LightStencil), 4, &m_uboData.LightStencil);

	// Draw only into depth-stencil buffer
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glClear(GL_STENCIL_BUFFER_BIT);
	glStencilFunc(GL_ALWAYS, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, vertex_count);

	// Now draw into color buffers
	m_uboData.LightStencil = 0;
	glBufferSubData(GL_UNIFORM_BUFFER, offsetof(LightPointBuffer, LightStencil), 4, &m_uboData.LightStencil);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
	glDrawArrays(GL_TRIANGLES, 0, vertex_count);
}

void Light_Point_Component::indirectPass()
{
	glBindBufferBase(GL_UNIFORM_BUFFER, 6, m_uboID);
	glDrawArraysIndirect(GL_POINTS, 0);
}

void Light_Point_Component::shadowPass()
{
	update();
	glBindBufferBase(GL_UNIFORM_BUFFER, 6, m_uboID);
	m_shadowMapper->clearShadow(SHADOW_REGULAR, getShadowSpot(false));
	m_shadowMapper->clearShadow(SHADOW_REGULAR, getShadowSpot(true));

	for (int x = 0; x < 2; x++) {
		m_uboData.p_dir = (float(x) * 2.0f) - 1.0f;;
		glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(LightPointBuffer, p_dir), 4, &m_uboData.p_dir);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		const Visibility_Token vis_token = m_camera[x].getVisibilityToken();
		for each (auto &component in vis_token.getTypeList<Geometry_Component>("Anim_Model"))
			component->draw();
	}

	m_shadowUpdateTime = glfwGetTime();
}

bool Light_Point_Component::isVisible(const mat4 & PMatrix, const mat4 &VMatrix)
{
	Frustum frustum(PMatrix * VMatrix * m_uboData.lightV);
	return frustum.sphereInFrustum(m_uboData.LightPosition, vec3(m_squaredRadius));
}

float Light_Point_Component::getImportance(const vec3 & position) const
{
	return m_uboData.LightRadius / glm::length(position - m_uboData.LightPosition);
}

void Light_Point_Component::update()
{
	// Calculate view matrix
	const mat4 trans = glm::translate(mat4(1.0f), m_uboData.LightPosition);
	const mat4 scl = glm::scale(mat4(1.0f), vec3(m_squaredRadius));
	m_uboData.lightV = glm::translate(mat4(1.0f), -m_uboData.LightPosition);
	m_uboData.mMatrix = trans * scl;

	// Calculate perspective matrix
	const vec2 &size = m_shadowMapper->getSize(SHADOW_REGULAR);
	m_uboData.ShadowSize = size.x;

	for (int x = 0; x < 2; ++x) {
		m_camera[x].setPosition(m_uboData.LightPosition);
		m_camera[x].setDimensions(size);
		m_camera[x].setOrientation(glm::rotate(quat(1, 0, 0, 0), glm::radians(180.0f * x), vec3(0, 1, 0)));
		m_camera[x].update();
	}

	glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightPointBuffer), &m_uboData);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 6, m_uboID);
}

GLuint Light_Point_Component::getShadowSpot(const bool & front) const
{
	return front ? m_uboData.Shadow_Spot1 : m_uboData.Shadow_Spot2;
}