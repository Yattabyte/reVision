#pragma once
#ifndef SYSTEM_INPUT_H
#define SYSTEM_INPUT_H

#include "Systems\System_Interface.h"
#include "Systems\Input\InputBinding.h"

class EnginePackage;
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
	 * @param	bind_interface	an optional bindingInterface. Will default to loading binds.cfg*/
	System_Input(const InputBinding & bind_interface = InputBinding());


	// Interface Implementations
	virtual void initialize(EnginePackage * enginePackage);
	virtual void update(const float & deltaTime);
	virtual void updateThreaded(const float & deltaTime) {};


private:
	// Private Attributes
	InputBinding m_binds; 
};

#endif // SYSTEM_INPUT_H