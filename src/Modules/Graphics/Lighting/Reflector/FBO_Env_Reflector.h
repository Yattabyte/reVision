#pragma once
#ifndef FBO_ENVMAP_H
#define FBO_ENVMAP_H

#include "glm/glm.hpp"
#include <glad/glad.h>


/** Framebuffer containing texture data for all environment maps. */
class FBO_Env_Reflector {
public:
	// Public (De)Constructors
	/** Destroy this framebuffer. */
	~FBO_Env_Reflector();
	/** Construct a framebuffer. */
	FBO_Env_Reflector() noexcept;


	// Public Methods
	/** Set the size of this framebuffer.
	@param	newSize		the new size to use.
	@param	depth		the new depth to use. */
	void resize(const glm::ivec2 newSize, const GLuint& depth);
	/** Clear the data out of the entire framebuffer. */
	void clear() noexcept;
	/** Clear the data out of a specific layer in the framebuffer.
	@param	zOffset		the layer to clear out of. */
	void clear(const GLint& zOffset) noexcept;
	/** Bind this framebuffer for writing, at the specific mipmap index.
	@param	index		the FBO index to bind. */
	void bindForWriting(const int& index) noexcept;
	/** Bind this framebuffer for reading, with the binding offset specified.
	@param	binding		the texture binding point. */
	void bindForReading(const GLuint& binding = 0) noexcept;


private:
	// Private but deleted
	/** Disallow move constructor. */
	inline FBO_Env_Reflector(FBO_Env_Reflector&&) noexcept = delete;
	/** Disallow copy constructor. */
	inline FBO_Env_Reflector(const FBO_Env_Reflector&) noexcept = delete;
	/** Disallow move assignment. */
	inline FBO_Env_Reflector& operator =(FBO_Env_Reflector&&) noexcept = delete;
	/** Disallow copy assignment. */
	inline FBO_Env_Reflector& operator =(const FBO_Env_Reflector&) noexcept = delete;


	// Private Attributes
	GLuint m_fboID[6] = { 0,0,0,0,0,0 }, m_textureID = 0;
	glm::ivec2 m_size = glm::ivec2(1);
	GLuint m_depth = 1;
};

#endif // FBO_ENVMAP_H