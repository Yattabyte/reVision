#include "Managers\World_Manager.h"
#include <algorithm>
#include <vector>

static vector<Entity*> level_entities;

void World_Manager::shutdown()
{
	UnloadWorld();
}

void World_Manager::addEntity(Entity * entity)
{
	level_entities.push_back(entity);
	entity->registerSelf();
}

void World_Manager::removeEntity(Entity * entity)
{
	// Using std algorithm, erase all occurences of entity in level_entities
	level_entities.erase(std::remove_if(begin(level_entities), end(level_entities), [entity](Entity *ent) {
		if (entity == ent)
			return true;
		return false;
	}), end(level_entities));
	entity->unregisterSelf();
	delete entity;
}

void World_Manager::LoadWorld()
{

}

void World_Manager::UnloadWorld()
{
	for each (auto *entity in level_entities) {
		entity->unregisterSelf();
		delete entity;
	}
	level_entities.clear();
}
