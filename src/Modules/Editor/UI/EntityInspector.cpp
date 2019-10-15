#include "Modules/Editor/UI/EntityInspector.h"
#include "Modules/Editor/Editor_M.h"
#include "imgui.h"
#include "Engine.h"

// Component Inspectors
#include "Modules/Editor/Systems/Inspector_Transform_System.h"
#include "Modules/Editor/Systems/Inspector_Prop_System.h"
#include "Modules/Editor/Systems/Inspector_Light_System.h"
#include "Modules/Editor/Systems/Inspector_LightColor_System.h"
#include "Modules/Editor/Systems/Inspector_LightRadius_System.h"
#include "Modules/Editor/Systems/Inspector_LightCutoff_System.h"


EntityInspector::EntityInspector(Engine* engine, LevelEditor_Module* editor)
	: m_engine(engine), m_editor(editor)
{
	m_open = true;
	m_inspectorSystems.makeSystem<Inspector_Transform_System>(engine, editor);
	m_inspectorSystems.makeSystem<Inspector_Prop_System>(engine, editor);
	m_inspectorSystems.makeSystem<Inspector_Light_System>(engine, editor);
	m_inspectorSystems.makeSystem<Inspector_LightColor_System>(engine, editor);
	m_inspectorSystems.makeSystem<Inspector_LightRadius_System>(engine, editor);
	m_inspectorSystems.makeSystem<Inspector_LightCutoff_System>(engine, editor);
}

void EntityInspector::tick(const float& deltaTime)
{
	if (m_open) {
		const auto& selectedEntities = m_editor->getSelection();
		ImGui::SetNextWindowDockID(ImGui::GetID("RightDock"), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Entity Inspector", &m_open, ImGuiWindowFlags_AlwaysAutoResize)) {
			// Render the selected component attributes that we have widgets for
			const auto text = std::string("Entities Selected: (" + std::to_string(selectedEntities.size()) + ")");
			ImGui::Text(text.c_str());
			m_editor->getActiveWorld().updateSystems(m_inspectorSystems, deltaTime);
		}
		ImGui::End();
	}
}