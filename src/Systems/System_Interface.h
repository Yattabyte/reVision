#pragma once
#ifndef SYSTEM_INTERFACE
#define SYSTEM_INTERFACE
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

class EnginePackage;


/**
 * An abstract class representing an engine system. 
 **/
class DT_ENGINE_API System
{
public:
	// (de)Constructors
	/** Virtual Destructor*/
	virtual ~System() {};
	/** Constructor*/
	System() { m_Initialized = false; }
	/** Initializes the system.
	 * @param	enginePackage	the engine package */
	virtual void initialize(EnginePackage * enginePackage) = 0;
	/** Tell the system to update, typically called within the main loop.
	 * @param	deltaTime		the time since last update */
	virtual void update(const float & deltaTime) = 0;
	/** Tell the system to update in a separate thread.
	 * @brief					can be used if this system supports multi-threading.
	 * @param	deltaTime		the time since last update */
	virtual void updateThreaded(const float & deltaTime) = 0;


protected:
	// Protected Attributes
	bool m_Initialized;
	EnginePackage *m_enginePackage;
};

#endif // SYSTEM_INTERFACE