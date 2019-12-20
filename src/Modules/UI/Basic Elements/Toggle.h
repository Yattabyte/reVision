#pragma once
#ifndef UI_TOGGLE_H
#define UI_TOGGLE_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Label.h"
#include "Modules/UI/Basic Elements/Panel.h"
#include "Modules/UI/Decorators/Border.h"


/** UI toggle switch class, affords being switched left and right. */
class Toggle final : public UI_Element {
public:
	// Public Interaction Enums
	enum class Interact : int {
		on_toggle = (int)UI_Element::Interact::last_interact_index
	};


	// Public (De)Constructors
	/** Destroy this toggle switch. */
	inline ~Toggle() noexcept = default;
	/** Construct a toggle switch with a given on/off state.
	@param	engine		reference to the engine to use. 
	@param	state		the on/off state to use. */
	explicit Toggle(Engine& engine, const bool& state = true);


	// Public Interface Implementation
	void renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) final;
	void userAction(ActionState& actionState) final;


	// Public Methods
	/** Set this slider's text.
	@param	text	the text to use. */
	void setText(const std::string& text);
	/** Retrieve this slider's text.
	@return			the text this label uses. */
	std::string getText() const;
	/** Set the toggle state of this button.
	@param	state	the new state to use. */
	void setToggled(const bool& state);
	/** Retrieve the toggle state of this button.
	@return			whether or not this toggle is on or off. */
	bool isToggled() const noexcept;


protected:
	// Protected Methods
	/** Update the data dependant on the scale of this element. */
	void updateGeometry();


	// Protected Attributes
	bool m_toggledOn = true;
	std::shared_ptr<Label> m_label;
	std::shared_ptr<Border> m_backPanel;
	std::shared_ptr<Panel> m_paddle;
};

#endif // UI_TOGGLE_H