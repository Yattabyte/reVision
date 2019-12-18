#include "Modules/Graphics/Lighting/Reflector/ReflectorSync_System.h"
#include "Modules/Graphics/Lighting/Reflector/ReflectorData.h"
#include "Modules/ECS/component_types.h"


ReflectorSync_System::ReflectorSync_System(ReflectorData& frameData) noexcept :
	m_frameData(frameData)
{
	addComponentType(Reflector_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
	addComponentType(Transform_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
}

void ReflectorSync_System::updateComponents(const float&, const std::vector<std::vector<ecsBaseComponent*>>& components)
{
	// Resize light buffers to match number of entities this frame
	m_frameData.lightBuffer.resize(components.size());
	m_frameData.lightBuffer.beginWriting();
	int index = 0;
	for (const auto& componentParam : components) {
		auto* reflectorComponent = static_cast<Reflector_Component*>(componentParam[0]);
		const auto* transformComponent = static_cast<Transform_Component*>(componentParam[1]);

		const auto& position = transformComponent->m_worldTransform.m_position;
		const auto& orientation = transformComponent->m_worldTransform.m_orientation;
		const auto& scale = transformComponent->m_worldTransform.m_scale;
		const auto& modelMatrix = transformComponent->m_worldTransform.m_modelMatrix;
		const auto matRot = glm::mat4_cast(orientation);
		const float largest = pow(std::max(std::max(scale.x, scale.y), scale.z), 2.0f);
		m_frameData.lightBuffer[index].mMatrix = modelMatrix;
		m_frameData.lightBuffer[index].rotMatrix = glm::inverse(matRot);
		m_frameData.lightBuffer[index].BoxCamPos = position;
		m_frameData.lightBuffer[index].BoxScale = scale;
		const glm::mat4 pMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, largest);
		const glm::mat4 pMatrixInverse = glm::inverse(pMatrix);
		const glm::mat4 vMatrices[6] = {
			glm::lookAt(position, position + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)),
			glm::lookAt(position, position + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)),
			glm::lookAt(position, position + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
			glm::lookAt(position, position + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)),
			glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)),
			glm::lookAt(position, position + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0))
		};
		reflectorComponent->m_cameras.resize(6);
		reflectorComponent->m_updateTimes.resize(6);
		for (int x = 0; x < 6; ++x) {
			auto& camData = *reflectorComponent->m_cameras[x].get();
			camData.Dimensions = m_frameData.envmapSize;
			camData.FOV = 90.0f;
			camData.FarPlane = largest;
			camData.EyePosition = position;
			camData.pMatrix = pMatrix;
			camData.pMatrixInverse = pMatrixInverse;
			camData.vMatrix = vMatrices[x];
			camData.vMatrixInverse = glm::inverse(vMatrices[x]);
			camData.pvMatrix = pMatrix * vMatrices[x];
			reflectorComponent->m_cameras[x].updateFrustum();
		}

		// Sync Buffer Attributes
		m_frameData.lightBuffer[index].CubeSpot = reflectorComponent->m_cubeSpot;
		index++;
	}
	m_frameData.lightBuffer.endWriting();
}