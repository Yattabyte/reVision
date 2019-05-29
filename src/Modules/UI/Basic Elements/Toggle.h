#pragma once
#ifndef UI_TOGGLE_H
#define UI_TOGGLE_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Label.h"
#include "Modules/UI/Basic Elements/Panel.h"
#include "Modules/UI/Decorators/Border.h"


/** UI toggle switch class, affords being switched left and right. */
class Toggle : public UI_Element {
public:
	// Public Interaction Enums
	const enum interact {
		on_toggle = UI_Element::last_interact_index
	};


	// Public (de)Constructors
	/** Destroy the toggle switch. */
	inline ~Toggle() = default;
	/** Construct a toggle switch with a given on/off state.
	@param	engine		the engine to use.
	@param	state		the on/off state to use. */
	inline Toggle(Engine * engine, const bool & state = true) : m_toggledOn(state) {
		// Make a background panel for cosemetic purposes
		auto panel = std::make_shared<Panel>(engine);
		panel->setColor(glm::vec4(0.3f));
		m_backPanel = std::make_shared<Border>(engine, panel);
		addElement(m_backPanel);

		// Create the sliding paddle
		auto paddle = std::make_shared<Panel>(engine);
		paddle->setColor(glm::vec4(0.75f));
		panel->addCallback(UI_Element::on_hover_start, [paddle]() {paddle->setColor(glm::vec4(0.75f * 1.5f)); });
		panel->addCallback(UI_Element::on_hover_stop, [paddle]() {paddle->setColor(glm::vec4(0.75f)); });
		panel->addCallback(UI_Element::on_press, [paddle]() {paddle->setColor(glm::vec4(0.75f * 0.5f)); });
		panel->addCallback(UI_Element::on_release, [paddle]() {paddle->setColor(glm::vec4(0.75f)); });
		m_paddle = paddle;
		panel->addElement(m_paddle);

		// Add a label indicating the toggle state
		m_label = std::make_shared<Label>(engine, state ? "ON" : "OFF");
		m_label->setAlignment(Label::align_right);
		m_label->setTextScale(12.0f);
		m_label->setMaxScale(glm::vec2(30.0f, m_label->getMaxScale().y));
		m_label->setColor(glm::vec3(0.75f));
		addElement(m_label);

		// Callbacks
		addCallback(on_hover_start, [&]() { m_hovered = true; });
		addCallback(on_hover_stop, [&]() { m_hovered = false; });
		addCallback(on_press, [&]() { m_pressed = true; });
		addCallback(on_release, [&]() { 
			m_pressed = false;
			setToggled(!m_toggledOn);
			setText(m_toggledOn ? "ON" : "OFF");
			enactCallback(on_toggle);
		});
		update();
	}


	// Public Interface Implementation	
	inline virtual void setScale(const glm::vec2 & scale) override {
		m_paddle->setMaxScale({ 50.0f, 14 });
		m_paddle->setScale({ 50.0f, 14 });
		m_backPanel->setMaxScale({ 100, 14 });
		m_backPanel->setScale({ 100, 14 });
		m_backPanel->setPosition({ 48, 0 });
		m_label->setMaxScale({ 50, 28 });
		m_label->setScale({ 50, 28 });
		m_label->setPosition({ -125, 0 });
		setMaxScale({ 150, 28 });
		UI_Element::setScale({ 150, 28 });
	}
	inline virtual void update() override {
		if (!m_paddle) return;
		if (m_toggledOn)
			m_paddle->setPosition({ 50, 0 });
		else
			m_paddle->setPosition({ -50, 0 });
	}
	inline virtual void renderElement(const float & deltaTime, const glm::vec2 & position, const glm::vec2 & scale) override {
		glm::vec4 color(0.75);
		if (m_pressed)
			color *= 0.5f;
		if (m_hovered)
			color *= 1.5f;
		m_paddle->setColor(color);
		UI_Element::renderElement(deltaTime, position, glm::min(m_scale, scale));
	}


	// Public Methods
	/** Set this slider's text.
	@param	text	the text to use. */
	inline void setText(const std::string & text) {
		m_label->setText(text);
		update();
	}
	/** Retrieve this slider's text.
	@return			the text this label uses. */
	inline std::string getText() const {
		return m_label->getText();
	}
	/** Set the toggle state of this button.
	@param	state	the new state to use. */
	inline void setToggled(const bool & state) {
		m_toggledOn = state;
		update();
	}
	/** Return the toggle state of this button. 
	@return			whether or not this toggle is on or off. */
	inline bool getToggled() const {
		return m_toggledOn;
	}


protected:
	// Protected Attributes
	bool
		m_hovered = false,
		m_pressed = false,
		m_toggledOn = true;
	std::shared_ptr<Label> m_label;
	std::shared_ptr<Border> m_backPanel;
	std::shared_ptr<Panel> m_paddle;
};

#endif // UI_TOGGLE_H