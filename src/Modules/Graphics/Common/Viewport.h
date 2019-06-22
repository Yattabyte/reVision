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
	Viewport(Engine * engine, const glm::ivec2 & screenPosition, const glm::ivec2 & dimensions, const CameraBuffer::BufferStructure & cameraData = CameraBuffer::BufferStructure());
	

	// Public Methods
	/***/
	void resize(const glm::ivec2 & size);
	/***/
	void setDrawDistance(const float & drawDistance);
	/***/
	float getDrawDistance() const;
	/***/
	void setFOV(const float & fov);
	/***/
	float getFOV() const;
	/***/
	void set3DPosition(const glm::vec3 & position);
	/***/
	glm::vec3 get3DPosition() const;
	/***/
	void setViewMatrix(const glm::mat4 & vMatrix);
	/***/
	glm::mat4 getViewMatrix() const;
	/***/
	void genPerspectiveMatrix();
	/***/
	glm::mat4 getPerspectiveMatrix() const;
	/***/
	void bind();
	/***/
	void clear();


	// Public Attributes
	glm::ivec2 m_screenPosition = glm::vec2(0);
	glm::ivec2 m_dimensions = glm::vec2(1);
	std::shared_ptr<CameraBuffer> m_cameraBuffer;
	std::shared_ptr<Graphics_Framebuffers> m_gfxFBOS;
	std::shared_ptr<RH_Volume> m_rhVolume;
};

#endif // VIEWPORT_H