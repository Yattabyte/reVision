#pragma once
#ifndef GRAPHICS_FRAMEBUFFERS_H
#define GRAPHICS_FRAMEBUFFERS_H

#include "Utilities/MappedChar.h"
#include "Utilities/GL/glad/glad.h"
#include "glm/glm.hpp"
#include <functional>
#include <memory>
#include <tuple>
#include <vector>


class Engine;

/***/
class Graphics_Framebuffers {
public:
	// Public (de)Constructors
	/***/
	~Graphics_Framebuffers();
	/***/
	Graphics_Framebuffers(const glm::ivec2 & size);


	// Public Methods
	/***/
	void createFBO(const char * name, const std::vector<std::tuple<GLenum, GLenum, GLenum>> textureFormats);
	/***/
	void bindForWriting(const char * name);
	/***/
	void bindForReading(const char * name, const GLuint & binding = 0);
	/***/
	void clear();
	/***/
	void resize(const glm::ivec2 & newSize);
	/***/
	GLuint getFboID(const char * name);
	/***/
	GLuint getTexID(const char * name, const size_t & index);


	// Public Attributes
	MappedChar<std::pair<
		GLuint,				// FBO ID
		std::vector<std::tuple<
		GLuint,			// Texture ID
		GLenum,			// Internal Format
		GLenum,			// Format
		GLenum,			// Type
		GLenum			// Attachment
		>>
	>>						m_fbos;
	glm::ivec2				m_renderSize = glm::ivec2(1);
	std::shared_ptr<bool>	m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // GRAPHICS_FRAMEBUFFERS_H