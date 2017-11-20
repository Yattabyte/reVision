#include "Systems\ECS\ComponentFactory.h"
#include "Entities\Components\Anim_Model_Component.h"
#include "Entities\Components\Light_Directional_Component.h"
#include "Systems\ECS\ECSmessage.h"
#include <deque>

static map<char*, vector<Component*>, cmp_str> level_components;
static map<char*, deque<unsigned int>> free_spots;
static map<char*, ComponentCreator*, cmp_str> creator_map;
static shared_mutex data_lock;

void ComponentFactory::Startup()
{
	creator_map.insert(pair<char*, ComponentCreator*>("Anim_Model", new Anim_Model_Creator()));
	creator_map.insert(pair<char*, ComponentCreator*>("Light_Directional", new Light_Directional_Creator()));
}

ECSHandle ComponentFactory::CreateComponent(char * type, const const ECSHandle &parent_ID)
{
	unique_lock<shared_mutex> write_lock(data_lock);

	level_components.insert(pair<char*, vector<Component*>>(type, vector<Component*>()));
	Component *component;
	unsigned int spot;

	if ((free_spots.find(type) != free_spots.end()) && free_spots[type].size()) {
		spot = free_spots[type].front();
		level_components[type][spot] = nullptr;
		free_spots[type].pop_front();
	}
	else {
		spot = level_components[type].size();
		level_components[type].push_back(nullptr);
	}
	
	component = creator_map[type]->Create(ECSHandle(type, spot), parent_ID);
	level_components[type][spot] = component;
	return ECSHandle(type, spot);
}

void ComponentFactory::DeleteComponent(const ECSHandle& id)
{
	unique_lock<shared_mutex> write_lock(data_lock);

	if (level_components.find(id.first) == level_components.end())
		return;
	if (level_components[id.first].size() <= id.second)
		return;
	creator_map[id.first]->Destroy(level_components.at(id.first).at(id.second));
	free_spots.insert(pair<char*, deque<unsigned int>>(id.first, deque<unsigned int>()));
	free_spots[id.first].push_back(id.second);
}


Component * ComponentFactory::GetComponent(const ECSHandle& id)
{
	shared_lock<shared_mutex> read_lock(data_lock);

	if (level_components.find(id.first) == level_components.end())
		return nullptr;
	return level_components[id.first][id.second];
}

vector<Component*> &ComponentFactory::GetComponentsByType(char * type)
{
	shared_lock<shared_mutex> read_lock(data_lock);

	if (level_components.find(type) == level_components.end())
		return vector<Component*>();
	return level_components[type];
}

void ComponentFactory::SendMessageToComponents(ECSmessage * message, const std::map<char*, std::vector<unsigned int>, cmp_str>& handles)
{
	shared_lock<shared_mutex> read_lock(data_lock);

	for each (const auto &pair in handles)
		for each (const auto &id in pair.second)
			level_components[pair.first][id]->ReceiveMessage(message);
}

void ComponentFactory::Flush()
{
	unique_lock<shared_mutex> write_lock(data_lock);

	for each (auto pair in level_components) {
		for each (auto *component in pair.second) {
			creator_map[pair.first]->Destroy(component);
		}
	}
	level_components.clear();
	free_spots.clear();
}

shared_mutex & ComponentFactory::GetDataLock()
{
	return data_lock;
}
