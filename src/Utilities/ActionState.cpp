#include "Utilities/ActionState.h"


float& ActionState::operator[](const ActionState::Action& index) 
{
	return std::get<1>(m_keyStates[index]);
}

const float& ActionState::operator[](const ActionState::Action& index) const 
{
	return std::get<1>(m_keyStates.at(index));
}

ActionState::State ActionState::isAction(const ActionState::Action& actionEnum) 
{
	if (m_keyStates.find(actionEnum) != m_keyStates.end()) {
		auto& [state, amount] = m_keyStates[actionEnum];
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