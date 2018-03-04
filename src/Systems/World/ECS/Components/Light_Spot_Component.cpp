#include "Systems\World\ECS\Components\Light_Spot_Component.h"
#include "Systems\World\ECS\Components\Geometry_Component.h"
#include "Utilities\EnginePackage.h"
#include "Systems\World\World.h"
#include "Systems\World\ECS\ECSmessage.h"
#include "Utilities\Frustum.h"
#include "Utilities\Transform.h"
#include "Systems\Graphics\Graphics.h"
#include "GLFW\glfw3.h"
#include <math.h>


Light_Spot_Component::~Light_Spot_Component()
{
	glDeleteBuffers(1, &m_uboID);
	if (m_enginePackage) {
		if (m_shadowMapper)
			m_shadowMapper->unregisterShadowCaster(SHADOW_REGULAR, m_uboData.Shadow_Spot);
		if (m_world)
			m_world->unregisterViewer(&m_camera);
	}
}

Light_Spot_Component::Light_Spot_Component(const ECShandle & id, const ECShandle & pid, EnginePackage *enginePackage) : Lighting_Component(id, pid)
{
	m_enginePackage = enginePackage;
	m_uboID = 0;
	m_squaredRadius = 0;
	m_orientation = quat(1, 0, 0, 0);
	glGenBuffers(1, &m_uboID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(LightSpotBuffer), &m_uboData, GL_DYNAMIC_COPY);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	if (m_enginePackage->findSubSystem("Graphics")) {
		m_shadowMapper = &m_enginePackage->getSubSystem<System_Graphics>("Graphics")->getShadowBuffer();
		m_shadowMapper->registerShadowCaster(SHADOW_REGULAR, m_uboData.Shadow_Spot);
	}
	if (m_enginePackage->findSubSystem("World")) {
		m_world = m_enginePackage->getSubSystem<System_World>("World");
		m_world->registerViewer(&m_camera);
	}
}

void Light_Spot_Component::receiveMessage(const ECSmessage &message)
{
	if (Component::compareMSGSender(message)) return;
	glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);

	switch (message.GetCommandID()) {
	case SET_LIGHT_COLOR: {
		if (!message.IsOfType<vec3>()) break;
		const auto &payload = message.GetPayload<vec3>();
		m_uboData.LightColor = payload;
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(LightSpotBuffer, LightColor), sizeof(vec3), &m_uboData.LightColor);
		break;
	}
	case SET_LIGHT_INTENSITY: {
		if (!message.IsOfType<float>()) break;
		const auto &payload = message.GetPayload<float>();
		m_uboData.LightIntensity = payload;
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(LightSpotBuffer, LightIntensity), sizeof(float), &m_uboData.LightIntensity);
		break;
	}
	case SET_LIGHT_RADIUS: {
		if (!message.IsOfType<float>()) break;
		const auto &payload = message.GetPayload<float>();
		m_uboData.LightRadius = payload;
		m_squaredRadius = payload * payload;
		m_camera.setFarPlane(m_squaredRadius);
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(LightSpotBuffer, LightRadius), sizeof(float), &m_uboData.LightRadius);
		update();
		break;
	}
	case SET_LIGHT_CUTOFF: {
		if (!message.IsOfType<float>()) break;
		const auto &payload = message.GetPayload<float>();
		m_uboData.LightCutoff = cosf(glm::radians(payload));
		m_camera.setHorizontalFOV(payload * 2.0f);
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(LightSpotBuffer, LightCutoff), sizeof(float), &m_uboData.LightCutoff);
		update();
		break;
	}
	case SET_ORIENTATION: {
		if (!message.IsOfType<quat>()) break;
		const auto &payload = message.GetPayload<quat>();
		m_orientation = payload;
		update();
		break;
	}
	case SET_POSITION: {
		if (!message.IsOfType<vec3>()) break;
		const auto &payload = message.GetPayload<vec3>();
		m_uboData.LightPosition = payload;
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(LightSpotBuffer, LightPosition), sizeof(float), &m_uboData.LightPosition);
		update();
		break;
	}
	case SET_TRANSFORM: {
		if (!message.IsOfType<Transform>()) break;
		const auto &payload = message.GetPayload<Transform>();
		m_uboData.LightPosition = payload.m_position;
		m_orientation = payload.m_orientation;
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(LightSpotBuffer, LightPosition), sizeof(float), &m_uboData.LightPosition);
		update();
		break;
	}
	}

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Light_Spot_Component::directPass(const int & vertex_count)
{
	glBindBufferBase(GL_UNIFORM_BUFFER, 6, m_uboID);

	glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
	m_uboData.LightStencil = 1;
	glBufferSubData(GL_UNIFORM_BUFFER, offsetof(LightSpotBuffer, LightStencil), 4, &m_uboData.LightStencil);

	// Draw only into depth-stencil buffer
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glClear(GL_STENCIL_BUFFER_BIT);
	glStencilFunc(GL_ALWAYS, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, vertex_count);

	// Now draw into color buffers
	m_uboData.LightStencil = 0;
	glBufferSubData(GL_UNIFORM_BUFFER, offsetof(LightSpotBuffer, LightStencil), 4, &m_uboData.LightStencil);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
	glDrawArrays(GL_TRIANGLES, 0, vertex_count);
}

