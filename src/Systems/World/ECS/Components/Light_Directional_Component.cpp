#include "Systems\World\ECS\Components\Light_Directional_Component.h"
#include "Systems\World\ECS\Components\Geometry_Component.h"
#include "Utilities\EnginePackage.h"
#include "Systems\World\World.h"
#include "Systems\World\ECS\ECSmessage.h"
#include "Utilities\Frustum.h"
#include "Utilities\Transform.h"
#include "Systems\Graphics\Graphics.h"
#include "GLFW\glfw3.h"


Light_Directional_Component::~Light_Directional_Component()
{
	for (int x = 0; x < NUM_CASCADES; ++x)
		m_shadowMapper->unregisterShadowCaster(SHADOW_LARGE, m_shadowSpots[x]);
	m_enginePackage->getSubSystem<System_World>("World")->unregisterViewer(&m_camera);	
}

Light_Directional_Component::Light_Directional_Component(const ECShandle & id, const ECShandle & pid, EnginePackage *enginePackage) : Lighting_Component(id, pid)
{
	m_enginePackage = enginePackage;

	float near_plane = -0.1f;
	float far_plane = - m_enginePackage->getPreference(PreferenceState::C_DRAW_DISTANCE);
	m_cascadeEnd[0] = near_plane;
	for (int x = 1; x < NUM_CASCADES + 1; ++x) {
		float cLog = near_plane * powf((far_plane / near_plane), (float(x) / float(NUM_CASCADES)));
		float cUni = near_plane + ((far_plane - near_plane) * x / NUM_CASCADES);
		float lambda = 0.3f;
		m_cascadeEnd[x] = (lambda*cLog) + ((1 - lambda)*cUni);
	}

	auto graphics = m_enginePackage->getSubSystem<System_Graphics>("Graphics");
	m_uboBuffer = graphics->m_lightDirSSBO.addElement(&m_uboIndex);
	m_shadowMapper = &graphics->m_shadowFBO;
	Directional_Struct * uboData = &reinterpret_cast<Directional_Struct*>(m_uboBuffer->pointer)[m_uboIndex];
	for (int x = 0; x < NUM_CASCADES; ++x) {
		m_shadowMapper->registerShadowCaster(SHADOW_LARGE, m_shadowSpots[x]);
		uboData->Shadow_Spot[x] = m_shadowSpots[x];
	}
		
	if (m_enginePackage->findSubSystem("World"))
		m_enginePackage->getSubSystem<System_World>("World")->registerViewer(&m_camera);
}

void Light_Directional_Component::receiveMessage(const ECSmessage &message)
{
	if (Component::compareMSGSender(message)) return;
	Directional_Struct * uboData = &reinterpret_cast<Directional_Struct*>(m_uboBuffer->pointer)[m_uboIndex];

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
		case SET_ORIENTATION: {
			if (!message.IsOfType<quat>()) break;
			const auto &payload = message.GetPayload<quat>();
			const mat4 rotation = glm::mat4_cast(payload);
			uboData->LightDirection = glm::normalize(rotation * vec4(1.0f, 0.0f, 0.0f, 0.0f)).xyz;
			m_mMatrix = glm::inverse(rotation * glm::mat4_cast(glm::rotate(quat(1, 0, 0, 0), glm::radians(90.0f), vec3(0, 1.0f, 0))));
			uboData->lightV = m_mMatrix;
			update();
			break;
		}
		case SET_TRANSFORM: {
			if (!message.IsOfType<Transform>()) break;
			const auto &payload = message.GetPayload<Transform>();
			const mat4 &rotation = payload.m_modelMatrix;
			uboData->LightDirection = glm::normalize(rotation * vec4(1.0f, 0.0f, 0.0f, 0.0f)).xyz;
			m_mMatrix = glm::inverse(rotation * glm::mat4_cast(glm::rotate(quat(1, 0, 0, 0), glm::radians(90.0f), vec3(0, 1.0f, 0))));
			uboData->lightV = m_mMatrix;
			update();
			break;
		}
	}
}

void Light_Directional_Component::shadowPass()
{
	const size_t size = m_camera.getVisibilityToken().specificSize("Anim_Model");
	if (size) {
		glUniform1i(0, getBufferIndex());

		// Draw render lists
		m_visGeoUBO.bindBufferBase(GL_UNIFORM_BUFFER, 3);
		m_indirectGeo.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		for (int x = 0; x < 4; ++x) {
			glUniform1i(1, x);
			glMultiDrawArraysIndirect(GL_TRIANGLES, 0, size, 0);
		}

		m_shadowUpdateTime = glfwGetTime();
	}
}

