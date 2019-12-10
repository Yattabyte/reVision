#pragma once
#ifndef UI_SCROLLBAR_V_H
#define UI_SCROLLBAR_V_H

#include "Modules/UI/Basic Elements/Button.h"
#include "Modules/UI/Decorators/UI_Decorator.h"
#include "Utilities/GL/IndirectDraw.h"


/** Scrollbar decorator object. */
class Scrollbar_V final : public UI_Decorator {
public:
	// Public Interaction Enums
	enum class Interact : int {
		on_scroll_change = (int)UI_Element::Interact::last_interact_index
	};


	// Public (De)Constructors
	/** Destroy this scrollbar. */
	~Scrollbar_V() noexcept;
	/** Construct a vertical scrollbar, decorating the supplied component.
	@param	engine		reference to the engine to use. 
	@param	component	the component to decorate. */
	Scrollbar_V(Engine& engine, const std::shared_ptr<UI_Element>& component) noexcept;


	// Public Interface Implementation
	void mouseAction(const MouseEvent& mouseEvent) noexcept final;
	void renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) noexcept final;


	// Public Methods
	/** Set the linear amount for the location of the scroll bar.
	@param	linear		the linear amount to put the scroll bar. */
	void setLinear(const float& linear) noexcept;
	/** Retrieve the linear value for this scrollbar.
	@return				the linear value for this scroll bar. */
	float getLinear() const noexcept;


protected:
	// Protected Methods
	/** Update the data dependant on the scale of this element. */
	void updateGeometry() noexcept;
	/** Update the position of all scrollbar elements. */
	void updateElementPosition() noexcept;


	// Protected Attributes
	float m_linear = 1.0f;
	GLuint m_vaoID = 0, m_vboID = 0;
	Shared_Shader m_shader;
	IndirectDraw<1> m_indirect;
};

#endif // UI_SCROLLBAR_V_H