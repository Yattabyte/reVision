#pragma once
#ifndef PRIORITY_LIST_H
#define PRIORITY_LIST_H

#include <vector>

using namespace std;


/**
 * A list which provides automatic insertion sort functionality.
 * @param	<T_key>		the sorting key type
 * @param	<T_elmt>	the element to store type
 * @param	<T_compare>	the comparator type, defaults to less (an ascending list)
 **/
template <typename T_key, typename T_elmt, typename T_compare = less<T_key>>
class PriorityList
{
private:
	// Private Attributes
	/** Nested element struct */
	struct Element {
		// Public Members
		T_key m_key;
		T_elmt m_value;
		Element() : m_key(0), m_value(0) {}
		Element(const T_key & key, const T_elmt & value) : m_key(key), m_value(value) {}
	};
	vector<Element> m_list;
	T_compare m_comparator;


public:
	// (de)Constructors
	/** Default destructor. */
	~PriorityList() {}
	/** Construct a priority list, optionally reserve a given capacity.
	 * @param	capacity	the amount to reserve */
	PriorityList(const unsigned int & capacity = 16) {
		m_list.reserve(capacity);
	}


	// Public Methods
	/** Get the size of the list.
	 * @return			the size of the list */
	const size_t size() const {
		return m_list.size();
	}
	/** Insert a new element into the lsit into a correct relative position.
	 * @param	key		the key used to sort this element
	 * @param	value	the value paired with the key to be inserted */
	void insert(const T_key & key, const T_elmt & value) {
		for (auto walk = m_list.begin(); walk != m_list.end(); ++walk) {
			if (m_comparator(key, (*walk).m_key)) {
				m_list.insert(walk, Element(key, value)); 
				return;
			}			
		}
		// List was empty
		m_list.push_back(Element(key, value));
	}
	/** Create a final list of elements only.
	 * @return			a sorted list of elements */
	const vector<T_elmt> toList() const {
		vector<T_elmt> outList;
		outList.reserve(m_list.size());
		for each (const auto &element in m_list)
			outList.push_back(element.m_value);
		return outList;
	}
};

#endif // PRIORITY_LIST_H