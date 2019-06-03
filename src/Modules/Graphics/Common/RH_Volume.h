#ifndef RH_VOLUME_H
#define RH_VOLUME_H

#include "Modules/Graphics/Common/CameraBuffer.h"
#include <memory>


class Engine;

/** Represents a data-structure containing radiance-hints volume data. */
class RH_Volume {
public:
	// Public (de)Constructors
	/** Destroy the radiance hint volume. */
	~RH_Volume();
	/** Construct a radiance hint volume. */
	RH_Volume(Engine * engine);


	// Public Methods
	/** Update the volume's attributes based on the input camera.
	@param	cameraBuffer	the camera to use the frustum of. */
	void updateVolume(const CameraBuffer & cameraBuffer);


	// Public Attributes
	glm::vec3 m_min = glm::vec3(0.0f), m_max = glm::vec3(0.0f), m_center = glm::vec3(0.0f);
	float m_resolution = 16.0F, m_unitSize = 0.0f;

	
private:
	// Private Attributes
	Engine * m_engine = nullptr;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // RH_VOLUME_H