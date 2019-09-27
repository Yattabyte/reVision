#pragma once
#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "Modules/Graphics/Common/Graphics_Framebuffers.h"
#include "glm/glm.hpp"
#include <memory>


/** Representation of the data backing a unique viewing perspective. Not a camera, but the buffers one would render into and with. */
struct Viewport {
public:
	// Public (de)Constructors
	/** Destroy this viewport. */
	inline ~Viewport() = default;
	/** Construct a viewport, given an offset and size.
	@param	screenPosition	the offset relative to the bottom left of the screen.
	@param	dimensions		the size of the viewport. */
	Viewport(const glm::ivec2& screenPosition, const glm::ivec2& dimensions);


	// Public Methods
	/** Change the size of this viewport.
	@param	size			the new size to use.
	@param	layerFaces		the number of layer faces to use. */
	void resize(const glm::ivec2& size, const int& layerFaces);
	/** Bind this viewport. */
	void bind();
	/** Clear the data held by this viewport, such as it's framebuffers. */
	void clear();


	// Public Attributes
	glm::ivec2 m_screenPosition = glm::vec2(0);
	glm::ivec2 m_dimensions = glm::vec2(1);
	int m_layerFaces = 1;
	std::shared_ptr<Graphics_Framebuffers> m_gfxFBOS;
};

#endif // VIEWPORT_H