/*
	Component_Factory

	- Handles creation and storage for all entity components
*/

#pragma once
#ifndef COMPONENTFACTORY
#define COMPONENTFACTORY
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include "Entities\Components\Component.h"
#include <vector>

using namespace std;

namespace ComponentFactory {
	DELTA_CORE_API void Startup();
	DELTA_CORE_API unsigned int CreateComponent(char *type);
	DELTA_CORE_API void DeleteComponent(char * type, const unsigned int & id);
	DELTA_CORE_API Component * GetComponent(char *type, const unsigned int &id);
	DELTA_CORE_API vector<Component*> &GetComponentsByType(char *type);
	DELTA_CORE_API void Flush();
}

#endif // COMPONENTFACTORY