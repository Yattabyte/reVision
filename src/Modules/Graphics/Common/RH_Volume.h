#ifndef RH_VOLUME_H
#define RH_VOLUME_H
#define RH_TEXTURE_COUNT 4

#include "Modules/Graphics/Common/CameraBuffer.h"
#include <memory>


class Engine;

/** Represents a data-structure containing radiance-hints volume data. */
class RH_Volume {
public:
	// Public (de)Constructors
	/** Destroy the radiance hint volume. */
	~RH_Volume();
	/** Construct a radiance hint volume. 
	@param	engine			the engine to use.
	@param	cameraBuffer	the camera to use the frustum of. */
	RH_Volume(Engine * engine, const std::shared_ptr<CameraBuffer> & cameraBuffer);


	// Public Methods
	/** Update the volume's attributes based on the input camera.*/
	void updateVolume();
	/***/
	void resize(const float & resolution = 16.0f);
	/***/
	void clear();
	/***/
	void writePrimary();
	/***/
	void readPrimary(const GLuint & binding = 0);
	/***/
	void writeSecondary();
	/***/
	void readSecondary(const GLuint & binding = 0);


	// Public Attributes
	glm::vec3 m_min = glm::vec3(0.0f), m_max = glm::vec3(0.0f), m_center = glm::vec3(0.0f);
	float m_resolution = 16.0F, m_unitSize = 0.0f;

	
private:
	// Private Attributes
	Engine * m_engine = nullptr;
	GLuint m_fboIDS[2] = { 0,0 }, m_textureIDS[2][RH_TEXTURE_COUNT] = { { 0,0,0,0 }, { 0,0,0,0 } };
	std::shared_ptr<CameraBuffer> m_cameraBuffer;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // RH_VOLUME_H