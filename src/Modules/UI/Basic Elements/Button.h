#pragma once
#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Label.h"


/** UI button class, affords being pushed and released. */
class Button : public UI_Element {
public:
	// Public (De)Constructors
	/** Destroy the button. */
	inline ~Button() = default;
	/** Creates a button with specific text inside.
	@param	engine		the engine.
	@param	text		the button text. */
	inline explicit Button(Engine* engine, const std::string& text = "Button")
		: UI_Element(engine), m_label(std::make_shared<Label>(engine, text)) {
		// All buttons have labels
		m_label->setAlignment(Label::align_center);
		m_label->setTextScale(12.5f);
		addElement(m_label);

		// Callbacks
		addCallback(on_resize, [&]() { m_label->setScale(getScale()); });
	}


	// Public Interface Implementation
	inline virtual void userAction(ActionState& actionState) override {
		// Only thing a user can do is press the button
		if (actionState.isAction(ActionState::UI_ENTER) == ActionState::PRESS)
			pressButton();
	}
	inline virtual void renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) override {
		// Update Colors
		glm::vec4 color(0.75);
		if (m_pressed)
			color *= 0.5f;
		if (m_hovered)
			color *= 1.5f;
		m_label->setColor(color);

		// Render Children
		UI_Element::renderElement(deltaTime, position, scale);
	}


	// Public Methods
	/** Fully press and release this button, enacting its on_clicked callback. */
	inline void pressButton() {
		enactCallback(UI_Element::on_clicked);
	}
	/** Set this label element's text.
	@param	text	the text to use. */
	inline void setText(const std::string& text) {
		m_label->setText(text);
	}
	/** Retrieve this buttons' labels text.
	@return			the text this label uses. */
	inline std::string getText() const {
		return m_label->getText();
	}
	/** Set this label element's text scaling factor.
	@param	text	the new scaling factor to use. */
	inline void setTextScale(const float& textScale) {
		m_label->setTextScale(textScale);
	}

	/** Retrieve this label's text scaling factor.
	@return	the text scaling factor. */
	inline float getTextScale() const {
		return m_label->getTextScale();
	}
	/** Set the bevel radius for this button.
	@param radius	the new radius to use. */
	inline void setBevelRadius(const float& radius) {
		m_bevelRadius = radius;
	}
	/** Get the bevel radius from this button.
	@return			this buttons' bevel radius. */
	inline float getBevelRadius() const {
		return m_bevelRadius;
	}


protected:
	// Protected Attributes
	float m_bevelRadius = 10.0f;
	std::shared_ptr<Label> m_label;
};

#endif // UI_BUTTON_H