void Light_Spot_Component::indirectPass(const int & vertex_count)
{
	glBindBufferBase(GL_UNIFORM_BUFFER, 6, m_uboID);
	glDrawArraysInstanced(GL_POINTS, 0, 1, vertex_count);
}

void Light_Spot_Component::shadowPass()
{
	update();
	glBindBufferBase(GL_UNIFORM_BUFFER, 6, m_uboID);
	m_shadowMapper->clearShadow(SHADOW_REGULAR, getShadowSpot());

	const Visibility_Token vis_token = m_camera.getVisibilityToken();
	for each (auto &component in vis_token.getTypeList<Geometry_Component>("Anim_Model"))
		component->draw();

	m_shadowUpdateTime = glfwGetTime();
}

bool Light_Spot_Component::isVisible(const mat4 & PMatrix, const mat4 &VMatrix)
{
	Frustum frustum(PMatrix * VMatrix * m_uboData.lightV);
	return frustum.sphereInFrustum(m_uboData.LightPosition, vec3(m_squaredRadius));
}

float Light_Spot_Component::getImportance(const vec3 & position)
{
	return m_uboData.LightRadius / glm::length(position - m_uboData.LightPosition);
}

void Light_Spot_Component::update()
{
	// Calculate view matrix
	const mat4 trans = glm::translate(mat4(1.0f), m_uboData.LightPosition);
	const mat4 rot = glm::mat4_cast(m_orientation);
	const mat4 scl = glm::scale(mat4(1.0f), vec3(m_squaredRadius));
	const mat4 final = glm::inverse(trans * rot * glm::rotate(mat4(1.0f), glm::radians(-90.0f), vec3(0, 1, 0)));
	m_uboData.LightDirection = (rot * vec4(1, 0, 0, 0)).xyz;
	m_uboData.lightV = final;
	m_uboData.mMatrix = (trans * rot) * scl;

	// Calculate perspective matrix
	const vec2 &size = m_shadowMapper->getSize(SHADOW_REGULAR);
	m_uboData.ShadowSize = size.x;
	m_camera.setDimensions(size);
	m_camera.update();
	m_uboData.lightP = m_camera.getCameraBuffer().pMatrix;	
	m_camera.setMatrices(m_uboData.lightP, final);

	glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightSpotBuffer), &m_uboData);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 6, m_uboID);
}

GLuint Light_Spot_Component::getShadowSpot() const
{
	return m_uboData.Shadow_Spot;
}