bool Light_Directional_Component::isVisible(const mat4 & PMatrix, const mat4 &VMatrix)
{
	// Directional lights are infinite as they simulate the sun.
	return true;
}

float Light_Directional_Component::getImportance(const vec3 & position) const
{
	return 1.0f;
}

#include "Systems\World\ECS\Components\Anim_Model_Component.h"
void Light_Directional_Component::update()
{
	calculateCascades();

	// Update render list
	const Visibility_Token vis_token = m_camera.getVisibilityToken();
	const size_t size = vis_token.specificSize("Anim_Model");
	if (size) {
		// Clear out the shadows which we will update next shadow pass
		for (int x = 0; x < NUM_CASCADES; ++x)
			m_shadowMapper->clearShadow(SHADOW_LARGE, m_shadowSpots[x]);
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
		m_visGeoUBO.write(0, sizeof(ivec4)*geoArray.size(), geoArray.data());
		m_indirectGeo.write(0, sizeof(DrawData) * size, drawData.data());
	}
}

void Light_Directional_Component::calculateCascades()
{
	Directional_Struct * uboData = &reinterpret_cast<Directional_Struct*>(m_uboBuffer->pointer)[m_uboIndex];
	const auto cameraBuffer = m_enginePackage->m_Camera.getCameraBuffer(); // returns a copy, no need to mutex
	const mat4 CamInv = glm::inverse(cameraBuffer.vMatrix);

	const vec2 &size = cameraBuffer.Dimensions;
	float ar = size.x / size.y;
	float tanHalfHFOV = (tanf(glm::radians(cameraBuffer.FOV / 2.0f)));
	float tanHalfVFOV = (tanf(glm::radians((cameraBuffer.FOV / ar) / 2.0f)));
	const float shadowSize = m_enginePackage->getPreference(PreferenceState::C_SHADOW_SIZE_LARGE);
	uboData->ShadowSize = shadowSize;

	for (int i = 0; i < NUM_CASCADES; i++) {
		float points[4] = { m_cascadeEnd[i] * tanHalfHFOV,
			m_cascadeEnd[i + 1] * tanHalfHFOV,
			m_cascadeEnd[i] * tanHalfVFOV,
			m_cascadeEnd[i + 1] * tanHalfVFOV };

		vec3 frustumCorners[8] = {
			// near face
			vec3(points[0], points[2], m_cascadeEnd[i]),
			vec3(-points[0], points[2], m_cascadeEnd[i]),
			vec3(points[0], -points[2], m_cascadeEnd[i]),
			vec3(-points[0], -points[2], m_cascadeEnd[i]),
			// far face
			vec3(points[1], points[3], m_cascadeEnd[i + 1]),
			vec3(-points[1], points[3], m_cascadeEnd[i + 1]),
			vec3(points[1], -points[3], m_cascadeEnd[i + 1]),
			vec3(-points[1], -points[3], m_cascadeEnd[i + 1])
		};

		// Find the middle of current view frustum chunk
		vec3 middle(0, 0, ((m_cascadeEnd[i + 1] - m_cascadeEnd[i]) / 2.0f) + m_cascadeEnd[i]);

		// Measure distance from middle to the furthest point of frustum slice
		// Use to make a bounding sphere, but then convert into a bounding box
		float radius = glm::length(frustumCorners[7] - middle);
		vec3 aabb(radius);

		const vec3 volumeUnitSize = (aabb - -aabb) / shadowSize;
		const vec3 frustumpos = (m_mMatrix * CamInv * vec4(middle, 1.0f)).xyz;
		const vec3 clampedPos = glm::floor((frustumpos + (volumeUnitSize / 2.0f)) / volumeUnitSize) * volumeUnitSize;
		vec3 newMin = -aabb + clampedPos;
		vec3 newMax = aabb + clampedPos;

		float l = newMin.x, r = newMax.x, b = newMax.y, t = newMin.y, n = -newMin.z, f = -newMax.z;
		mat4 pMatrix = glm::ortho(l, r, b, t, n, f);
		uboData->lightP[i] = pMatrix;

		if (i == 0)
			m_camera.setMatrices(pMatrix, mat4(1.0f));
	}

	for (int x = 0; x < NUM_CASCADES; ++x) {
		const vec4 v1 = vec4(0, 0, m_cascadeEnd[x + 1], 1.0f);
		const vec4 v2 = cameraBuffer.pMatrix * v1;
		uboData->CascadeEndClipSpace[x] = v2.z;
	}
}