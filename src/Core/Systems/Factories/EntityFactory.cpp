#include "Systems\Factories\EntityFactory.h"
#include "Entities\Prop.h"
#include <deque>
#include <map>

static map<char*, vector<Entity*>, cmp_str> level_entities;
static map<char*, deque<unsigned int>> free_spots;
static map<char*, EntityCreator*, cmp_str> creator_map;

void EntityFactory::Startup()
{
	creator_map.insert(pair<char*, EntityCreator*>("Prop", new PropCreator()));
}

unsigned int EntityFactory::CreateEntity(char * type)
{
	Entity *entity = creator_map[type]->Create();

	level_entities.insert(pair<char*, vector<Entity*>>(type, vector<Entity*>()));
	if ((free_spots.find(type) != free_spots.end()) && free_spots[type].size()) {
		const unsigned int spot = free_spots[type].front();
		level_entities[type][spot] = entity;
		free_spots[type].pop_front();
		return spot;
	}
	else {
		level_entities[type].push_back(entity);
		return level_entities[type].size() - 1;
	}
}

void EntityFactory::DeleteEntity(char * type, const unsigned int & id)
{
	if (level_entities.find(type) == level_entities.end())
		return;
	if (level_entities[type].size() <= id)
		return;
	creator_map[type]->Destroy(level_entities.at(type).at(id));
	free_spots.insert(pair<char*, deque<unsigned int>>(type, deque<unsigned int>()));
	free_spots[type].push_back(id);
}

Entity * EntityFactory::GetEntity(char * type, const unsigned int & id)
{
	if (level_entities.find(type) == level_entities.end())
		return nullptr;
	return level_entities[type][id];
}

vector<Entity*> &EntityFactory::GetEntitiesByType(char * type)
{
	if (level_entities.find(type) == level_entities.end())
		return vector<Entity*>();
	return level_entities[type];
}

void EntityFactory::Flush()
{
	for each (auto pair in level_entities) {
		for each (auto *entity in pair.second) {
			creator_map[pair.first]->Destroy(entity);
		}
	}
	level_entities.clear();
	free_spots.clear();
}
