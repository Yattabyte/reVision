#include "Systems\ECS\EntityFactory.h"
#include "Systems\ECS\ECSMessage.h"
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

ECSHandle EntityFactory::CreateEntity(char * type)
{
	level_entities.insert(pair<char*, vector<Entity*>>(type, vector<Entity*>()));
	Entity *entity;
	unsigned int spot;	

	if ((free_spots.find(type) != free_spots.end()) && free_spots[type].size()) {
		spot = free_spots[type].front();
		level_entities[type][spot] = nullptr;
		free_spots[type].pop_front();
	}
	else {
		spot = level_entities[type].size();
		level_entities[type].push_back(nullptr);
	}

	entity = creator_map[type]->Create(ECSHandle(type, spot));
	level_entities[type][spot] = entity;
	return ECSHandle(type, spot);
}

void EntityFactory::DeleteEntity(const ECSHandle& id)
{
	if (level_entities.find(id.first) == level_entities.end())
		return;
	if (level_entities[id.first].size() <= id.second)
		return;
	creator_map[id.first]->Destroy(level_entities.at(id.first).at(id.second));
	free_spots.insert(pair<char*, deque<unsigned int>>(id.first, deque<unsigned int>()));
	free_spots[id.first].push_back(id.second);
}

Entity * EntityFactory::GetEntity(const ECSHandle& id)
{
	if (level_entities.find(id.first) == level_entities.end())
		return nullptr;
	return level_entities[id.first][id.second];
}

vector<Entity*> &EntityFactory::GetEntitiesByType(char * type)
{
	if (level_entities.find(type) == level_entities.end())
		return vector<Entity*>();
	return level_entities[type];
}

void EntityFactory::SendMessageToEntity(ECSMessage * message, const ECSHandle & target)
{
	if (target.first && level_entities.find(target.first) != level_entities.end()) {

	}
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
