#ifndef RH_VOLUME_H
#define RH_VOLUME_H
#define RH_TEXTURE_COUNT 4

#include "glm/glm.hpp"
#include "glad/glad.h"
#include <memory>


// Forward Declarations
class Engine;
class Camera;

/** Represents a data-structure containing radiance-hints volume data. */
class RH_Volume {
public:
	// Public (De)Constructors
	/** Destroy the radiance hint volume. */
	~RH_Volume() noexcept;
	/** Construct a radiance hint volume.
	@param	engine			reference to the engine to use. */
	explicit RH_Volume(Engine& engine) noexcept;


	// Public Methods
	/** Update the volume's attributes based on the input camera.
	@param	camera			the camera to use the frustum of. */
	void updateVolume(const Camera& camera) noexcept;
	/** Resize this volume's 3D texture.
	@param	resolution		the new size to use. */
	void resize(const float& resolution = 16.0f) noexcept;
	/** Clear the data out of this framebuffer's textures. */
	void clear() noexcept;
	/** Write to the first bounce framebuffer. */
	void writePrimary() noexcept;
	/** Read from the first bounce framebuffer.
	@param	binding			the reading index to bind to. */
	void readPrimary(const GLuint& binding = 0) noexcept;
	/** Write to the second bounce framebuffer. */
	void writeSecondary() noexcept;
	/** Read from the second bounce framebuffer.
	@param	binding			the reading index to bind to. */
	void readSecondary(const GLuint& binding = 0) noexcept;


	// Public Attributes
	glm::vec3 m_min = glm::vec3(0.0f), m_max = glm::vec3(0.0f), m_center = glm::vec3(0.0f);
	float m_resolution = 16.0F, m_unitSize = 0.0f;


private:
	// Private Attributes
	Engine& m_engine;
	GLuint m_fboIDS[2]{0}, m_textureIDS[2][RH_TEXTURE_COUNT] = { {0}, {0} };
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // RH_VOLUME_H