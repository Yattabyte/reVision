#pragma once
#ifndef MOUSEEVENT_H
#define MOUSEEVENT_H


/** Holds mouse interaction information. */
struct MouseEvent {
	// Public Attributes
	double m_xPos = 0.0, m_yPos = 0.0;
	int m_mods = 0;


	// Public Enumerations
	/** The action states a key can be in: released, pressed, etc. */
	enum class Action : int {
		RELEASE = 0,
		PRESS = 1,
		MOVE = 2,
	} m_action = Action::RELEASE;
	/** The name of all keys supported, and an appropriate key code for each of them. */
	enum class Key : int {
		LEFT = 0,
		RIGHT = 1,
		MIDDLE = 2,
	} m_button = Key::LEFT;
};

#endif // MOUSEEVENT_H