#pragma once
#ifndef VISIBILITY_TOKEN
#define VISIBILITY_TOKEN
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\World\ECS\Components\Component.h"
#include "Systems\World\ECS\ECSdefines.h"
#include "Utilities\MappedChar.h"


/**
 * An object that holds a list of components considered to be visible.
 **/
class Visibility_Token {
public:
	// (de)Constructors
	/** Destroy the token. */
	Visibility_Token() {}
	/** Construct the token. */
	~Visibility_Token() {}
	

	// Public Methods
	/** Insert a component type into the map ahead of time. 
	 * @param	name	the name of the type to insert */
	void insertType(const char * name) {
		mList.insert(name);
	}
	/** Get a type-casted list of components that match the given type
	 * @param	name	the name of the type to retrieve
	 * @param	<T>		the type to down-cast the components to.
	 * @return			a type-casted list of components matching the supplied type */
	template <typename T> 
	const vector<T*>& getTypeList(const char * name) const {
		return *(vector<T*>*)(&mList.at(name));
	}
	/** Retrieve a list of components of the given type.
	 * @param	name	the name of the type to retrieve
	 * @return			the list of components matching the supplied type */
	vector<Component*>& operator[](const char * name) {
		return mList.at(name);
	}
	/** Retrieve the size of the map.
	 * @return			the size of the map */
	size_t size() const {
		return mList.size();
	}
	/** Retrieve the size of a list within the map.
	* @return			the size of the list */
	const size_t specificSize(const char * name) const {
		return mList.at(name).size();
	}
	/** Check if the given component type. 
	 * @param	name	the name of the type to retrieve
	 * @return			true if found, false otherwise */
	bool find(const char * name) const {
		return mList.find(name);
	}


	// Public Attributes
	VectorMap<Component*> mList;
};

#endif // VISIBILITY_TOKEN