#include "Managers\Geometry_Manager.h"
#include <algorithm>

static map<int, vector<Geometry*>> geometry_objects;
static shared_mutex system_mutex;

void Geometry_Manager::RegisterGeometry(const int &typeID, Geometry *geometry) {
	lock_guard<shared_mutex> write(system_mutex);

	// Ensure there is a vector for geometry for this category
	// Is safe, only inserts once
	// Add this geometry to that list
	geometry_objects.insert(pair<int, vector<Geometry*>>(typeID, vector<Geometry*>()));
	geometry_objects[typeID].push_back(geometry);
}

void Geometry_Manager::UnRegisterGeometry(const int &typeID, Geometry *geometry) {
	lock_guard<shared_mutex> write(system_mutex);

	// Check if the category even exists yet
	// Remove all instances of the provided geometry in that category
	if (geometry_objects.find(typeID) != geometry_objects.end()) {
		auto &objects = geometry_objects[typeID];
		objects.erase(std::remove_if(begin(objects), end(objects), [geometry](const auto *ref) {
			return (ref == geometry);
		}), end(objects));
	}
}

shared_mutex & Geometry_Manager::GetDataLock()
{
	return system_mutex;
}

map<int, vector<Geometry*>>& Geometry_Manager::GetAllGeometry()
{
	return geometry_objects;
}
