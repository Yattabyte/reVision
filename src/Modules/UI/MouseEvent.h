#pragma once
#ifndef MOUSEEVENT_H
#define MOUSEEVENT_H


/** Holds mouse interaction information. */
class MouseEvent
{
public:
	// Public Interaction Enums
	enum Actions {
		RELEASE = 0,
		PRESS = 1,
		MOVE = 2,
	};


	// Public (de)Constructors
	/** Destroy the mouse event. */
	inline ~MouseEvent() = default;
	/** Construct a mouse event. */
	inline MouseEvent() = default;


	// Public Attributes
	double m_xPos = 0.0, m_yPos = 0.0;
	int m_button = 0, m_action = 0, m_mods = 0;
};

#endif // MOUSEEVENT_H