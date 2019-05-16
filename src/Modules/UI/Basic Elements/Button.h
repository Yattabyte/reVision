#pragma once
#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Label.h"
#include "Assets/Shader.h"
#include "Utilities/GL/StaticBuffer.h"
#include <memory>


/** UI button class, affords being pushed and released. */
class Button : public UI_Element
{
public:
	// (de)Constructors
	inline ~Button() = default;
	inline Button(Engine * engine, const std::string & text = "Button") {

		// All buttons have labels, but it isn't interactive
		m_label = std::make_shared<Label>(engine, text);
		m_label->setAlignment(Label::align_center);
		m_label->setTextScale(12.5f);
		addElement(m_label);

		// Callbacks
		addCallback(on_resize, [&]() {m_label->setScale(getScale()); });
		addCallback(on_mouse_press, [&]() {m_pressed = true; });
		addCallback(on_mouse_release, [&]() {m_pressed = false; });
		addCallback(on_mouse_enter, [&]() {m_highlighted = true; });
		addCallback(on_mouse_exit, [&]() {m_highlighted = false; });
	}


	// Interface Implementation
	inline virtual void renderElement(const float & deltaTime, const glm::vec2 & position, const glm::vec2 & scale) override {
		glm::vec3 textColor(0.75);
		if (m_pressed)
			textColor *= 0.5f;
		if (m_highlighted)
			textColor *= 1.5f;
		m_label->setColor(textColor);
		UI_Element::renderElement(deltaTime, position, scale);
	}


	// Public Methods
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
	/** Get if this button is pressed. */
	inline bool getPressed() const {
		return m_pressed;
	}


protected:
	// Protected Attributes
	bool 
		m_highlighted = false, 
		m_pressed = false;
	float m_bevelRadius = 10.0f;


private:
	// Private Attributes
	std::shared_ptr<Label> m_label;
};

#endif // UI_BUTTON_H