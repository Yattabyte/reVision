#pragma once
#ifndef UI_PANEL_H
#define UI_PANEL_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Assets/Shader.h"
#include "Utilities/GL/IndirectDraw.h"


/** UI panel class, affords containing other elements, and rendering a fixed color. */
class Panel final : public UI_Element {
public:
	// Public (De)Constructors
	/** Destroy this panel. */
	~Panel() noexcept;
	/** Construct a panel.
	@param	engine		reference to the engine to use. */
	explicit Panel(Engine& engine);


	// Public Interface Implementation
	void renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) final;


	// Public Methods
	/** Set this panel's color.
	@param	color	the new color to render with. */
	void setColor(const glm::vec4& color) noexcept;
	/** Retrieve this panel's color.
	@return			the color used by this element. */
	glm::vec4 getColor() const noexcept;


protected:
	// Protected Methods
	/** Update the data dependant on the scale of this element. */
	void updateGeometry();


	// Protected Attributes
	glm::vec4 m_color = glm::vec4(0.2f);
	GLuint m_vaoID = 0, m_vboID = 0;
	Shared_Shader m_shader;
	IndirectDraw<1> m_indirect;
};

#endif // UI_PANEL_H