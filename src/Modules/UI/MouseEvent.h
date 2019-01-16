#pragma once
#ifndef MOUSEEVENT_H
#define MOUSEEVENT_H


/** Holds mouse interaction information. */
class MouseEvent
{
public:
	// (de)Constructors
	~MouseEvent() = default;
	MouseEvent() = default;


	// Public Attributes
	double m_xPos = 0.0, m_yPos = 0.0;
	int m_button, m_action, m_mods;
};

#endif // MOUSEEVENT_H
