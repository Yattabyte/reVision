#include "Entities\Light_Directional.h"
#include "Managers\Lighting_Manager.h"

Light_Directional::~Light_Directional()
{
}

Light_Directional::Light_Directional(const vec3 & rgb, const float & ints, const bool & use_shadows)
{
	m_lightBuffer.Use_Shadows = use_shadows; 
	
	glGenBuffers(1, &m_UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(LightBuffer), &m_lightBuffer, GL_DYNAMIC_COPY);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);	

	setColor(rgb);
	setIntensity(ints);
	Update();
}

Light_Directional::Light_Directional(const Light_Directional & other)
{

}

void Light_Directional::operator=(const Light_Directional & other)
{

}

void Light_Directional::registerSelf()
{
	Lighting_Manager::RegisterLight(GetLightType(), this);
}

void Light_Directional::unregisterSelf()
{
	Lighting_Manager::UnRegisterLight(GetLightType(), this);
}

void Light_Directional::directPass(const int & vertex_count)
{
	glBindBufferBase(GL_UNIFORM_BUFFER, 6, m_UBO);
	glDrawArrays(GL_TRIANGLES, 0, vertex_count);
}

void Light_Directional::indirectPass(const int & vertex_count)
{
	glBindBufferBase(GL_UNIFORM_BUFFER, 6, m_UBO);
	glDrawArraysInstanced(GL_POINTS, 0, 1, vertex_count);
}

void Light_Directional::Update()
{
	const mat4 rotation = glm::mat4_cast(m_orientation);
	m_lightBuffer.LightDirection = glm::normalize(rotation * vec4(1.0f, 0.0f, 0.0f, 0.0f)).xyz;
	m_lightBuffer.lightV = glm::inverse(rotation * glm::mat4_cast(glm::rotate(quat(1, 0, 0, 0), glm::radians(90.0f), vec3(0, 1.0f, 0))));

	glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightBuffer), &m_lightBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 6, m_UBO);
}
