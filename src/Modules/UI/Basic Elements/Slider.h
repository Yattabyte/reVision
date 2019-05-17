#pragma once
#ifndef UI_SLIDER_H
#define UI_SLIDER_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Label.h"
#include "Modules/UI/Decorators/Border.h"


/** Slider ui element. */
class Slider : public UI_Element
{
public:
	// Interaction enums
	enum interact {
		on_slider_change = UI_Element::last_interact_index
	};


	// Public (de)Constructors
	inline ~Slider() = default;
	inline Slider(Engine * engine, const float & startingValue = 0.0f) {
		// Make a background panel for cosemetic purposes
		auto panel = std::make_shared<Panel>(engine);
		panel->setColor(glm::vec4(0.3f));
		m_backPanel = std::make_shared<Border>(engine, panel);
		addElement(m_backPanel);

		// Create the sliding paddle
		auto paddle = std::make_shared<Panel>(engine);
		paddle->setColor(glm::vec4(0.75f));
		m_paddle = paddle;
		panel->addElement(m_paddle);

		// Add a label indicating the toggle state
		m_label = std::make_shared<Label>(engine, std::to_string(startingValue));
		m_label->setAlignment(Label::align_right);
		m_label->setTextScale(12.0f);
		m_label->setMaxScale(glm::vec2(30.0f, m_label->getMaxScale().y));
		m_label->setColor(glm::vec3(0.75f));
		addElement(m_label);

		// Callbacks
		addCallback(on_mouse_enter, [&]() { m_highlighted = true; });
		addCallback(on_mouse_exit, [&]() { m_highlighted = false; });
		addCallback(on_mouse_press, [&]() { m_pressed = true; });
		addCallback(on_mouse_release, [&]() { m_pressed = false; });

		m_percentage = startingValue;
		update();
	}


	// Interface Implementation	
	inline virtual void update() override {
		if (!m_paddle) return;
		m_paddle->setPosition({(2.0f * m_percentage - 1.0f) * (200.0f - m_paddle->getScale().x), 0 });
	}
	inline virtual void setScale(const glm::vec2 & scale) override {
		m_paddle->setMaxScale({ 25, 14 });
		m_paddle->setScale({ 25, 14 });
		m_backPanel->setMaxScale({ 200, 14 });
		m_backPanel->setScale({ 200, 14 });
		m_backPanel->setPosition({ 48, 0 });
		m_label->setMaxScale({ 50, 28 });
		m_label->setScale({ 50, 28 });
		m_label->setPosition({ -225, 0 });
		setMaxScale({ 250, 28 });
		UI_Element::setScale({ 250, 28 });
	}
	inline virtual void mouseAction(const MouseEvent & mouseEvent) override {
		m_highlighted = false;
		m_pressed = false;
		if (getVisible() && getEnabled()) {
			if (mouseWithin(mouseEvent)) {
				m_highlighted = true;
				m_pressed = (bool)mouseEvent.m_action;
				if (mouseEvent.m_action == MouseEvent::PRESS) {
					const float mx = float(mouseEvent.m_xPos) - m_position.x - m_backPanel->getPosition().x + m_backPanel->getScale().x;
					setPercentage(mx / (m_backPanel->getScale().x * 2.0f));
					enactCallback(on_slider_change);
				}
			}
			UI_Element::mouseAction(mouseEvent);
		}
	}


	// Public Methods
	/** Set the percentage for this slider.
	@param	percentage	the percentage amount to put this slider at. */
	inline void setPercentage(const float & linear) {
		m_percentage = std::clamp<float>(linear, 0.0f, 1.0f);
		setText(std::to_string((int)std::round<int>(m_percentage * 100.0f)) + "%");
		update();
	}
	/** Get the percentage value for this scrollbar.
	@return				the percentage value for this slider. */
	inline float getPercentage() const {
		return m_percentage;
	}
	/** Set this slider's text.
	@param	text	the text to use. */
	inline void setText(const std::string & text) {
		m_label->setText(text);
	}
	/** Retrieve this slider's text.
	@return	the text this label uses. */
	inline std::string getText() const {
		return m_label->getText();
	}


private:
	// Private Attributes
	float m_percentage = 0.0f;
	bool
		m_highlighted = false,
		m_pressed = false;
	std::shared_ptr<Label> m_label;
	std::shared_ptr<UI_Element> m_backPanel, m_paddle;
};

#endif // UI_SLIDER_H