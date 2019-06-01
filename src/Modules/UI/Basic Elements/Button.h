#pragma once
#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Label.h"


/** UI button class, affords being pushed and released. */
class Button : public UI_Element {
public:
	// Public (de)Constructors
	/** Destroy the button. */
	inline ~Button() = default;
	/** Creates a button with specific text inside. 
	@param	engine		the engine.
	@param	text		the button text. */
	inline Button(Engine * engine, const std::string & text = "Button") : UI_Element(engine) {
		// All buttons have labels
		m_label = std::make_shared<Label>(engine, text);
		m_label->setAlignment(Label::align_center);
		m_label->setTextScale(12.5f);
		addElement(m_label);

		// Callbacks
		addCallback(on_hover_start, [&]() { updateColors(); });
		addCallback(on_hover_stop, [&]() { updateColors(); });
		addCallback(on_press, [&]() { updateColors(); });
		addCallback(on_release, [&]() {	updateColors(); });
		addCallback(on_clicked, [&]() {	updateColors(); });
		addCallback(on_resize, [&]() { m_label->setScale(getScale()); });
		updateColors();
	}


	// Public Interface Implementation
	inline virtual void userAction(ActionState & actionState) override {
		// Only thing a user can do is press the button
		if (actionState.isAction(ActionState::UI_ENTER) == ActionState::PRESS)
			pressButton();
	}


	// Public Methods
	/** Fully press and release this button, enacting its on_clicked callback. */
	inline void pressButton() {
		enactCallback(UI_Element::on_clicked);
	}
	/** Set this label element's text.
	@param	text	the text to use. */
	inline void setText(const std::string & text) {
		m_label->setText(text);
		update();
	}
	/** Retrieve this buttons' labels text.
	@return	the text this label uses. */
	inline std::string getText() const {
		return m_label->getText();
	}
	/** Set the bevel radius for this button.
	@param radius	the new radius to use. */
	inline void setBevelRadius(const float & radius) {
		m_bevelRadius = radius;
	}
	/** Get the bevel radius from this button.
	@return radius	this buttons' bevel radius. */
	inline float getBevelRadius() const {
		return m_bevelRadius;
	}


protected:
	// Protected Methods
	/** Modifies the color button text, depending on the state of the button. */
	inline void updateColors() {
		glm::vec3 textColor(0.75);
		if (m_pressed)
			textColor *= 0.5f;
		if (m_hovered)
			textColor *= 1.5f;
		m_label->setColor(textColor);
	}


	// Protected Attributes
	float m_bevelRadius = 10.0f;
	std::shared_ptr<Label> m_label;
};

#endif // UI_BUTTON_H