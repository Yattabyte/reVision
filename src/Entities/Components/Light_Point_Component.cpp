#include "Entities\Components\Light_Point_Component.h"
#include "Entities\Components\Geometry_Component.h"
#include "Utilities\EnginePackage.h"
#include "Systems\World\World.h"
#include "Systems\World\ECSmessage.h"
#include "Systems\World\ECSmessages.h"
#include "Utilities\Frustum.h"
#include "Utilities\Transform.h"
#include "Systems\Shadows\Shadowmap.h"
#include "GLFW\glfw3.h"
#include <math.h>

Light_Point_Component::~Light_Point_Component()
{
	glDeleteBuffers(1, &m_uboID);
	if (m_enginePackage) {
		if (m_shadowMapper) {
			m_shadowMapper->UnRegisterShadowCaster(SHADOW_REGULAR, m_uboData.Shadow_Spot1);
			m_shadowMapper->UnRegisterShadowCaster(SHADOW_REGULAR, m_uboData.Shadow_Spot2);
		}
		if (m_world) {
			m_world->UnRegisterViewer(&m_camera[0]);
			m_world->UnRegisterViewer(&m_camera[1]);
		}
	}
}

Light_Point_Component::Light_Point_Component(const ECShandle & id, const ECShandle & pid, EnginePackage *enginePackage) : Lighting_Component(id, pid)
{
	m_enginePackage = enginePackage;
	m_uboID = 0;
	m_squaredRadius = 0;
	glGenBuffers(1, &m_uboID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(LightPointBuffer), &m_uboData, GL_DYNAMIC_COPY);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	m_camera[0].setHorizontalFOV(180);
	m_camera[1].setHorizontalFOV(180);

	if (m_enginePackage->findSubSystem("Shadows")) {
		m_shadowMapper = m_enginePackage->getSubSystem<System_Shadowmap>("Shadows");
		m_shadowMapper->RegisterShadowCaster(SHADOW_REGULAR, m_uboData.Shadow_Spot1);
		m_shadowMapper->RegisterShadowCaster(SHADOW_REGULAR, m_uboData.Shadow_Spot2);
	}
	if (m_enginePackage->findSubSystem("World")) {
		m_world = m_enginePackage->getSubSystem<System_World>("World");
		m_world->RegisterViewer(&m_camera[0]);
		m_world->RegisterViewer(&m_camera[1]);
	}
}

void Light_Point_Component::ReceiveMessage(const ECSmessage &message)
{
	if (Component::Am_I_The_Sender(message)) return;
	glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);

	switch (message.GetCommandID()) {
		case SET_COLOR: {
			if (!message.IsOfType<vec3>()) break;
			const auto &payload = message.GetPayload<vec3>();
			m_uboData.LightColor = payload;
			glBufferSubData(GL_UNIFORM_BUFFER, offsetof(LightPointBuffer, LightColor), sizeof(vec3), &m_uboData.LightColor);
			break;
		}
		case SET_INTENSITY: {
			if (!message.IsOfType<float>()) break;
			const auto &payload = message.GetPayload<float>();
			m_uboData.LightIntensity = payload;
			glBufferSubData(GL_UNIFORM_BUFFER, offsetof(LightPointBuffer, LightIntensity), sizeof(float), &m_uboData.LightIntensity);
			break;
		}
		case SET_RADIUS: {
			if (!message.IsOfType<float>()) break;
			const auto &payload = message.GetPayload<float>();
			m_squaredRadius = payload * payload;
			m_uboData.p_far = m_squaredRadius;
			m_uboData.LightRadius = payload;
			m_camera[0].setFarPlane(m_squaredRadius);
			m_camera[1].setFarPlane(m_squaredRadius);
			glBufferSubData(GL_UNIFORM_BUFFER, offsetof(LightPointBuffer, p_far), sizeof(float), &m_uboData.p_far);
			glBufferSubData(GL_UNIFORM_BUFFER, offsetof(LightPointBuffer, LightRadius), sizeof(float), &m_uboData.LightRadius);
			Update();
			break;
		}
		case SET_POSITION: {
			if (!message.IsOfType<vec3>()) break;
			const auto &payload = message.GetPayload<vec3>();
			m_uboData.LightPosition = payload;
			glBufferSubData(GL_UNIFORM_BUFFER, offsetof(LightPointBuffer, LightPosition), sizeof(float), &m_uboData.LightPosition);
			Update();
			break;
		}
		case SET_TRANSFORM: {
			if (!message.IsOfType<Transform>()) break;
			const auto &payload = message.GetPayload<Transform>();
			m_uboData.LightPosition = payload.position;
			glBufferSubData(GL_UNIFORM_BUFFER, offsetof(LightPointBuffer, LightPosition), sizeof(float), &m_uboData.LightPosition);
			Update();
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

void Light_Point_Component::indirectPass(const int & vertex_count)
{
	glBindBufferBase(GL_UNIFORM_BUFFER, 6, m_uboID);
	glDrawArraysInstanced(GL_POINTS, 0, 1, vertex_count);
}

void Light_Point_Component::shadowPass()
{
	Update();
	glBindBufferBase(GL_UNIFORM_BUFFER, 6, m_uboID);
	m_shadowMapper->ClearShadow(SHADOW_REGULAR, getShadowSpot(false));
	m_shadowMapper->ClearShadow(SHADOW_REGULAR, getShadowSpot(true));


	for (int x = 0; x < 2; x++) {
		m_uboData.p_dir = (float(x) * 2.0f) - 1.0f;;
		glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(LightPointBuffer, p_dir), 4, &m_uboData.p_dir);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		shared_lock<shared_mutex> read_guard(m_camera[x].getDataMutex());
		for each (auto &component in m_camera[x].GetVisibilityToken().getTypeList<Geometry_Component>("Anim_Model"))
			component->Draw();
	}

	m_shadowUpdateTime = glfwGetTime();
}

bool Light_Point_Component::IsVisible(const mat4 & PMatrix, const mat4 &VMatrix)
{
	Frustum frustum(PMatrix * VMatrix * m_uboData.lightV);
	return frustum.sphereInFrustum(m_uboData.LightPosition, vec3(m_squaredRadius));
}

float Light_Point_Component::getImportance(const vec3 & position)
{
	return m_uboData.LightRadius / glm::length(position - m_uboData.LightPosition);
}

GLuint Light_Point_Component::getShadowSpot(const bool & front) const
{
	return front ? m_uboData.Shadow_Spot1 : m_uboData.Shadow_Spot2;
}

void Light_Point_Component::Update()
{
	// Calculate view matrix
	const mat4 trans = glm::translate(mat4(1.0f), m_uboData.LightPosition);
	const mat4 scl = glm::scale(mat4(1.0f), vec3(m_squaredRadius));
	m_uboData.lightV = glm::translate(mat4(1.0f), -m_uboData.LightPosition);
	m_uboData.mMatrix = trans * scl;

	// Calculate perspective matrix
	auto shadowmapper = m_enginePackage->getSubSystem<System_Shadowmap>("Shadows");
	const vec2 &size = shadowmapper->GetSize(SHADOW_REGULAR);
	m_uboData.ShadowSize = size.x;

	for (int x = 0; x < 2; ++x) {
		m_camera[x].setPosition(m_uboData.LightPosition);
		m_camera[x].setDimensions(size);
		m_camera[x].setOrientation(glm::rotate(quat(1, 0, 0, 0), glm::radians(180.0f * x), vec3(0, 1, 0)));
		m_camera[x].Update();
	}

	glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightPointBuffer), &m_uboData);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 6, m_uboID);
}

