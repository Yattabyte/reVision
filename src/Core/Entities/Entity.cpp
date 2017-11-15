#include "Entities\Entity.h"
#include "Entities\Components\Component.h"
#include "Managers\Component_Manager.h"

Entity::~Entity()
{
	Component_Manager::DeRegisterComponent(m_component_handles);
}

void Entity::addComponent(Component * newComponent)
{
	const int category = newComponent->GetTypeID();
	const int spot = Component_Manager::RegisterComponent(category, newComponent);
	m_component_handles.push_back(std::pair<unsigned int, unsigned int>(category, spot));
}
