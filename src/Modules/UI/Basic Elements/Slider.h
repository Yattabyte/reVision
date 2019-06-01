#pragma once
#ifndef UI_SLIDER_H
#define UI_SLIDER_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Label.h"
#include "Modules/UI/Decorators/Border.h"


/** A UI component with a bar and a sliding paddle control. */
class Slider : public UI_Element {
public:
	// Public Interaction Enums
	const enum interact {
		on_value_change = UI_Element::last_interact_index
	};


	// Public (de)Constructors
	/** Destroy the slider. */
	inline ~Slider() = default;
	/** Construct a slider with a given starting value. 
	@param	engine		the engine to use.
	@param	value		the starting value to use. 
	@param	range		the starting range to use. */
	inline Slider(Engine * engine, const float & value = 0.0f, const glm::vec2 & range = {0.0f, 1.0f}, UI_Element * parent = nullptr)
		: UI_Element(engine, parent), m_value(value), m_lowerRange(range.x), m_upperRange(range.y) {
		// Make a background panel for cosemetic purposes
		auto panel = std::make_shared<Panel>(engine, this);
		panel->setColor(glm::vec4(0.3f));
		m_backPanel = std::make_shared<Border>(engine, panel, this);
		m_backPanel->setMaxScale({ 200, 14 });
		m_backPanel->setScale({ 200, 14 });
		m_backPanel->setPosition({ 48, 0 });
		addElement(m_backPanel);

		// Create the sliding paddle
		auto paddle = std::make_shared<Panel>(engine, this);
		paddle->setColor(glm::vec4(0.75f));
		paddle->setMaxScale({ 25, 14 });
		paddle->setScale({ 25, 14 });
		m_paddle = paddle;
		panel->addElement(m_paddle);

		// Add a label indicating the toggle state
		m_label = std::make_shared<Label>(engine, std::to_string(value), this);
		m_label->setAlignment(Label::align_right);
		m_label->setTextScale(12.0f);
		m_label->setMaxScale(glm::vec2(30.0f, m_label->getMaxScale().y));
		m_label->setColor(glm::vec3(0.75f));
		m_label->setMaxScale({ 50, 28 });
		m_label->setScale({ 50, 28 });
		m_label->setPosition({ -225, 0 });
		addElement(m_label);

		setMaxScale({ 250, 28 });
		setMinScale({ 250, 28 });
		setValue(value);
	}


	// Public Interface Implementation
	inline virtual void update() override {
		UI_Element::update();
		if (!m_paddle) return;
		m_paddle->setPosition({ (2.0f * ((m_value - m_lowerRange) / (m_upperRange - m_lowerRange)) - 1.0f) * (200.0f - m_paddle->getScale().x), 0 });
	}
	inline virtual void mouseAction(const MouseEvent & mouseEvent) override {
		UI_Element::mouseAction(mouseEvent);
		if (getVisible() && getEnabled() && mouseWithin(mouseEvent)) {
			if (m_pressed && mouseEvent.m_action == MouseEvent::MOVE) {
				const float mx = float(mouseEvent.m_xPos) - m_position.x - m_backPanel->getPosition().x + m_backPanel->getScale().x;
				setValue(((mx / (m_backPanel->getScale().x * 2.0f)) * (m_upperRange - m_lowerRange)) + m_lowerRange);
			}
		}
	}
	inline virtual void userAction(ActionState & actionState) override {
		const float offsetAmount = std::min<float>((m_upperRange - m_lowerRange) / 100.0f, 1.0f);
		if (actionState.isAction(ActionState::UI_LEFT) == ActionState::PRESS)
			setValue(getValue() - offsetAmount);
		else if (actionState.isAction(ActionState::UI_RIGHT) == ActionState::PRESS)
			setValue(getValue() + offsetAmount);
		else if (actionState.isAction(ActionState::UI_ESCAPE) == ActionState::PRESS)
			focusParent();
	}


	// Public Methods
	/** Set the percentage for this slider.
	@param	amount		the value to put this slider at. */
	inline void setValue(const float & amount) {
		m_value = std::clamp<float>(amount, m_lowerRange, m_upperRange);
		setText(std::to_string((int)std::round<int>(m_value)));
		update();
		enactCallback(on_value_change);
	}
	/** Get the percentage value for this scrollbar.
	@return				the percentage value for this slider. */
	inline float getValue() const {
		return m_value;
	}
	/** Set the lower and upper ranges for this slider.
	@param	lowerRange	the lowest number this slider can use.
	@param	upperRange	the highest number this slider can use. */
	inline void setRanges(const float & lowerRange, const float & upperRange) {
		m_lowerRange = lowerRange;
		m_upperRange = upperRange;
		m_value = std::clamp<float>(m_value, m_lowerRange, m_upperRange);
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


protected:
	// Protected Attributes
	float m_value = 0.0f, m_lowerRange = 0.0f, m_upperRange = 1.0f;
	std::shared_ptr<Label> m_label;
	std::shared_ptr<UI_Element> m_backPanel, m_paddle;
};

#endif // UI_SLIDER_H