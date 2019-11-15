#pragma once
#ifndef GRAPHICS_FRAMEBUFFERS_H
#define GRAPHICS_FRAMEBUFFERS_H

#include "Modules/Graphics/Common/RH_Volume.h"
#include "Utilities/MappedChar.h"
#include <glad/glad.h>
#include "glm/glm.hpp"
#include <functional>
#include <memory>
#include <tuple>
#include <vector>


class Engine;

/** Creates, stores, and manages the size of all the frame-buffers needed for our PBR rendering pipeline.
Decouples creation and management from the various sources who need a framebuffer, which isn't desirable, however easily allows them to be shared.
Necessary such that 1 rendering pipeline can be reused for different sized view-ports/viewing perspectives. */
class Graphics_Framebuffers {
public:
	// Public (De)Constructors
	/** Destroy this collection of frame-buffers. */
	~Graphics_Framebuffers() noexcept;
	/** Construct a collection of PBR frame-buffers at a specific size.
	@param	size			the size to use.
	@param	engine			the active engine. */
	Graphics_Framebuffers(const glm::ivec2& size, Engine* engine) noexcept;


	// Public Methods
	/** Create a framebuffer with a specific name, specific formats, and optionally mipmap it to 6 LOD intervals.
	@param	name			the name of the framebuffer, for lookup later.
	@param	textureFormats	vector of texture parameters to connect to the FBO.
	@param	mipmapped		(optional) whether to mipmap the FBO textures, down to 6 LOD intervals. */
	void createFBO(const char* name, const std::vector<std::tuple<GLenum, GLenum, GLenum>>& textureFormats, const bool& mipmapped = false) noexcept;
	/** Bind a framebuffer for writing.
	@param	name			the name of the framebuffer to bind. */
	void bindForWriting(const char* name) noexcept;
	/** Bind a framebuffer for reading.
	@param	name			the name of the framebuffer to bind.
	@param	binding			the reading index which the framebuffer should bind to. */
	void bindForReading(const char* name, const GLuint& binding = 0) noexcept;
	/** Clear all framebuffer color attachments of their data. */
	void clear() noexcept;
	/** Resize all framebuffer textures.
	@param	newSize			the new size all framebuffer textures should expand/shrink to.
	@param	layerFaces		the number of layers to use. */
	void resize(const glm::ivec2& newSize, const int& layerFaces) noexcept;
	/** Retrieve the FBO ID for a given framebuffer.
	@param	name			the name of the framebuffer to retrieve from. */
	GLuint getFboID(const char* name) noexcept;
	/** Retrieve the Texture ID for a given framebuffer at a given texture index.
	@param	name			the name of the framebuffer to retrieve from.
	@param	index			the index into this framebuffer to retrieve the texture ID from. (spot 0, spot 1, etc...) */
	GLuint getTexID(const char* name, const size_t& index) noexcept;


	// Public Attributes
	MappedChar<std::tuple<
		GLuint,			// FBO ID
		bool,			// MIPMAPPED
		std::vector<std::tuple<
		GLuint,			// Texture ID
		GLenum,			// Internal Format
		GLenum,			// Format
		GLenum,			// Type
		GLenum			// Attachment
		>>>>				m_fbos;
	glm::ivec2				m_renderSize = glm::ivec2(1);
	int						m_layerFaces = 1;
	RH_Volume				m_rhVolume;
	std::shared_ptr<bool>	m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // GRAPHICS_FRAMEBUFFERS_H