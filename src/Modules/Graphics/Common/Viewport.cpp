#include "Modules/Graphics/Common/Viewport.h"


Viewport::Viewport(const glm::ivec2& screenPosition, const glm::ivec2& dimensions, Engine& engine) noexcept :
	m_screenPosition(screenPosition),
	m_dimensions(dimensions),
	m_gfxFBOS(dimensions, engine)
{
	// Tie geometry depth texture to lighting & reflection FBO
	glNamedFramebufferTexture(m_gfxFBOS.getFboID("LIGHTING"), GL_DEPTH_STENCIL_ATTACHMENT, m_gfxFBOS.getTexID("GEOMETRY", 3), 0);
	glNamedFramebufferTexture(m_gfxFBOS.getFboID("REFLECTION"), GL_DEPTH_STENCIL_ATTACHMENT, m_gfxFBOS.getTexID("GEOMETRY", 3), 0);
}

void Viewport::resize(const glm::ivec2& size, const int& layerFaces) noexcept
{
	if (m_dimensions != size || m_layerFaces != layerFaces) {
		m_dimensions = size;
		m_layerFaces = layerFaces;
		m_gfxFBOS.resize(size, layerFaces);
	}
}

void Viewport::bind() noexcept
{
	glViewport(m_screenPosition.x, m_screenPosition.y, m_dimensions.x, m_dimensions.y);
}

void Viewport::clear() noexcept
{
	m_gfxFBOS.clear();
}