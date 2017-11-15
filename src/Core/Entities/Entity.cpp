#include "Entities\Entity.h"
#include "Entities\Components\Component.h"
#include "Systems\Factories\ComponentFactory.h"
//#include "Systems\Component_Manager.h"

Component* Entity::addComponent(char *type)
{
	m_component_handles.insert(std::pair<char*, vector<unsigned int>>(type, vector<unsigned int>()));
	m_component_handles[type].push_back(ComponentFactory::CreateComponent(type));
	return ComponentFactory::GetComponent(type, m_component_handles[type].back());
}

Entity::~Entity()
{
	for each (auto pair in m_component_handles)
		for each (auto id in pair.second) 
			ComponentFactory::DeleteComponent(pair.first, id);			
}
