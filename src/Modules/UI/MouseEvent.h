#pragma once
#ifndef MOUSEEVENT_H
#define MOUSEEVENT_H


/** Holds mouse interaction information. */
class MouseEvent {
public:
	// Public (De)Constructors
	/** Destroy the mouse event. */
	inline ~MouseEvent() = default;
	/** Construct a mouse event. */
	inline MouseEvent() = default;


	// Public Attributes
	double m_xPos = 0.0, m_yPos = 0.0;
	int m_mods = 0;
	enum class Action : int {
		RELEASE = 0,
		PRESS = 1,
		MOVE = 2,
	} m_action = Action::RELEASE;
	enum class Key : int {
		LEFT = 0,
		RIGHT = 1,
		MIDDLE = 2,
	} m_button = Key::LEFT;
};

#endif // MOUSEEVENT_H
