#pragma once
#ifndef FACTORY_ADT
#define FACTORY_ADT
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif


/**
 * An abstract data type encapsulating the 'Factory Design Pattern', requires an implementation.
 * @param	<T_Obj>		The type of element to be created by the factory
 **/
template <typename T_Obj>
class DT_ENGINE_API Factory
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

#endif // FACTORY_ADT