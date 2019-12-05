#pragma once
#ifndef UI_SLIDER_H
#define UI_SLIDER_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Label.h"
#include "Modules/UI/Basic Elements/Panel.h"
#include "Modules/UI/Decorators/Border.h"


/** A UI component with a bar and a sliding paddle control. */
class Slider : public UI_Element {
public:
	// Public Interaction Enums
	enum class Interact : int {
		on_value_change = (int)UI_Element::Interact::last_interact_index
	};


	// Public (De)Constructors
	/** Destroy the slider. */
	inline ~Slider() = default;
	/** Construct a slider with a given starting value.
	@param	engine		reference to the engine to use. 
	@param	value		the starting value to use.
	@param	range		the starting range to use. */
	explicit Slider(Engine& engine, const float& value = 0.0f, const glm::vec2& range = { 0.0f, 1.0f }) noexcept;


	// Public Interface Implementation
	virtual void renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) noexcept override;
	virtual void mouseAction(const MouseEvent& mouseEvent) noexcept override;
	virtual void userAction(ActionState& actionState) noexcept override;


	// Public Methods
	/** Set the percentage for this slider.
	@param	amount		the value to put this slider at. */
	void setValue(const float& amount) noexcept;
	/** Retrieve the percentage value for this scrollbar.
	@return				the percentage value for this slider. */
	float getValue() const noexcept;
	/** Set the lower and upper ranges for this slider.
	@param	lowerRange	the lowest number this slider can use.
	@param	upperRange	the highest number this slider can use. */
	void setRanges(const float& lowerRange, const float& upperRange) noexcept;
	/** Set this slider's text.
	@param	text		the text to use. */
	void setText(const std::string& text) noexcept;
	/** Retrieve this slider's text.
	@return				the text this label uses. */
	std::string getText() const noexcept;


protected:
	// Protected Methods
	/** Update the data dependant on the scale of this element. */
	void updateGeometry();
	/** Update the position of the paddle for this element. */
	void updatePaddle();


	// Protected Attributes
	float m_value = 0.0f, m_lowerRange = 0.0f, m_upperRange = 1.0f;
	std::shared_ptr<Label> m_label;
	std::shared_ptr<Border> m_backPanel;
	std::shared_ptr<Panel> m_paddle;
};

#endif // UI_SLIDER_H