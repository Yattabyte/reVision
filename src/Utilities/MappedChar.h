#pragma once
#ifndef CUSTOMMAP
#define CUSTOMMAP
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include <map>
#include <vector>

using namespace std;


/**
 * A std::map that uses a constant char array as the keys, and stores values of type <T>.\n
 * Uses the adapter pattern to hide some of the complexity and reduce the amount of redundant code written.
 * Also provides easier insertion, lookup, and boolean find operations.
 * @param	<T>		any type to store.
 **/
template <typename T>
class DT_ENGINE_API MappedChar {
private:
	// Nested Private Members
	struct compare_string { bool operator()(const char * a, const char * b) const { return strcmp(a, b) < 0; } };
	typedef typename map<const char *, T>::iterator iterator;
	typedef typename map<const char *, T>::const_iterator const_iterator;


	// Private Attributes
	map<const char *, T, compare_string> m_map;


public:
	// (de)Constructors
	/** Destroy the map. */
	~MappedChar()											{ };
	/** Construct the map. */
	MappedChar()											{ };


	// Public Methods
	/** Insert a new key into the map.
	 * @brief	will auto-generate a new zero-initialized <T> to associate with the key.
	 * @param	key		the new key to insert into the map */	
	void				insert(const char * key)			{ m_map.insert(pair<const char *, T>(key, T())); }
	/** Clears the map of all entries. */
	void				clear()								{ m_map.clear(); }	
	/** Retrieve the number of entries into the map.
	 * @return			the size of the map. */
	size_t				size()						const	{ return m_map.size(); }
	/** Retrieve the element associated with the given key.
	 * @param	key		the key associated with the element to retrieve
	 * @return			the element paired with the key supplied */
	T &					at(const char * key)				{ return m_map.at(key); }
	/** Retrieve the element associated with the given key.
	 * @param	key		the key associated with the element to retrieve
	 * @return			the element paired with the key supplied */
	const T &			at(const char * key)		const	{ return m_map.at(key); }
	/** Retrieve the element associated with the given key.
	 * @note			will insert the key into the map if they lookup fails, guaranteed to return a zero-initialized <T>.\nIf this is undesired, used .at()
	 * @param	key		the key associated with the element to retrieve
	 * @return			the element paired with the key supplied */
	T &					operator[](const char * key)		{	return m_map[key];	}

	// Navigation
	/** The beginning iterator.
	 * @return			the beginning iterator */
	iterator			begin()								{ return m_map.begin();	}
	/** The beginning constant iterator.
	* @return			the beginning constant iterator */
	const_iterator		begin()						const	{ return m_map.cbegin(); }
	/** The ending iterator.
	* @return			the ending iterator */
	iterator			end()								{ return m_map.end(); }
	/** The ending constant iterator.
	* @return			the ending constant iterator */
	const_iterator		end()						const	{ return m_map.cend(); }	
	/** Check if the given key exists in the map.
	 * @return			true if the key exists, false otherwise */
	bool				find(const char * key)		const	{ return find_I(key) != end(); }
	/** Retrieves an iterator of the element associated with the key given.
	 * @return			the iterator to the element desired, or the terminating iterator ('end()') if not found */
	iterator			find_I(const char * key)			{ return m_map.find(key); }
	/** Retrieves a constant iterator of the element associated with the key given.
	 * @return			the constant iterator to the element desired, or the terminating constant iterator ('end()') if not found */
	const_iterator		find_I(const char * key)	const	{ return m_map.find(key); }
};


/**
 * A std::map that uses a constant char array as the keys, and stores vectors of type <T>.\n
 * Extends the MappedChar class, but exists to shorten 'MappedChar<vector<T>> myMap' down to 'VectorMap<T> myMap'
 **/
template <typename T>
class DT_ENGINE_API VectorMap : public MappedChar<vector<T>> {
public:
	~VectorMap() {};
	VectorMap() {};
};

#endif // CUSTOMMAP