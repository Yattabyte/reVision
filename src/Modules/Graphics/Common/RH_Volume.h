#ifndef RH_VOLUME_H
#define RH_VOLUME_H
#define RH_TEXTURE_COUNT 4

#include "Modules/Graphics/Common/Camera.h"
#include <memory>


class Engine;

/** Represents a data-structure containing radiance-hints volume data. */
class RH_Volume {
public:
	// Public (de)Constructors
	/** Destroy the radiance hint volume. */
	~RH_Volume();
	/** Construct a radiance hint volume.
	@param	engine			the engine to use. */
	explicit RH_Volume(Engine* engine);


	// Public Methods
	/** Update the volume's attributes based on the input camera.
	@param	cameraBuffer	the camera to use the frustum of. */
	void updateVolume(const Camera* camera);
	/** Resize this volume's 3D texture.
	@param	resolution		the new size to use. */
	void resize(const float& resolution = 16.0f);
	/** Clear the data out of this framebuffer's textures. */
	void clear();
	/** Write to the first bounce framebuffer. */
	void writePrimary();
	/** Read from the first bounce framebuffer.
	@param	binding			the reading index to bind to. */
	void readPrimary(const GLuint& binding = 0);
	/** Write to the second bounce framebuffer. */
	void writeSecondary();
	/** Read from the second bounce framebuffer.
	@param	binding			the reading index to bind to. */
	void readSecondary(const GLuint& binding = 0);


	// Public Attributes
	glm::vec3 m_min = glm::vec3(0.0f), m_max = glm::vec3(0.0f), m_center = glm::vec3(0.0f);
	float m_resolution = 16.0F, m_unitSize = 0.0f;


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	GLuint m_fboIDS[2] = { 0,0 }, m_textureIDS[2][RH_TEXTURE_COUNT] = { { 0,0,0,0 }, { 0,0,0,0 } };
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // RH_VOLUME_H