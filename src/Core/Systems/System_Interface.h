/*
	System_Interface

	- An abstract class for an engine system
	- To be expanded upon to complete a particular goal
*/



#pragma once
#ifndef SYSTEM_INTERFACE
#define SYSTEM_INTERFACE
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

class Engine_Package;
class DT_ENGINE_API System
{
public:
	// All systems need to implement their own destructor
	virtual ~System() {};

	// Create da constructor
	System() { m_Initialized = false; }
	
	virtual void Initialize(Engine_Package * enginePackage) {}

	// All systems can implement their own shutdown function
	virtual void Shutdown() {};

	// All systems can implement their own update function
	virtual void Update(const float &deltaTime) {};

	// An optional secondary threaded update function
	virtual void Update_Threaded(const float &deltaTime) {};

protected:
	bool m_Initialized;
	Engine_Package *m_enginePackage;
};

#endif // SYSTEM_INTERFACE