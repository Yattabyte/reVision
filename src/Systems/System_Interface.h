#pragma once
#ifndef SYSTEM_INTERFACE_H
#define SYSTEM_INTERFACE_H

class Engine;


/**
 * An abstract class representing an engine system. 
 **/
class System
{
public:
	// (de)Constructors
	/** Virtual Destructor. */
	virtual ~System() {};
	/** Constructor. */
	System() { m_Initialized = false; }
	/** Initializes the system.
	 * @param	engine	the engine pointer */
	virtual void initialize(Engine * engine) = 0;
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
	Engine * m_engine;
};

#endif // SYSTEM_INTERFACE_H