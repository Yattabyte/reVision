#pragma once
#ifndef UI_BORDER_H
#define UI_BORDER_H

#include "Modules/UI/Decorators/UI_Decorator.h"
#include "Utilities/GL/IndirectDraw.h"
#include "Assets/Shader.h"


/** Border decorator object. */
class Border final : public UI_Decorator {
public:
	// Public (De)Constructors
	/** Destroy the border. */
	~Border() noexcept;
	/** Construct a border, decorating the supplied component.
	@param	engine		reference to the engine to use. 
	@param	component	the component to decorate. */
	Border(Engine& engine, const std::shared_ptr<UI_Element>& component);


	// Public Interface Implementations
	void renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) noexcept final;


	// Public Methods
	/** Set the border size for this decorator.
	@param		size		the new border size to use. */
	void setBorderSize(const float& size) noexcept;
	/** Retrieve the border size of this decorator.
	@return					the size of the border this decorator uses. */
	float getBorderSize() const noexcept;
	/** Set the border color.
	@param		color		the new border color to use. */
	void setBorderColor(const glm::vec3& color) noexcept;
	/** Retrieve the border color of this decorator.
	@return					the color of the border this decorator uses. */
	glm::vec3 getBorderColor() const noexcept;


protected:
	// Protected Methods
	void updateGeometry();


	// Protected Attributes
	float m_borderSize = 2.0f;
	glm::vec3 m_borderColor = glm::vec3(1.0f);
	GLuint m_vaoID = 0, m_vboID = 0;
	Shared_Shader m_shader;
	IndirectDraw<1> m_indirect;
};

#endif // UI_BORDER_H