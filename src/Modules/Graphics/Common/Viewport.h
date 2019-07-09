#pragma once
#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "Modules/Graphics/Common/CameraBuffer.h"
#include "Modules/Graphics/Common/Graphics_Framebuffers.h"
#include "glm/glm.hpp"
#include <memory>


/***/
struct Viewport {
public:
	// Public (de)Constructors
	/***/
	inline ~Viewport() = default;
	/***/
	Viewport(const glm::ivec2 & screenPosition, const glm::ivec2 & dimensions);
	

	// Public Methods
	/***/
	void resize(const glm::ivec2 & size, const int & layerFaces);
	/***/
	void bind(const CameraBuffer::CamStruct & camera);
	/***/
	void clear();


	// Public Attributes
	glm::ivec2 m_screenPosition = glm::vec2(0);
	glm::ivec2 m_dimensions = glm::vec2(1);
	int m_layerFaces = 1;
	std::shared_ptr<Graphics_Framebuffers> m_gfxFBOS;
};

#endif // VIEWPORT_H