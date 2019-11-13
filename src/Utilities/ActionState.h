#pragma once
#ifndef ACTION_STATE_H
#define ACTION_STATE_H

#include <map>
#include <string>
#include <vector>


/** A container class that holds the action state for the engine, such as forward/back/left/right and amount. */
class ActionState final {
public:
	// Public (De)Constructors
	/** Destroy the action state. */
	inline ~ActionState() = default;
	/** Construct the action state. */
	inline ActionState() {
		for (unsigned int x = 0; x < (unsigned int)Action::ACTION_COUNT; ++x)
			m_keyStates.insert({ Action(x), { false, 0.0f } });
	}


	// Public Static Enumerations
	/** Enumeration for whether the action key was pressed, released, or repeating. */
	enum class State : int {
		RELEASE,
		PRESS,
		REPEAT
	};
	/** Enumeration for indexing into actions. */
	enum class Action : unsigned int {
		MOUSE_X,
		MOUSE_Y,
		MOUSE_L,
		MOUSE_R,
		LOOK_X,
		LOOK_Y,
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT,
		JUMP,
		CROUCH,
		RUN,
		FIRE1,
		FIRE2,
		UI_UP,
		UI_DOWN,
		UI_LEFT,
		UI_RIGHT,
		UI_ENTER,
		UI_ESCAPE,
		ACTION_COUNT
	};


	// Public Static Methods
	/** Retrieve a static list of all input-actions.
	@return	std::vector of action names as strings */
	static std::vector<std::string> Action_Strings() {
		static const std::vector<std::string> actionStrings = {
			"MOUSE_X",
			"MOUSE_Y",
			"MOUSE_L",
			"MOUSE_R",
			"LOOK_X",
			"LOOK_Y",
			"FORWARD",
			"BACKWARD",
			"LEFT",
			"RIGHT",
			"JUMP",
			"CROUCH",
			"RUN",
			"FIRE1",
			"FIRE2",
			"UI_UP",
			"UI_DOWN",
			"UI_LEFT",
			"UI_RIGHT",
			"UI_ENTER",
			"UI_ESCAPE",
		};
		return actionStrings;
	};


	// Public Methods
	/***/
	inline float& operator[](const ActionState::Action& index) {
		return std::get<1>(m_keyStates[index]);
	}
	inline const float& operator[](const ActionState::Action& index) const {
		return std::get<1>(m_keyStates.at(index));
	}
	inline ActionState::State isAction(const ActionState::Action& actionEnum) {
		return isAction(actionEnum, &m_keyStates);
	}
	inline ActionState::State isAction(const ActionState::Action& actionEnum, std::map<ActionState::Action, std::pair<bool, float>>* keyStates) const {
		if (m_keyStates.find(actionEnum) != m_keyStates.end()) {
			auto& [state, amount] = (*keyStates)[actionEnum];
			if (m_keyStates.at(actionEnum).second > 0.5f) {
				if (!state) {
					state = true;
					return ActionState::State::PRESS;
				}
				return ActionState::State::REPEAT;
			}
			else
				state = false;
		}
		return ActionState::State::RELEASE;
	}


protected:
	// Protected Attributes
	std::map<ActionState::Action, std::pair<bool, float>> m_keyStates;
};

#endif // ACTION_STATE_H