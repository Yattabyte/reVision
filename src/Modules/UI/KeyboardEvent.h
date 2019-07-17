#pragma once
#ifndef KEYBOARD_EVENT_H
#define KEYBOARD_EVENT_H

#include <map>


/** Holds keyboard interaction information. */
class KeyboardEvent {
public:
	// Public Interaction Enums
	/** The action states a key can be in: released, pressed, etc. */
	enum Action;
	/** The name of all keys supported, and an appropriate key code for each of them. */
	enum Key : unsigned int;


	// Public (de)Constructors
	/** Destroy the keyboard event. */
	inline ~KeyboardEvent() = default;
	/** Construct a keyboard event. */
	inline KeyboardEvent() = default;


	// Public Methods
	/** Retrieve the keycode for the current key pressed. 
	@return					the current key pressed. */
	inline unsigned int getChar() const {
		return m_currentChar;
	}
	/** Set a keycode as the current key being pressed. 
	@param	currentChar		the key to set as pressed. */
	inline void setChar(const unsigned int & currentChar) {
		m_currentChar = currentChar;
	}
	/** Retrieve the key action state for a given key type.
	@param	key				the key to check the state of.
	@return					the action state for the given key. */
	inline KeyboardEvent::Action getState(const Key & key) const {
		if (m_keyStates.find(key) != m_keyStates.end())
			return m_keyStates.at(key);
		return KeyboardEvent::Action::RELEASE;
	}
	/** Set the action state for a given key.
	@param	key				the key to set the state for.
	@param	action			the state to set for the given key. */
	inline void setState(const Key & key, const KeyboardEvent::Action & action) {
		m_keyStates[key] = action;
	}


protected:
	// Protected Attributes
	/** The key being pressed in a char stream. */
	unsigned int m_currentChar = 0;
	/** Map of key states over time, e.g. history if one is pressed or released. */
	std::map<unsigned int, KeyboardEvent::Action> m_keyStates;


public:
	// Enumeration Implementation
	enum Action {
		RELEASE,
		PRESS,
		REPEAT
	};
	enum Key : unsigned int {
		/* Printable keys */
		SPACE = 32,
		APOSTROPHE = 39,
		COMMA = 44,
		MINUS,
		PERIOD,
		SLASH,
		ZERO,
		ONE,
		TWO,
		THREE,
		FOUR,
		FIVE,
		SIX,
		SEVEN,
		EIGHT,
		NINE,
		SEMICOLON = 59,
		EQUAL = 61,
		A = 65,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,
		LEFT_BRACKET,
		BACKSLASH,
		RIGHT_BRACKET,
		GRAVE_ACCENT = 96,
		WORLD_1 = 161,
		WORLD_2,

		/* Function keys */
		ESCAPE = 256,
		ENTER,
		TAB,
		BACKSPACE,
		INSERT,
		DEL,
		RIGHT,
		LEFT,
		DOWN,
		UP,
		PAGE_UP,
		PAGE_DOWN,
		HOME,
		END,
		CAPS_LOCK = 280,
		SCROLL_LOCK,
		NUM_LOCK,
		PRINT_SCREEN,
		PAUSE,
		F1 = 290,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		F13,
		F14,
		F15,
		F16,
		F17,
		F18,
		F19,
		F20,
		F21,
		F22,
		F23,
		F24,
		F25,
		KP_0 = 320,
		KP_1,
		KP_2,
		KP_3,
		KP_4,
		KP_5,
		KP_6,
		KP_7,
		KP_8,
		KP_9,
		KP_DECIMAL,
		KP_DIVIDE,
		KP_MULTIPLY,
		KP_SUBTRACT,
		KP_ADD,
		KP_ENTER,
		KP_EQUAL,
		LEFT_SHIFT = 340,
		LEFT_CONTROL,
		LEFT_ALT,
		LEFT_SUPER,
		RIGHT_SHIFT,
		RIGHT_CONTROL,
		RIGHT_ALT,
		RIGHT_SUPER,
		MENU,
	};
};

#endif // KEYBOARD_EVENT_H