#include "Modules/Editor/UI/Settings.h"
#include "Modules/Editor/Editor_M.h"
#include "Engine.h"
#include "imgui.h"


Settings::Settings(Engine& engine, LevelEditor_Module& editor) noexcept :
	m_engine(engine),
	m_editor(editor)
{
	m_open = true;
}

void Settings::tick(const float& /*deltaTime*/)
{
	tickMainDialogue();
}

void Settings::tickMainDialogue()
{
	if (m_open) {
		if (ImGui::Begin("Preferences", &m_open, ImGuiWindowFlags_AlwaysAutoResize)) {
			// Header
			ImGui::Text("Configurable level editor settings:");
			ImGui::Spacing();

			// Editor Settings
			if (ImGui::CollapsingHeader("Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
				static float autoSaveInterval = 60.0F;
				m_engine.getPreferenceState().getOrSetValue(PreferenceState::Preference::E_AUTOSAVE_INTERVAL, autoSaveInterval);
				autoSaveInterval /= 60.0F;
				if (ImGui::DragFloat("Autosave Time", &autoSaveInterval, 0.5F, 0.0F, 0.0F, "%.0f Minutes")) {
					autoSaveInterval = std::max(1.0F, autoSaveInterval);
					m_engine.getPreferenceState().setValue(PreferenceState::Preference::E_AUTOSAVE_INTERVAL, float(autoSaveInterval * 60.0F));
				}
				if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					const auto description = "Backup the current level every " + std::to_string(autoSaveInterval) + " minutes.";
					ImGui::Text(description.c_str());
					ImGui::EndTooltip();
				}

				static float floatStackSize = 500.0F;
				m_engine.getPreferenceState().getOrSetValue(PreferenceState::Preference::E_UNDO_STACKSIZE, floatStackSize);
				static int intStackSize = int(floatStackSize);
				if (ImGui::DragInt("Max Undo/Redo", &intStackSize, 1.0F, 0, 1000, "%d Actions")) {
					intStackSize = std::max(1, intStackSize);
					m_engine.getPreferenceState().setValue(PreferenceState::Preference::E_UNDO_STACKSIZE, float(intStackSize));
				}
				if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					const auto description = "Save a list of the previous " + std::to_string(intStackSize) + " undo-able actions.";
					ImGui::Text(description.c_str());
					ImGui::EndTooltip();
				}

				static float outlineScale = 0.05F;
				m_engine.getPreferenceState().getOrSetValue(PreferenceState::Preference::E_OUTLINE_SCALE, outlineScale);
				outlineScale *= 1000.0F;
				if (ImGui::DragFloat("Outline Size", &outlineScale, 1, 0, 100, "%.0f%%")) {
					outlineScale = std::clamp(outlineScale, 0.0F, 100.0F);
					m_engine.getPreferenceState().setValue(PreferenceState::Preference::E_OUTLINE_SCALE, outlineScale / 1000.0F);
				}
				if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					const auto description = "Set the outline size of selected objects to " + std::to_string(outlineScale) + ".";
					ImGui::Text(description.c_str());
					ImGui::EndTooltip();
				}

				static float gizmoScale = 0.02F;
				m_engine.getPreferenceState().getOrSetValue(PreferenceState::Preference::E_GIZMO_SCALE, gizmoScale);
				gizmoScale *= 1000.0F;
				if (ImGui::DragFloat("Gizmo Size", &gizmoScale, 1, 0, 100, "%.0f%%")) {
					gizmoScale = std::clamp(gizmoScale, 0.0F, 100.0F);
					m_engine.getPreferenceState().setValue(PreferenceState::Preference::E_GIZMO_SCALE, gizmoScale / 1000.0F);
				}
				if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					const auto description = "Set the screen-size scale to " + std::to_string(gizmoScale) + " for level editor gizmos.";
					ImGui::Text(description.c_str());
					ImGui::EndTooltip();
				}

				static float gridSnap = 1.0F;
				m_engine.getPreferenceState().getOrSetValue(PreferenceState::Preference::E_GRID_SNAP, gridSnap);
				if (ImGui::DragFloat("Grid Snap", &gridSnap, 1, 0, 1000.0F, "%.3f Units")) {
					gridSnap = std::clamp(gridSnap, 0.0F, 1000.0F);
					m_engine.getPreferenceState().setValue(PreferenceState::Preference::E_GRID_SNAP, gridSnap);
				}
				if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					const auto description = "Snap object translation/scaling to a grid of " + std::to_string(gridSnap) + " units.\r\nNot retroactive.";
					ImGui::Text(description.c_str());
					ImGui::EndTooltip();
				}

				static float angleSnapping = 1.0F;
				m_engine.getPreferenceState().getOrSetValue(PreferenceState::Preference::E_ANGLE_SNAP, angleSnapping);
				if (ImGui::DragFloat("Angle Snap", &angleSnapping, 1, 0, 360.0F, "%.3f Degrees")) {
					angleSnapping = std::clamp(angleSnapping, 0.0F, 360.0F);
					m_engine.getPreferenceState().setValue(PreferenceState::Preference::E_ANGLE_SNAP, angleSnapping);
				}
				if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					const auto description = "Snap object rotations every " + std::to_string(angleSnapping) + " degrees.\r\nNot retroactive.";
					ImGui::Text(description.c_str());
					ImGui::EndTooltip();
				}
			}
		}
		ImGui::End();
	}
}