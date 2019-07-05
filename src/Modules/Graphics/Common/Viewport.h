#pragma once
#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "Modules/Graphics/Common/CameraBuffer.h"
#include "Modules/Graphics/Common/Graphics_Framebuffers.h"
#include "Modules/Graphics/Common/RH_Volume.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/MappedChar.h"
#include "glm/glm.hpp"
#include <memory>


class Engine;

/***/
struct Viewport {
public:
	// Public (de)Constructors
	/***/
	~Viewport();
	/***/
	Viewport(Engine * engine, const glm::ivec2 & screenPosition, const glm::ivec2 & dimensions);
	

	// Public Methods
	/***/
	void resize(const glm::ivec2 & size);
	/***/
	void bind(const CameraBuffer::CamStruct & camera);
	/***/
	void clear();


	// Public Attributes
	glm::ivec2 m_screenPosition = glm::vec2(0);
	glm::ivec2 m_dimensions = glm::vec2(1);
	std::shared_ptr<Graphics_Framebuffers> m_gfxFBOS;
	std::shared_ptr<RH_Volume> m_rhVolume;
};

#endif // VIEWPORT_H