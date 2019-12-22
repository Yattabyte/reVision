#pragma once
#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Label.h"


/** UI button class, affords being pushed and released. */
class Button final : public UI_Element {
public:
	// Public (De)Constructors
	/** Destroy this button. */
	inline ~Button() = default;
	/** Creates a button with specific text inside.
	@param	engine		reference to the engine to use. 
	@param	text		the button text. */
	explicit Button(Engine& engine, const std::string& text = "Button");


	// Public Interface Implementation
	void userAction(ActionState& actionState) final;
	void renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) final;


	// Public Methods
	/** Fully press and release this button, enacting its on_clicked callback. */
	void pressButton();
	/** Set this label element's text.
	@param	text	the text to use. */
	void setText(const std::string& text);
	/** Retrieve this buttons' labels text.
	@return			the text this label uses. */
	std::string getText() const;
	/** Set this label element's text scaling factor.
	@param	text	the new scaling factor to use. */
	void setTextScale(const float& textScale) noexcept;

	/** Retrieve this label's text scaling factor.
	@return			the text scaling factor. */
	float getTextScale() const noexcept;
	/** Set the bevel radius for this button.
	@param radius	the new radius to use. */
	void setBevelRadius(const float& radius) noexcept;
	/** Retrieve the bevel radius from this button.
	@return			this buttons' bevel radius. */
	float getBevelRadius() const noexcept;


protected:
	// Protected Attributes
	float m_bevelRadius = 10.0f;
	std::shared_ptr<Label> m_label;
};

#endif // UI_BUTTON_H