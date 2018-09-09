#ifndef RH_VOLUME_H
#define RH_VOLUME_H

#include "ECS\Components\Camera_C.h"

class Engine;
class RH_Volume {
public:
	// Public Attributes
	glm::vec3 m_min = glm::vec3(0.0f), m_max = glm::vec3(0.0f), m_center = glm::vec3(0.0f);
	float m_nearPlane = -CAMERA_NEAR_PLANE, m_farPlane = -100.0f, m_resolution = 16.0F, m_unitSize = 0.0f;


	// Public (de)Constructors
	~RH_Volume();
	RH_Volume(Engine * engine);


	// Public Methods
	void updateVolume(const VB_Element<Camera_Buffer> & cameraBuffer);

	
private:
	// Private Attributes
	Engine * m_engine = nullptr;
};

#endif // RH_VOLUME_H