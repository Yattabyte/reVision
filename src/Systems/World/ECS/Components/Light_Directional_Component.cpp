#include "Systems\World\ECS\Components\Light_Directional_Component.h"
#include "Systems\World\ECS\Components\Geometry_Component.h"
#include "Systems\World\ECS\ECSmessage.h"
#include "Systems\World\World.h"
#include "Engine.h"
#include "Systems\Graphics\Graphics.h"
#include "Systems\Graphics\Resources\Lights\Directional.h"
#include "GLFW\glfw3.h"


Light_Directional_Component::~Light_Directional_Component()
{
	m_directionalTech->unregisterShadowCaster(m_shadowSpot);
	m_engine->getSubSystem<System_World>("World")->unregisterViewer(&m_camera);
	m_engine->getSubSystem<System_Graphics>("Graphics")->m_lightBuffers.m_lightDirSSBO.removeElement(&m_uboIndex);
}

Light_Directional_Component::Light_Directional_Component(Engine *engine)
{
	m_engine = engine;
	m_visSize[0] = 0;
	m_visSize[1] = 0;
	m_mMatrix = glm::mat4(1.0f);

	float near_plane = -0.1f;
	float far_plane = - m_engine->getPreference(PreferenceState::C_DRAW_DISTANCE);
	m_cascadeEnd[0] = near_plane;
	for (int x = 1; x < NUM_CASCADES + 1; ++x) {
		float cLog = near_plane * powf((far_plane / near_plane), (float(x) / float(NUM_CASCADES)));
		float cUni = near_plane + ((far_plane - near_plane) * x / NUM_CASCADES);
		float lambda = 0.3f;
		m_cascadeEnd[x] = (lambda*cLog) + ((1 - lambda)*cUni);
	}

	auto graphics = m_engine->getSubSystem<System_Graphics>("Graphics");
	m_directionalTech = graphics->getBaseTech<Directional_Tech>("Directional_Tech");
	m_uboBuffer = graphics->m_lightBuffers.m_lightDirSSBO.addElement(&m_uboIndex);
	Directional_Struct * uboData = &reinterpret_cast<Directional_Struct*>(m_uboBuffer->pointer)[m_uboIndex];

	m_directionalTech->registerShadowCaster(m_shadowSpot);
	uboData->Shadow_Spot = m_shadowSpot;
	
	m_shadowSize = m_engine->getPreference(PreferenceState::C_SHADOW_SIZE_DIRECTIONAL);
	uboData->ShadowSize_Recip = 1.0f / m_shadowSize;
		
	m_engine->getSubSystem<System_World>("World")->registerViewer(&m_camera);	
	m_commandMap["Set_Light_Color"] = [&](const ECS_Command & payload) {
		if (payload.isType<glm::vec3>()) setColor(payload.toType<glm::vec3>());
	};
	m_commandMap["Set_Light_Intensity"] = [&](const ECS_Command & payload) {
		if (payload.isType<float>()) setIntensity(payload.toType<float>());
	};
	m_commandMap["Set_Transform"] = [&](const ECS_Command & payload) {
		if (payload.isType<Transform>()) setTransform(payload.toType<Transform>());
	};
}

void Light_Directional_Component::setColor(const glm::vec3 & color)
{
	(&reinterpret_cast<Directional_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->LightColor = color;
}

void Light_Directional_Component::setIntensity(const float & intensity)
{
	(&reinterpret_cast<Directional_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->LightIntensity = intensity;
}

void Light_Directional_Component::setTransform(const Transform & transform)
{
	const glm::mat4 &rotation = transform.m_modelMatrix;
	(&reinterpret_cast<Directional_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->LightDirection = glm::normalize(rotation * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)).xyz;
	m_mMatrix = glm::inverse(rotation * glm::mat4_cast(glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 1.0f, 0))));
	(&reinterpret_cast<Directional_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->lightV = m_mMatrix;
}

bool Light_Directional_Component::isVisible(const float & radius, const glm::vec3 & eyePosition) const
{
	// Directional lights are infinite as they simulate the sun.
	return true;
}

void Light_Directional_Component::occlusionPass(const unsigned int & type)
{
	if (m_visSize[type]) {
		glUniform1i(0, getBufferIndex());
		const auto &visBuffers = m_camera.getVisibilityBuffers();
		visBuffers.m_buffer_Index[type].bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		visBuffers.m_buffer_Culling[type].bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		visBuffers.m_buffer_Render[type].bindBufferBase(GL_SHADER_STORAGE_BUFFER, 7);
		for (int x = 0; x < 4; ++x) {
			glUniform1i(1, x);
			glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_visSize[type], 0);
		}
	}
}

