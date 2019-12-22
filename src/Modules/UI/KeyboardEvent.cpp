#include "Modules/UI/KeyboardEvent.h"


KeyboardEvent::KeyboardEvent() noexcept
{
}

unsigned int KeyboardEvent::getChar() const noexcept
{
	return m_currentChar;
}

void KeyboardEvent::setChar(const unsigned int& currentChar) noexcept 
{
	m_currentChar = currentChar;
}

KeyboardEvent::Action KeyboardEvent::getState(const KeyboardEvent::Key& key) const 
{
	if (m_keyStates.find(key) != m_keyStates.end())
		return m_keyStates.at(key);
	return KeyboardEvent::Action::RELEASE;
}

void KeyboardEvent::setState(const KeyboardEvent::Key& key, const KeyboardEvent::Action& action) 
{
	m_keyStates[key] = action;
}