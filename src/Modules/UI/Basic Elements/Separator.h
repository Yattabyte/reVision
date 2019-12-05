#pragma once
#ifndef UI_SEPARATOR_H
#define UI_SEPARATOR_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Assets/Shader.h"
#include "Utilities/GL/IndirectDraw.h"


/** UI separator class. Renders a faded out line across its width. */
class Separator : public UI_Element {
public:
	// Public (De)Constructors
	/** Destroy this separator. */
	~Separator() noexcept;
	/** Construct a separator.
	@param	engine		reference to the engine to use. */
	explicit Separator(Engine& engine) noexcept;


	// Public Interface Implementation
	virtual void renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) noexcept override;


	// Public Methods
	/** Set this element's color.
	@param	text	the new color to render with. */
	void setColor(const glm::vec4& color) noexcept;
	/** Retrieve this element's color.
	@return			the color used by this element. */
	glm::vec4 getColor() const noexcept;


protected:
	// Protected Attributes
	glm::vec4 m_color = glm::vec4(1.0f);
	GLuint m_vaoID = 0, m_vboID = 0;
	Shared_Shader m_shader;
	IndirectDraw<1> m_indirect;
};

#endif // UI_SEPARATOR_H