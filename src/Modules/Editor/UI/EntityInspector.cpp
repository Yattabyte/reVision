#include "Modules/Editor/UI/EntityInspector.h"
#include "Modules/Editor/Editor_M.h"
#include "imgui.h"

// Component Inspectors
#include "Modules/Editor/Systems/Inspector_Transform_System.h"
#include "Modules/Editor/Systems/Inspector_Prop_System.h"
#include "Modules/Editor/Systems/Inspector_Skeleton_System.h"
#include "Modules/Editor/Systems/Inspector_Collider_System.h"
#include "Modules/Editor/Systems/Inspector_Light_System.h"


EntityInspector::EntityInspector(Engine& engine, LevelEditor_Module& editor) noexcept :
	m_engine(engine),
	m_editor(editor)
{
	m_open = true;
	m_inspectorSystems.makeSystem<Inspector_Transform_System>(engine, editor);
	m_inspectorSystems.makeSystem<Inspector_Prop_System>(engine, editor);
	m_inspectorSystems.makeSystem<Inspector_Skeleton_System>(engine, editor);
	m_inspectorSystems.makeSystem<Inspector_Collider_System>(engine, editor);
	m_inspectorSystems.makeSystem<Inspector_Light_System>(engine, editor);
}

void EntityInspector::tick(const float& deltaTime) noexcept
{
	if (m_open) {
		const auto& selectedEntities = m_editor.getSelection();
		if (ImGui::Begin("Entity Inspector", &m_open, ImGuiWindowFlags_AlwaysAutoResize)) {
			// Render the selected component attributes that we have widgets for
			const auto text = std::string("Entities Selected: (" + std::to_string(selectedEntities.size()) + ")");
			ImGui::Text(text.c_str());
			m_editor.getWorld().updateSystems(m_inspectorSystems, deltaTime);
		}
		ImGui::End();
	}
}