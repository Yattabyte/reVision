#pragma once
#ifndef CALLBACK_CONTAINER
#define CALLBACK_CONTAINER
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

class Preference_State;


/**
 * An interface for preference callbacks. 
 * Re-implement to dictate what to do following a preference change.
 **/
class DT_ENGINE_API Callback_Container {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	virtual ~Callback_Container() {};
	/** Constructor. */
	Callback_Container() {};


	// Public Attributes
	Preference_State *m_preferenceState;


private:
	// Private Methods
	/** Gets called when the preference (tied to this class) changes. */
	virtual void Callback(const float & value) = 0;


	// Private Attributes
	friend class Preference_State;
};

#endif // CALLBACK_CONTAINER