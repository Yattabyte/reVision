#pragma once
#ifndef MAPPEDCHAR_H
#define MAPPEDCHAR_H

#include <map>
#include <vector>


/** A std::map that uses a constant char array as the keys, and stores values of type <T>.\n
Uses the adapter pattern to hide some of the complexity and reduce the amount of redundant code written.
Also provides easier insertion, lookup, and boolean find operations.
@param	<T>		any type to store. */
template <typename T>
class MappedChar {
private:
	// Nested Private Members
	struct compare_string { bool operator()(const char * a, const char * b) const { return strcmp(a, b) < 0; } };
	typedef typename std::map<const char *, T>::iterator iterator;
	typedef typename std::map<const char *, T>::const_iterator const_iterator;


	// Private Attributes
	std::map<const char *, T, compare_string> m_map;


public:
	// (de)Constructors
	/** Destroy the map. */
	~MappedChar() = default;
	/** Construct the map. */
	MappedChar() = default;


	// Public Methods
	/** Insert a new key into the map.
	@brief	will auto-generate a new zero-initialized <T> to associate with the key.
	@param	key		the new key to insert into the map */	
	inline void insert(const char * key) {
		m_map.insert(std::pair<const char *, T>(key, T())); 
	}
	/***/
	inline void erase(const char * key) {
		auto spot = m_map.find(key);
		if (spot != m_map.end())	
			m_map.erase(spot);		
	}
	/** Clears the map of all entries. */
	inline void	clear() { 
		m_map.clear(); 
	}
	/** Retrieve the number of entries into the map.
	@return			the size of the map. */
	inline size_t size() const {
		return m_map.size(); 
	}
	/** Retrieve the element associated with the given key.
	@param	key		the key associated with the element to retrieve
	@return			the element paired with the key supplied */
	inline T & at(const char * key) {
		return m_map.at(key);
	}
	/** Retrieve the element associated with the given key.
	@param	key		the key associated with the element to retrieve
	@return			the element paired with the key supplied */
	inline const T & at(const char * key) const {
		return m_map.at(key);
	}
	/** Retrieve the element associated with the given key.
	@note			will insert the key into the map if they lookup fails, guaranteed to return a zero-initialized <T>.\nIf this is undesired, used .at()
	@param	key		the key associated with the element to retrieve
	@return			the element paired with the key supplied */
	inline T & operator[](const char * key) {
		return m_map[key];
	}


	// Navigation
	/** The beginning iterator.
	@return			the beginning iterator */
	inline iterator begin() {
		return m_map.begin();
	}
	/** The beginning constant iterator.
	@return			the beginning constant iterator */
	inline const_iterator begin() const {
		return m_map.cbegin();
	}
	/** The ending iterator.
	@return			the ending iterator */
	inline iterator end() {
		return m_map.end();
	}
	/** The ending constant iterator.
	@return			the ending constant iterator */
	inline const_iterator end() const {
		return m_map.cend();
	}
	/** Check if the given key exists in the map.
	@return			true if the key exists, false otherwise */
	inline bool find(const char * key) const {
		return find_I(key) != end();
	}
	/** Retrieves an iterator of the element associated with the key given.
	@return			the iterator to the element desired, or the terminating iterator ('end()') if not found */
	inline iterator	find_I(const char * key) {
		return m_map.find(key);
	}
	/** Retrieves a constant iterator of the element associated with the key given.
	@return			the constant iterator to the element desired, or the terminating constant iterator ('end()') if not found */
	inline const_iterator find_I(const char * key) const {
		return m_map.find(key);
	}
};

/** A std::map that uses a constant char array as the keys, and stores vectors of type <T>.\n
Extends the MappedChar class, but exists to shorten 'MappedChar<std::vector<T>> myMap' down to 'VectorMap<T> myMap'. */
template <typename T>
class VectorMap : public MappedChar<std::vector<T>> {
public:
	~VectorMap() = default;
	VectorMap() = default;
};

#endif // MAPPEDCHAR_H