#pragma once
#ifndef SYSTEM_INPUT_H
#define SYSTEM_INPUT_H

#include "Systems\System_Interface.h"
#include "Systems\Input\InputBinding.h"


class Engine;
class GLFWwindow;

/**
 * An engine system responsible for receiving user input from various peripheral devices, and converting it into standardized engine inputs.
 **/
class System_Input : public System
{
public:
	// (de)Constructors
	/** Destroy the input system. */
	~System_Input();
	/** Construct the input system. 
	 * @param	engine			the engine
	 * @param	filename		an optional relative path to a key-bind file to load. Will default to loading binds.cfg*/
	System_Input(Engine * engine, const std::string & filename = "binds");


	// Interface Implementations
	virtual void initialize(Engine * engine);
	virtual void update(const float & deltaTime);
	virtual void updateThreaded(const float & deltaTime) {};


private:
	// Private Attributes
	InputBinding m_binds; 
};

#endif // SYSTEM_INPUT_H