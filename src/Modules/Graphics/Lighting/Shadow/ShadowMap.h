#pragma once
#ifndef SHADOW_MAP_H
#define SHADOW_MAP_H

#include <glad/glad.h>
#include "glm/glm.hpp"


/** Framebuffer containing shadow data for all light maps. */
class ShadowMap {
public:
	// Public (De)Constructors
	/** Destroy this framebuffer. */
	~ShadowMap();
	/** Construct this framebuffer. */
	ShadowMap() noexcept;


	// Public Methods
	/** Set the size of this framebuffer.
	@param	newSize		the new size to use.
	@param	depth		the new depth to use. */
	void resize(const glm::ivec2& newSize, const int& depth) noexcept;
	/** Clear the data out of a specific layer in the framebuffer.
	@param	zOffset		the layer to clear out of.
	@param	amount		the number of layers to clear. */
	void clear(const GLint& zOffset, const GLsizei& amount) noexcept;
	/** Bind this framebuffer for writing. */
	void bindForWriting() noexcept;


	// Public Attributes
	const GLuint& m_texNormal = m_textureIDS[0];
	const GLuint& m_texColor = m_textureIDS[1];
	const GLuint& m_texDepth = m_textureIDS[2];


private:
	// Private Attributes
	GLuint m_fboID = 0, m_textureIDS[3] = { 0,0,0 };
	glm::ivec2 m_size = glm::ivec2(1);
	int m_layerFaces = 1;
};

#endif // SHADOW_MAP_H