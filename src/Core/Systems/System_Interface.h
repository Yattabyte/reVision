/*
	System_Interface

	- An abstract class for an engine system
	- To be expanded upon to complete a particular goal
*/



#pragma once
#ifndef SYSTEM_INTERFACE
#define SYSTEM_INTERFACE
#ifdef	ENGINE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

class DELTA_CORE_API System
{
public:
	// All systems need to implement their own destructor
	virtual ~System() {};

	// All systems need to implement their own update function
	virtual void Update(const float &deltaTime) = 0;
};

#endif // SYSTEM_INTERFACE