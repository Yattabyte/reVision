#include "ECS\Components\Light_Spot.h"
#include "ECS\Components\Geometry.h"
#include "ECS\ECSmessage.h"
#include "Systems\World\World.h"
#include "Engine.h"
#include "Systems\Graphics\Graphics.h"
#include "Systems\Graphics\Resources\Lights\Spot.h"
#include "GLFW\glfw3.h"
#include <math.h>


Light_Spot_C::~Light_Spot_C()
{
	m_spotTech->unregisterShadowCaster(m_shadowSpot);
	m_world->unregisterViewer(&m_camera);
	m_engine->getSubSystem<System_Graphics>("Graphics")->m_lightBuffers.m_lightSpotSSBO.removeElement(&m_uboIndex);
}

Light_Spot_C::Light_Spot_C(Engine * engine, const glm::vec3 & color, const glm::vec2 &intensity_radius, const float & cutoff, const Transform & transform)
	: Lighting_C(engine)
{
	// Default Parameters
	m_squaredRadius = 0;
	m_orientation = glm::quat(1, 0, 0, 0);
	m_lightPos = glm::vec3(0.0f);
	m_visSize[0] = 0; 
	m_visSize[1] = 0;

	// Acquire and update buffers
	auto graphics = m_engine->getSubSystem<System_Graphics>("Graphics");
	m_spotTech = graphics->getBaseTech<Spot_Tech>("Spot_Tech");
	m_uboBuffer = graphics->m_lightBuffers.m_lightSpotSSBO.addElement(&m_uboIndex);
	m_spotTech->registerShadowCaster(m_shadowSpot);	
	m_world = m_engine->getSubSystem<System_World>("World");
	m_world->registerViewer(&m_camera);	
	Spot_Struct * uboData = &reinterpret_cast<Spot_Struct*>(m_uboBuffer->pointer)[m_uboIndex];
	uboData->Shadow_Spot = m_shadowSpot;
	uboData->ShadowSize_Recip = 1.0f / m_spotTech->getSize().x;
	m_camera.setDimensions(m_spotTech->getSize());

	// Register Commands
	m_commandMap["Set_Light_Color"] = [&](const ECS_Command & payload) {
		if (payload.isType<glm::vec3>()) setColor(payload.toType<glm::vec3>());
	};
	m_commandMap["Set_Light_Intensity"] = [&](const ECS_Command & payload) {
		if (payload.isType<float>()) setIntensity(payload.toType<float>());
	};
	m_commandMap["Set_Light_Radius"] = [&](const ECS_Command & payload) {
		if (payload.isType<float>()) setRadius(payload.toType<float>());
	};
	m_commandMap["Set_Light_Cutoff"] = [&](const ECS_Command & payload) {
		if (payload.isType<float>()) setCutoff(payload.toType<float>());
	};
	m_commandMap["Set_Transform"] = [&](const ECS_Command & payload) {
		if (payload.isType<Transform>()) setTransform(payload.toType<Transform>());
	};

	// Update with passed parameters
	setColor(color);
	setIntensity(intensity_radius.x);
	setRadius(intensity_radius.y);
	setCutoff(cutoff);
	setTransform(transform);
}

Light_Spot_C * Light_Spot_C::Create(const ArgumentList & argumentList)
{
	return new Light_Spot_C(
		argumentList.checkParameter<Engine>(0) ? (Engine*)argumentList.dataPointers[0] : nullptr,
		argumentList.checkParameter<glm::vec3>(1) ? *(glm::vec3*)argumentList.dataPointers[1] : glm::vec3(1.0f),
		argumentList.checkParameter<glm::vec2>(2) ? *(glm::vec2*)argumentList.dataPointers[2] : glm::vec2(1.0f),
		argumentList.checkParameter<glm::vec2>(3) ? *(float*)argumentList.dataPointers[3] : float(1.0f),
		argumentList.checkParameter<Transform>(4) ? *(Transform*)argumentList.dataPointers[4] : Transform()
	);
}

