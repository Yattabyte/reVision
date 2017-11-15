#include "Entities\Light_Directional.h"
#include "Systems\Lighting_Manager.h"
#include "Systems\Shadowmap_Manager.h"
#include "Rendering\Visibility_Token.h"

Light_Directional::~Light_Directional()
{
	glDeleteBuffers(1, &m_UBO);
}

Light_Directional::Light_Directional(const vec3 & rgb, const float & ints, const bool & use_shadows)
{
	m_lightBuffer.Use_Shadows = use_shadows; 
	setColor(rgb);
	setIntensity(ints);

	glGenBuffers(1, &m_UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(LightBuffer), &m_lightBuffer, GL_DYNAMIC_COPY);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);	
	
	Update();
}

Light_Directional::Light_Directional(const Light_Directional & other)
{
	m_lightBuffer = other.m_lightBuffer;
	m_orientation = other.getOrientation();

	glGenBuffers(1, &m_UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(LightBuffer), &m_lightBuffer, GL_DYNAMIC_COPY);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	Update();
}

void Light_Directional::operator=(const Light_Directional & other)
{
	m_lightBuffer = other.m_lightBuffer;
	m_orientation = other.getOrientation();

	Update();
}

void Light_Directional::registerSelf()
{
	Lighting_Manager::RegisterLight(GetLightType(), this);
	if (m_lightBuffer.Use_Shadows)
		for (int x = 0; x < NUM_CASCADES; ++x)
			Shadowmap_Manager::RegisterShadowCaster(SHADOW_LARGE, m_lightBuffer.Shadow_Spot[x].x);
}

void Light_Directional::unregisterSelf()
{
	Lighting_Manager::UnRegisterLight(GetLightType(), this);
	if (m_lightBuffer.Use_Shadows)
		for (int x = 0; x < NUM_CASCADES; ++x)
			Shadowmap_Manager::UnRegisterShadowCaster(SHADOW_LARGE, m_lightBuffer.Shadow_Spot[x].x);
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

bool Light_Directional::shouldRender(const mat4 & PVMatrix)
{
	// Directional lights are infinite as they simulate the sun.
	return true; 
}

void Light_Directional::shadowPass(const Visibility_Token & vis_token) const
{
	glBindBufferBase(GL_UNIFORM_BUFFER, 6, m_UBO);

	for each (auto vec in vis_token.visible_geometry)
		for each (auto obj in vec.second)
			obj->geometryPass();
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
