#include "Modules/Editor/Systems/ClearSelection_System.h"
#include "Modules/Editor/Editor_M.h"
#include "Modules/ECS/component_types.h"


ClearSelection_System::ClearSelection_System(Engine& engine, LevelEditor_Module& editor) :
	m_engine(engine),
	m_editor(editor)
{
	// Declare component types used
	addComponentType(Selected_Component::Runtime_ID);
}

void ClearSelection_System::updateComponents(const float&, const std::vector<std::vector<ecsBaseComponent*>>& components) 
{
	auto& ecsWorld = m_editor.getWorld();
	for (const auto& componentParam : components)
		ecsWorld.removeEntityComponent((static_cast<Selected_Component*>(componentParam[0]))->m_entity, Selected_Component::Runtime_ID);
}