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
#include <map>
#include <vector>

using namespace std;

struct cmp_str { bool operator()(const char *a, const char *b) const { return std::strcmp(a, b) < 0; } };
class ECSMessage;
namespace ComponentFactory {
	DELTA_CORE_API void Startup();
	DELTA_CORE_API ECSHandle CreateComponent(char *type, const ECSHandle &parent_ID);
	DELTA_CORE_API void DeleteComponent(const ECSHandle& id);
	DELTA_CORE_API Component * GetComponent(const ECSHandle& id);
	DELTA_CORE_API vector<Component*> &GetComponentsByType(char *type);
	DELTA_CORE_API void SendMessageToComponents(ECSMessage *message, const std::map<char *, std::vector<unsigned int>, cmp_str> &targets);
	DELTA_CORE_API void Flush();
}

#endif // COMPONENTFACTORY