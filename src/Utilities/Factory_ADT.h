#pragma once
#ifndef FACTORY_ADT_H
#define FACTORY_ADT_H


/**
 * An abstract data type encapsulating the 'Factory Design Pattern', requires an implementation.
 * @param	<T_Obj>		The type of element to be created by the factory
 **/
template <typename T_Obj>
class Factory
{
public:
	// (de)Constructors
	/** Virtual Destructor. */
	virtual ~Factory() = default;
	/** Constructor. */
	Factory() = default;
	/** Creates an object of the type specified
	 * @return	the created object */
	T_Obj createObject() = 0;
};

#endif // FACTORY_ADT_H