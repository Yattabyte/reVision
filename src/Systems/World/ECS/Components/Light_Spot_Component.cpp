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
	m_shadowMapper->unregisterShadowCaster(SHADOW_REGULAR, m_shadowSpot);
	m_world->unregisterViewer(&m_camera);	
}

Light_Spot_Component::Light_Spot_Component(const ECShandle & id, const ECShandle & pid, EnginePackage * enginePackage) : Lighting_Component(id, pid)
{
	m_enginePackage = enginePackage;
	m_squaredRadius = 0;
	m_orientation = quat(1, 0, 0, 0);

	auto graphics = m_enginePackage->getSubSystem<System_Graphics>("Graphics");
	m_uboBuffer = graphics->m_lightSpotSSBO.addElement(&m_uboIndex);
	m_shadowMapper = &graphics->m_shadowFBO;
	m_shadowMapper->registerShadowCaster(SHADOW_REGULAR, m_shadowSpot);
	
	m_world = m_enginePackage->getSubSystem<System_World>("World");
	m_world->registerViewer(&m_camera);	

	// Write data to our index spot
	Spot_Struct * uboData = &reinterpret_cast<Spot_Struct*>(m_uboBuffer->pointer)[m_uboIndex];
	uboData->Shadow_Spot = m_shadowSpot;
	uboData->ShadowSize = m_shadowMapper->getSize(SHADOW_REGULAR).x;
}

void Light_Spot_Component::receiveMessage(const ECSmessage &message)
{
	if (Component::compareMSGSender(message)) return;
	Spot_Struct * uboData = &reinterpret_cast<Spot_Struct*>(m_uboBuffer->pointer)[m_uboIndex];

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
			m_camera.setFarPlane(m_squaredRadius);
			uboData->LightRadius = payload;
			update();
			break;
		}
		case SET_LIGHT_CUTOFF: {
			if (!message.IsOfType<float>()) break;
			const auto &payload = message.GetPayload<float>();
			uboData->LightCutoff = cosf(glm::radians(payload));
			m_camera.setHorizontalFOV(payload * 2.0f);
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
			m_lightPos = payload;
			uboData->LightPosition = payload;
			update();
			break;
		}
		case SET_TRANSFORM: {
			if (!message.IsOfType<Transform>()) break;
			const auto &payload = message.GetPayload<Transform>();
			uboData->LightPosition = payload.m_position;
			m_lightPos = payload.m_position;
			update();
			break;
		}
	}
}

void Light_Spot_Component::shadowPass()
{
	update();
	const size_t size = m_camera.getVisibilityToken().specificSize("Anim_Model");
	if (size) {
		m_shadowMapper->clearShadow(SHADOW_REGULAR, m_shadowSpot);
		glUniform1i(0, getBufferIndex());
		m_visGeoUBO.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		m_indirectGeo.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, size, 0);

		m_shadowUpdateTime = glfwGetTime();
	}
}

bool Light_Spot_Component::isVisible(const mat4 & PMatrix, const mat4 &VMatrix)
{
	Frustum frustum(PMatrix * VMatrix * m_lightVMatrix);
	return frustum.sphereInFrustum(m_lightPos, vec3(m_squaredRadius));
}

float Light_Spot_Component::getImportance(const vec3 & position) const
{
	return m_radius / glm::length(position - m_lightPos);
}

#include "Systems\World\ECS\Components\Anim_Model_Component.h"
void Light_Spot_Component::update()
{
	Spot_Struct * uboData = &reinterpret_cast<Spot_Struct*>(m_uboBuffer->pointer)[m_uboIndex];

	// Calculate view matrix
	const mat4 trans = glm::translate(mat4(1.0f), m_lightPos);
	const mat4 rot = glm::mat4_cast(m_orientation);
	const mat4 scl = glm::scale(mat4(1.0f), vec3(m_squaredRadius));
	const mat4 final = glm::inverse(trans * rot * glm::rotate(mat4(1.0f), glm::radians(-90.0f), vec3(0, 1, 0)));
	m_lightVMatrix = final;
	uboData->lightV = final;
	uboData->LightDirection = (rot * vec4(1, 0, 0, 0)).xyz;
	uboData->mMatrix = (trans * rot) * scl;

	// Calculate perspective matrix
	m_camera.setDimensions(m_shadowMapper->getSize(SHADOW_REGULAR));
	m_camera.update();
	const mat4 lightP = m_camera.getCameraBuffer().pMatrix;
	uboData->lightP = lightP;
	m_camera.setMatrices(lightP, final);

	// Update render list
	const Visibility_Token vis_token = m_camera.getVisibilityToken();
	const size_t size = vis_token.specificSize("Anim_Model");
	if (size) {
		struct DrawData {
			GLuint count;
			GLuint instanceCount = 1;
			GLuint first;
			GLuint baseInstance = 0;
			DrawData(const GLuint & c = 0, const GLuint & f = 0) : count(c), first(f) {}
		};
		vector<GLuint> geoArray(size);
		vector<DrawData> drawData(size);
		m_visGeoUBO.checkFence();
		unsigned int count = 0;
		for each (const auto &component in vis_token.getTypeList<Anim_Model_Component>("Anim_Model")) {
			geoArray[count] = component->getBufferIndex();
			const ivec2 drawInfo = component->getDrawInfo();
			drawData[count++] = DrawData(drawInfo.y, drawInfo.x);
		}
		m_visGeoUBO.write(0, sizeof(GLuint)*geoArray.size(), geoArray.data());
		m_indirectGeo.write(0, sizeof(DrawData) * size, drawData.data());
	}
}