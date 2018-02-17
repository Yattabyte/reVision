#pragma once
#ifndef VISIBILITY_TOKEN
#define VISIBILITY_TOKEN
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Entities\Components\Component.h"
#include "Systems\World\ECSdefines.h"
#include <map>
#include <vector>


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
	void insertType(char * name) {
		mList.insert(pair<char*, vector<Component*>>(name, vector<Component*>()));
	}
	/** Get a type-casted list of components that match the given type
	 * @param	name	the name of the type to retrieve
	 * @param	<T>		the type to down-cast the components to.
	 * @return	a type-casted list of components matching the supplied type */
	template <typename T> 
	const std::vector<T*>& getTypeList(char * name) const {
		return *(vector<T*>*)(&mList.at(name));
	}
	/** Retrieve a list of components of the given type.
	 * @param	name	the name of the type to retrieve
	 * @return	the list of components matching the supplied type */
	std::vector<Component*>& operator[](char * name) {
		return mList.at(name);
	}
	/** Retrieve the size of the map.
	 * @return	the size of the map */
	size_t size() const {
		return mList.size();
	}
	/** Check if the given component type. 
	 * @param	name	the name of the type to retrieve */
	bool find(char * name) const {
		return !(mList.find(name) == mList.end());
	}


	// Public Attributes
	std::map<char*, std::vector<Component*>, cmp_str> mList;
};

#endif // VISIBILITY_TOKEN