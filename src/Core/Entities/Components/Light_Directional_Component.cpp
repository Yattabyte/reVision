#include "Entities\Components\Light_Directional_Component.h"
#include "Systems\ECS\ECSmessage.h"
#include "Systems\ECS\ECSmessages.h"
#include "Utilities\Frustum.h"

Light_Directional_Component::~Light_Directional_Component()
{
	glDeleteBuffers(1, &m_uboID);
}

Light_Directional_Component::Light_Directional_Component(const ECSHandle & id, const ECSHandle & pid) : Lighting_Component(id, pid)
{
	m_uboID = 0;
	glGenBuffers(1, &m_uboID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(LightBuffer), &m_uboData, GL_DYNAMIC_COPY);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Light_Directional_Component::ReceiveMessage(ECSmessage * message)
{
	glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);

	/*if (message->IsOfType<vec3>()) {
		const auto &payload = *(vec3*)(message->GetPayload());
		m_uboData.LightColor = payload;
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4x4), sizeof(vec3), &m_uboData.LightColor);
	}
	else if (message->IsOfType<float>()) {
		const auto &payload = *(float*)(message->GetPayload());
		m_uboData.LightIntensity = payload;
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4x4) + sizeof(vec4), sizeof(float), &m_uboData.LightIntensity);
	}
	else if (message->IsOfType<quat>()) {
		const auto &payload = *(quat*)(message->GetPayload());
		const mat4 rotation = glm::mat4_cast(payload);
		m_uboData.LightDirection = glm::normalize(rotation * vec4(1.0f, 0.0f, 0.0f, 0.0f)).xyz;
		m_uboData.lightV = glm::inverse(rotation * glm::mat4_cast(glm::rotate(quat(1, 0, 0, 0), glm::radians(90.0f), vec3(0, 1.0f, 0))));
		Update();
	}
	else if (message->IsOfType<Transform>()) {
		const auto &payload = *(Transform*)(message->GetPayload());
		const mat4 &rotation = payload.modelMatrix;
		m_uboData.LightDirection = glm::normalize(rotation * vec4(1.0f, 0.0f, 0.0f, 0.0f)).xyz;
		m_uboData.lightV = glm::inverse(rotation * glm::mat4_cast(glm::rotate(quat(1, 0, 0, 0), glm::radians(90.0f), vec3(0, 1.0f, 0))));
		Update();
	}*/
	
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Light_Directional_Component::directPass(const int & vertex_count)
{
	glBindBufferBase(GL_UNIFORM_BUFFER, 6, m_uboID);
	glDrawArrays(GL_TRIANGLES, 0, vertex_count);
}

void Light_Directional_Component::indirectPass(const int & vertex_count)
{
	glBindBufferBase(GL_UNIFORM_BUFFER, 6, m_uboID);
	glDrawArraysInstanced(GL_POINTS, 0, 1, vertex_count);
}

void Light_Directional_Component::shadowPass(const Visibility_Token & vis_token) const
{
	glBindBufferBase(GL_UNIFORM_BUFFER, 6, m_uboID);

	/*	for each (auto vec in vis_token.visible_geometry)
	for each (auto obj in vec.second)
	obj->geometryPass();*/
}

bool Light_Directional_Component::IsVisible(const mat4 & PVMatrix)
{
	// Directional lights are infinite as they simulate the sun.
	return true;
}

void Light_Directional_Component::Update()
{
	glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightBuffer), &m_uboData);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 6, m_uboID);
}

