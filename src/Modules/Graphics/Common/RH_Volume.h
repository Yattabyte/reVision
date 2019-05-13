#ifndef RH_VOLUME_H
#define RH_VOLUME_H

#include "Modules/Graphics/Components/Camera_C.h"
#include <memory>


class Engine;
class RH_Volume {
public:
	// Public (de)Constructors
	~RH_Volume();
	RH_Volume(Engine * engine);


	// Public Methods
	void updateVolume(const VB_Element<Camera_Buffer> & cameraBuffer);


	// Public Attributes
	glm::vec3 m_min = glm::vec3(0.0f), m_max = glm::vec3(0.0f), m_center = glm::vec3(0.0f);
	float m_nearPlane = -CAMERA_NEAR_PLANE, m_farPlane = 1000.0f, m_resolution = 16.0F, m_unitSize = 0.0f;

	
private:
	// Private Attributes
	Engine * m_engine = nullptr;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // RH_VOLUME_H