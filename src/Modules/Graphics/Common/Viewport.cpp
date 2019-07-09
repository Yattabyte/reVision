#include "Modules/Graphics/Common/Viewport.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Engine.h"
#include <algorithm>


Viewport::Viewport(const glm::ivec2 & screenPosition, const glm::ivec2 & dimensions)
	: m_screenPosition(screenPosition), m_dimensions(dimensions)
{
	// Initialize FBO's
	m_gfxFBOS = std::make_shared<Graphics_Framebuffers>(dimensions);
	glNamedFramebufferTexture(m_gfxFBOS->getFboID("LIGHTING"), GL_DEPTH_STENCIL_ATTACHMENT, m_gfxFBOS->getTexID("GEOMETRY", 3), 0);
	glNamedFramebufferTexture(m_gfxFBOS->getFboID("REFLECTION"), GL_DEPTH_STENCIL_ATTACHMENT, m_gfxFBOS->getTexID("GEOMETRY", 3), 0);
}

void Viewport::resize(const glm::ivec2 & size, const int & layerFaces)
{
	if (m_dimensions != size || m_layerFaces != layerFaces) {
		m_dimensions = size;
		m_layerFaces = layerFaces;
		m_gfxFBOS->resize(size, layerFaces);
	}
}

void Viewport::bind(const CameraBuffer::CamStruct & camera)
{
	glViewport(m_screenPosition.x, m_screenPosition.y, m_dimensions.x, m_dimensions.y);
}

void Viewport::clear()
{
	m_gfxFBOS->clear();
}