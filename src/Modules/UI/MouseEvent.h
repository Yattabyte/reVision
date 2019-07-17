#pragma once
#ifndef MOUSEEVENT_H
#define MOUSEEVENT_H


/** Holds mouse interaction information. */
class MouseEvent {
public:
	// Public Interaction Enums
	/** The action states a mouse can be in: released, pressed, etc. */
	enum Actions;
	/** The name of all keys supported, and an appropriate key code for each of them. */
	enum Key : int;


	// Public (de)Constructors
	/** Destroy the mouse event. */
	inline ~MouseEvent() = default;
	/** Construct a mouse event. */
	inline MouseEvent() = default;


	// Public Attributes
	double m_xPos = 0.0, m_yPos = 0.0;
	int m_button = 0, m_action = 0, m_mods = 0;


	// Enumeration Implementation
	enum Action {
		RELEASE,
		PRESS,
		MOVE,
	};
	enum Key : int {
		LEFT = 0,
		RIGHT,
		MIDDLE,
	};
};

#endif // MOUSEEVENT_H