void Light_Spot_C::setColor(const glm::vec3 & color)
{
	(&reinterpret_cast<Spot_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->LightColor = color;
}

void Light_Spot_C::setIntensity(const float & intensity)
{
	(&reinterpret_cast<Spot_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->LightIntensity = intensity;
}

void Light_Spot_C::setRadius(const float & radius)
{
	m_radius = radius;
	m_squaredRadius = radius * radius;
	(&reinterpret_cast<Spot_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->LightRadius = radius;
	m_camera.setFarPlane(m_squaredRadius);
	updateViews();
}

void Light_Spot_C::setCutoff(const float & cutoff)
{
	(&reinterpret_cast<Spot_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->LightCutoff = cosf(glm::radians(cutoff));
	m_camera.setHorizontalFOV(cutoff * 2.0f);
	updateViews();
}

void Light_Spot_C::setTransform(const Transform & transform)
{
	Spot_Struct * uboData = &reinterpret_cast<Spot_Struct*>(m_uboBuffer->pointer)[m_uboIndex];
	uboData->LightPosition = transform.m_position;
	m_lightPos = transform.m_position;
	m_orientation = transform.m_orientation;
	m_camera.setPosition(transform.m_position);

	// Recalculate view matrix
	const glm::mat4 trans = glm::translate(glm::mat4(1.0f), m_lightPos);
	const glm::mat4 rot = glm::mat4_cast(m_orientation);
	const glm::mat4 scl = glm::scale(glm::mat4(1.0f), glm::vec3(m_squaredRadius));
	const glm::mat4 final = glm::inverse(trans * rot * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0, 1, 0)));
	m_lightVMatrix = final;
	uboData->lightV = final;
	uboData->LightDirection = (rot * glm::vec4(1, 0, 0, 0)).xyz;
	uboData->mMatrix = (trans * rot) * scl;

	updateViews();
}

void Light_Spot_C::updateViews()
{
	// Recalculate perspective matrix
	m_camera.update();
	const glm::mat4 lightP = m_camera.getCameraBuffer().pMatrix;
	const glm::mat4 lightPV = lightP * m_lightVMatrix;
	(&reinterpret_cast<Spot_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->lightPV = lightPV;
	(&reinterpret_cast<Spot_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->inversePV = glm::inverse(lightPV);
	m_camera.setMatrices(lightP, m_lightVMatrix);
}

bool Light_Spot_C::isVisible(const float & radius, const glm::vec3 & eyePosition) const
{
	const float distance = glm::distance(m_lightPos, eyePosition);
	return radius + m_radius > distance;
}

void Light_Spot_C::occlusionPass(const unsigned int & type)
{
	if (m_visSize[type]) {
		glUniform1i(0, getBufferIndex());
		const auto &visBuffers = m_camera.getVisibilityBuffers();
		visBuffers.m_buffer_Index[type].bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		visBuffers.m_buffer_Culling[type].bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		visBuffers.m_buffer_Render[type].bindBufferBase(GL_SHADER_STORAGE_BUFFER, 7);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_visSize[type], 0);
	}
}

void Light_Spot_C::shadowPass(const unsigned int & type)
{
	if (m_visSize[type]) {
		// Clear out the shadows
		m_spotTech->clearShadow(type, m_shadowSpot);
		glUniform1i(0, getBufferIndex());

		// Draw render lists
		glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
		const auto &visBuffers = m_camera.getVisibilityBuffers();
		visBuffers.m_buffer_Index[type].bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		visBuffers.m_buffer_Render[type].bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_visSize[type], 0);

		m_shadowUpdateTime = glfwGetTime();
	}
}

float Light_Spot_C::getImportance(const glm::vec3 & position) const
{
	return m_radius / glm::length(position - m_lightPos);
}

#include "ECS\Components\Model_Animated.h"
#include "ECS\Components\Model_Static.h"
#include "Systems\Graphics\Resources\Geometry Techniques\Model_Technique.h"
#include "Systems\Graphics\Resources\Geometry Techniques\Model_Static_Technique.h"
void Light_Spot_C::update(const unsigned int & type)
{
	// Update render lists
	const char * string_type;
	switch (type) {
		case CAM_GEOMETRY_DYNAMIC:
			Model_Technique::writeCameraBuffers(m_camera);
			string_type = Model_Animated_C::GetName();
			break;
		case CAM_GEOMETRY_STATIC:
			Model_Static_Technique::writeCameraBuffers(m_camera);
			string_type = Model_Static_C::GetName();
			break;
	}
	m_visSize[type] = m_camera.getVisibilityToken().specificSize(string_type);
}