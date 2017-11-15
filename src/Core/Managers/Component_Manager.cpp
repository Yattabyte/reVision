#include "Managers\Component_Manager.h"
#include "Entities\Components\Component.h"
#include <algorithm>
#include <map>
#include <vector>

using namespace std;

static map<unsigned int, vector<Component*>> object_components;

unsigned int Component_Manager::RegisterComponent(const unsigned int & typeID, Component * component)
{
	object_components.insert(pair<unsigned int, vector<Component*>>(typeID, vector<Component*>()));
	object_components[typeID].push_back(component);
	return object_components[typeID].size() - 1;
}

void Component_Manager::DeRegisterComponent(const vector<pair<unsigned int, unsigned int>> &handles)
{
	for each (auto handle in handles) {
		if (object_components.find(handle.first) != object_components.end()) {
			auto &vec = object_components[handle.first];
			vec.erase(vec.begin() + handle.second);
		}
	}
}

void Component_Manager::DeRegisterComponent(const unsigned int & typeID, const unsigned int & spot)
{
	if (object_components.find(typeID) != object_components.end()) {
		auto &vec = object_components[typeID];
		vec.erase(vec.begin() + spot);
	}
}

void Component_Manager::DeRegisterComponent(const unsigned int & typeID, Component * component)
{
	if (object_components.find(typeID) != object_components.end()) {
		auto &vec = object_components[typeID];
		vec.erase(std::remove_if(begin(vec), end(vec), [component](const auto *ref) {
			return (component == ref);
		}), end(vec));
	}
}

void * Component_Manager::GetComponent(const pair<unsigned int, unsigned int>& handle)
{
	return object_components[handle.first][handle.second];
}
