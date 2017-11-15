/*
	Component

	- An extendable container like object
	- To be derived/inherited from to accomplish a specific goal
*/

#pragma once
#ifndef COMPONENT
#define COMPONENT
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

struct Component
{
	~Component() {};
	Component() {};
	virtual unsigned int GetTypeID() { return -1; }
};

#endif // COMPONENT