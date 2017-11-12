#include "Managers\Lighting_Manager.h"
#include <algorithm>

static map<int, vector<Light*>> light_objects;
static shared_mutex system_mutex;

void Lighting_Manager::shutdown()
{
	light_objects.clear();
}

void Lighting_Manager::RegisterLight(const int &typeID, Light *light) {
	lock_guard<shared_mutex> write(system_mutex);

	// Ensure there is a vector for light for this category
	// Is safe, only inserts once
	// Add this light to that list
	light_objects.insert(pair<int, vector<Light*>>(typeID, vector<Light*>()));
	light_objects[typeID].push_back(light);
}

void Lighting_Manager::UnRegisterLight(const int &typeID, Light *light) {
	lock_guard<shared_mutex> write(system_mutex);

	// Check if the category even exists yet
	// Remove all instances of the provided light in that category
	if (light_objects.find(typeID) != light_objects.end()) {
		auto &objects = light_objects[typeID];
		objects.erase(std::remove_if(begin(objects), end(objects), [light](const auto *ref) {
			return (ref == light);
		}), end(objects));
	}
}

shared_mutex & Lighting_Manager::GetDataLock()
{
	return system_mutex;
}

map<int, vector<Light*>>& Lighting_Manager::GetAllLights()
{
	return light_objects;
}