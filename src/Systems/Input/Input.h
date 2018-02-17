#pragma once
#ifndef SYSTEM_INPUT
#define SYSTEM_INPUT
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\System_Interface.h"
#include "Systems\Input\Input_Binding.h"

class EnginePackage;
class GLFWwindow;


/**
 * An engine system responsible for receiving user input from various peripheral devices, and converting it into standardized engine inputs.
 **/
class DT_ENGINE_API System_Input : public System
{
public:
	// (de)Constructors
	/** Destroy the input system. */
	~System_Input();
	/** Construct the input system. 
	 * @param	bind_interface	an optional bindingInterface. Will default to loading binds.cfg*/
	System_Input(const Input_Binding & bind_interface = Input_Binding());


	// Interface Implementations
	virtual void initialize(EnginePackage * enginePackage);
	virtual void update(const float & deltaTime);
	virtual void updateThreaded(const float & deltaTime) {};


private:
	// Private Attributes
	Input_Binding m_binds; 
};

#endif // SYSTEM_INPUT