void Light_Directional_Component::shadowPass(const unsigned int & type)
{
	if (m_visSize[type]) {
		// Clear out the shadows
		if (type == CAM_GEOMETRY_DYNAMIC)
			m_directionalTech->clearShadow(m_shadowSpot);

		glUniform1i(0, getBufferIndex());

		// Draw render lists		
		glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
		const auto &visBuffers = m_camera.getVisibilityBuffers();
		visBuffers.m_buffer_Index[type].bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		visBuffers.m_buffer_Render[type].bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		for (int x = 0; x < 4; ++x) {
			glUniform1i(1, x);
			glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_visSize[type], 0);
		}

		m_shadowUpdateTime = glfwGetTime();
	}
}

float Light_Directional_Component::getImportance(const glm::vec3 & position) const
{
	return 1.0f;
}

#include "Systems\Graphics\Resources\Geometry Techniques\Model_Technique.h"
#include "Systems\Graphics\Resources\Geometry Techniques\Model_Static_Technique.h"
void Light_Directional_Component::update(const unsigned int & type)
{
	calculateCascades();

	// Update render lists
	const char * string_type;
	switch (type) {
		case CAM_GEOMETRY_DYNAMIC:
			Model_Technique::writeCameraBuffers(m_camera);
			string_type = "Anim_Model";
			break;
		case CAM_GEOMETRY_STATIC:
			Model_Static_Technique::writeCameraBuffers(m_camera);
			string_type = "Static_Model";
			break;
	}
	m_visSize[type] = m_camera.getVisibilityToken().specificSize(string_type);
}

void Light_Directional_Component::calculateCascades()
{
	Directional_Struct * uboData = &reinterpret_cast<Directional_Struct*>(m_uboBuffer->pointer)[m_uboIndex];
	const auto cameraBuffer = m_engine->getCamera()->getCameraBuffer(); // returns a copy, no need to std::mutex
	const glm::mat4 CamInv = glm::inverse(cameraBuffer.vMatrix);
	m_camera.setPosition(cameraBuffer.EyePosition);
	m_camera.setFarPlane(cameraBuffer.FarPlane);

	const glm::vec2 &size = cameraBuffer.Dimensions;
	float ar = size.x / size.y;
	float tanHalfHFOV = glm::radians(cameraBuffer.FOV) / 2.0f;
	float tanHalfVFOV = atanf(tanf(tanHalfHFOV) / ar);

	for (int i = 0; i < NUM_CASCADES; i++) {
		float points[4] = { m_cascadeEnd[i] * tanHalfHFOV,
			m_cascadeEnd[i + 1] * tanHalfHFOV,
			m_cascadeEnd[i] * tanHalfVFOV,
			m_cascadeEnd[i + 1] * tanHalfVFOV };

		glm::vec3 frustumCorners[8] = {
			// near face
			glm::vec3(points[0], points[2], m_cascadeEnd[i]),
			glm::vec3(-points[0], points[2], m_cascadeEnd[i]),
			glm::vec3(points[0], -points[2], m_cascadeEnd[i]),
			glm::vec3(-points[0], -points[2], m_cascadeEnd[i]),
			// far face
			glm::vec3(points[1], points[3], m_cascadeEnd[i + 1]),
			glm::vec3(-points[1], points[3], m_cascadeEnd[i + 1]),
			glm::vec3(points[1], -points[3], m_cascadeEnd[i + 1]),
			glm::vec3(-points[1], -points[3], m_cascadeEnd[i + 1])
		};

		// Find the middle of current view frustum chunk
		glm::vec3 middle(0, 0, ((m_cascadeEnd[i + 1] - m_cascadeEnd[i]) / 2.0f) + m_cascadeEnd[i]);

		// Measure distance from middle to the furthest point of frustum slice
		// Use to make a bounding sphere, but then convert into a bounding box
		float radius = glm::length(frustumCorners[7] - middle);
		glm::vec3 aabb(radius);

		const glm::vec3 volumeUnitSize = (aabb - -aabb) / m_shadowSize;
		const glm::vec3 frustumpos = (m_mMatrix * CamInv * glm::vec4(middle, 1.0f)).xyz;
		const glm::vec3 clampedPos = glm::floor((frustumpos + (volumeUnitSize / 2.0f)) / volumeUnitSize) * volumeUnitSize;
		glm::vec3 newMin = -aabb + clampedPos;
		glm::vec3 newMax = aabb + clampedPos;

		float l = newMin.x, r = newMax.x, b = newMax.y, t = newMin.y, n = -newMin.z, f = -newMax.z;
		const glm::mat4 pMatrix = glm::ortho(l, r, b, t, n, f);
		const glm::mat4 pvMatrix = pMatrix * m_mMatrix;
		uboData->lightVP[i] = pvMatrix;
		uboData->inverseVP[i] = inverse(pvMatrix);

		if (i == 0)
			m_camera.setMatrices(pMatrix, glm::mat4(1.0f));
	}

	for (int x = 0; x < NUM_CASCADES; ++x) {
		const glm::vec4 v1 = glm::vec4(0, 0, m_cascadeEnd[x + 1], 1.0f);
		const glm::vec4 v2 = cameraBuffer.pMatrix * v1;
		uboData->CascadeEndClipSpace[x] = v2.z;
	}
}