#pragma once
#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "Modules/Graphics/Common/Graphics_Framebuffers.h"
#include "glm/glm.hpp"
#include <memory>


class Engine;

/** Representation of the data backing a unique viewing perspective. Not a camera, but the buffers one would render into and with. */
struct Viewport {
public:
	// Public (De)Constructors
	/** Destroy this viewport. */
	inline ~Viewport() = default;
	/** Construct a viewport, given an offset and size.
	@param	screenPosition	the offset relative to the bottom left of the screen.
	@param	dimensions		the size of the viewport.
	@param	engine			reference to the engine to use. */
	Viewport(const glm::ivec2& screenPosition, const glm::ivec2& dimensions, Engine& engine) noexcept;


	// Public Methods
	/** Change the size of this viewport.
	@param	size			the new size to use.
	@param	layerFaces		the number of layer faces to use. */
	void resize(const glm::ivec2& size, const int& layerFaces) noexcept;
	/** Bind this viewport. */
	void bind() noexcept;
	/** Clear the data held by this viewport, such as it's frame-buffers. */
	void clear() noexcept;


	// Public Attributes
	glm::ivec2 m_screenPosition = glm::vec2(0);
	glm::ivec2 m_dimensions = glm::vec2(1);
	int m_layerFaces = 1;
	Graphics_Framebuffers m_gfxFBOS;
};

#endif // VIEWPORT_H
