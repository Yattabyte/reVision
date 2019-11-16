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
	enum class Interact : int {
		on_toggle = (int)UI_Element::Interact::last_interact_index
	};


	// Public (De)Constructors
	/** Destroy the toggle switch. */
	inline ~Toggle() = default;
	/** Construct a toggle switch with a given on/off state.
	@param	engine		the engine to use.
	@param	state		the on/off state to use. */
	inline explicit Toggle(Engine* engine, const bool& state = true) noexcept :
		UI_Element(engine)
	{
		// Make a background panel for cosmetic purposes
		auto panel = std::make_shared<Panel>(engine);
		panel->setColor(glm::vec4(0.3f));
		m_backPanel = std::make_shared<Border>(engine, panel);
		addElement(m_backPanel);

		// Create the sliding paddle
		m_paddle = std::make_shared<Panel>(engine);
		m_paddle->setColor(glm::vec4(0.75f));
		panel->addElement(m_paddle);

		// Add a label indicating the toggle state
		m_label = std::make_shared<Label>(engine);
		m_label->setAlignment(Label::Alignment::align_right);
		m_label->setTextScale(12.0f);
		m_label->setColor(glm::vec3(0.75f));
		addElement(m_label);

		// Callbacks
		addCallback((int)UI_Element::Interact::on_clicked, [&]() { setToggled(!m_toggledOn); });
		addCallback((int)UI_Element::Interact::on_resize, [&]() { updateGeometry(); });

		// Configure THIS element
		setToggled(state);
	}


	// Public Interface Implementation
	inline virtual void renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) noexcept override {
		// Update Colors
		glm::vec4 color(0.75);
		if (m_pressed)
			color *= 0.5f;
		if (m_hovered)
			color *= 1.5f;
		m_paddle->setColor(color);

		// Render Children
		UI_Element::renderElement(deltaTime, position, scale);
	}
	inline virtual void userAction(ActionState& actionState) noexcept override {
		if (actionState.isAction(ActionState::Action::UI_LEFT) == ActionState::State::PRESS)
			setToggled(false);
		else if (actionState.isAction(ActionState::Action::UI_RIGHT) == ActionState::State::PRESS)
			setToggled(true);
		else if (actionState.isAction(ActionState::Action::UI_ENTER) == ActionState::State::PRESS)
			setToggled(!m_toggledOn);
	}


	// Public Methods
	/** Set this slider's text.
	@param	text	the text to use. */
	inline void setText(const std::string& text) noexcept {
		m_label->setText(text);
	}
	/** Retrieve this slider's text.
	@return			the text this label uses. */
	inline std::string getText() const noexcept {
		return m_label->getText();
	}
	/** Set the toggle state of this button.
	@param	state	the new state to use. */
	inline void setToggled(const bool& state) noexcept {
		m_toggledOn = state;
		setText(m_toggledOn ? "ON" : "OFF");
		updateGeometry();
		enactCallback((int)Toggle::Interact::on_toggle);
	}
	/** Return the toggle state of this button.
	@return			whether or not this toggle is on or off. */
	inline bool getToggled() const noexcept {
		return m_toggledOn;
	}


protected:
	// Protected Methods
	/** Update the data dependant on the scale of this element. */
	inline void updateGeometry() noexcept {
		// Shorten the back panel by 50 units, and it is offset to the right by 50 units
		m_backPanel->setPosition({ 50, 0 });
		m_backPanel->setScale(glm::vec2(getScale().x - m_backPanel->getPosition().x, getScale().y));
		// Move the label to the left side of the back panel
		m_label->setPosition(-glm::vec2(getScale().x - 25, 0));
		m_label->setScale({ 50, 28 });
		// The paddle fills HALF of the back panel, and is positioned on one of the 2 sides of it.
		m_paddle->setScale(glm::vec2((getScale().x - 50) / 2.0f, getScale().y - 2.0f) - 2.0f);
		m_paddle->setPosition(glm::vec2(m_paddle->getScale().x, 0) * (m_toggledOn ? 1.0f : -1.0f));
	}


	// Protected Attributes
	bool m_toggledOn = true;
	std::shared_ptr<Label> m_label;
	std::shared_ptr<Border> m_backPanel;
	std::shared_ptr<Panel> m_paddle;
};

#endif // UI_TOGGLE_H
