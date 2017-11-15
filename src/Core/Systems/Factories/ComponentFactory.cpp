#include "Systems\Factories\ComponentFactory.h"
#include "Entities\Components\Anim_Model_Component.h"
#include <deque>
#include <map>


struct cmp_str { bool operator()(const char *a, const char *b) const { return std::strcmp(a, b) < 0; } };
static map<char*, vector<Component*>, cmp_str> level_components;
static map<char*, deque<unsigned int>> free_spots;
static map<char*, ComponentCreator*, cmp_str> creator_map;

void ComponentFactory::Startup()
{
	creator_map.insert(pair<char*, ComponentCreator*>("Anim_Model", new Anim_Model_Creator()));
}

unsigned int ComponentFactory::CreateComponent(char * type)
{
	Component *component = creator_map[type]->Create();

	level_components.insert(pair<char*, vector<Component*>>(type, vector<Component*>()));
	if ((free_spots.find(type) != free_spots.end()) && free_spots[type].size()) {
		const unsigned int spot = free_spots[type].front();
		level_components[type][spot] = component;
		free_spots[type].pop_front();
		return spot;
	}
	else {
		level_components[type].push_back(component);
		return level_components[type].size() - 1;
	}
}

void ComponentFactory::DeleteComponent(char * type, const unsigned int & id)
{
	if (level_components.find(type) == level_components.end())
		return;
	if (level_components[type].size() <= id)
		return;
	creator_map[type]->Destroy(level_components.at(type).at(id));
	free_spots.insert(pair<char*, deque<unsigned int>>(type, deque<unsigned int>()));
	free_spots[type].push_back(id);
}


Component * ComponentFactory::GetComponent(char * type, const unsigned int & id)
{
	if (level_components.find(type) == level_components.end())
		return nullptr;
	return level_components[type][id];
}

vector<Component*> &ComponentFactory::GetComponentsByType(char * type)
{
	if (level_components.find(type) == level_components.end())
		return vector<Component*>();
	return level_components[type];
}

void ComponentFactory::Flush()
{
	for each (auto pair in level_components) {
		for each (auto *component in pair.second) {
			creator_map[pair.first]->Destroy(component);
		}
	}
	level_components.clear();
	free_spots.clear();
}