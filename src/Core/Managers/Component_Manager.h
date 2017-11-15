/*
	Component_Manager

	- TO DO
*/

#pragma once
#ifndef COMPONENT_MANAGER
#define COMPONENT_MANAGER
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include <vector>

using namespace std;

class Component;

namespace Component_Manager {
	unsigned int RegisterComponent(const unsigned int &typeID, Component *component);
	void DeRegisterComponent(const vector<pair<unsigned int, unsigned int>> &handles);
	void DeRegisterComponent(const unsigned int &typeID, const unsigned int &spot);
	void DeRegisterComponent(const unsigned int &typeID, Component *component);

	void *GetComponent(const pair<unsigned int, unsigned int> &handle);
}

#endif // COMPONENT_MANAGER