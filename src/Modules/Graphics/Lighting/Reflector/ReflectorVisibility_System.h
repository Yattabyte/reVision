#pragma once
#ifndef REFLECTORVISIBILITY_SYSTEM_H
#define REFLECTORVISIBILITY_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Logical/components.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Reflector/ReflectorData.h"


/***/
class ReflectorVisibility_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~ReflectorVisibility_System() = default;
	/***/
	inline ReflectorVisibility_System(const std::shared_ptr<ReflectorData> & visibility, const std::shared_ptr<std::vector<std::shared_ptr<CameraBuffer>>> & cameras)
		: m_frameData(visibility), m_cameras(cameras) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(Reflector_Component::ID, FLAG_REQUIRED);
		addComponentType(Transform_Component::ID, FLAG_REQUIRED);
		addComponentType(CameraArray_Component::ID, FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		// Link together the dimensions of view info and viewport vectors
		m_frameData->viewInfo.resize(m_cameras->size());

		// Compile results PER viewport
		for (int x = 0; x < m_frameData->viewInfo.size(); ++x) {
			auto & viewInfo = m_frameData->viewInfo[x];

			std::vector<GLint> lightIndices;
			int index = 0;
			for each (const auto & componentParam in components) {
				Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
				Reflector_Component * reflectorComponent = (Reflector_Component*)componentParam[1];
				Transform_Component * transformComponent = (Transform_Component*)componentParam[2];
				CameraArray_Component * cameraComponent = (CameraArray_Component*)componentParam[3];

				// Synchronize the component if it is visible
				if (renderableComponent->m_visible[x])
					lightIndices.push_back((GLuint)index);
				index++;
			}

			// Update camera buffers
			viewInfo.visLights.beginWriting();
			viewInfo.visLightCount = (GLsizei)lightIndices.size();
			viewInfo.visLights.write(0, sizeof(GLuint) * lightIndices.size(), lightIndices.data());
			// count, primCount, first, reserved
			const GLuint data[] = { (GLuint)m_frameData->shapeVertexCount, (GLuint)viewInfo.visLightCount,0,0 };
			viewInfo.indirectShape.write(0, sizeof(GLuint) * 4, &data);		
		}
	}


private:
	// Private Attributes
	std::shared_ptr<ReflectorData> m_frameData;
	std::shared_ptr<std::vector<std::shared_ptr<CameraBuffer>>> m_cameras;
};

#endif // REFLECTORVISIBILITY_SYSTEM_H