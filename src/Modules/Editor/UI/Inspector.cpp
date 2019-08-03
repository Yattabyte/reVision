#include "Modules/Editor/UI/Inspector.h"
#include "Modules/Editor/Editor_M.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "Engine.h"

// Component Inspectors
#include "Modules/Editor/Systems/Inspector_Transform_System.h"
#include "Modules/Editor/Systems/Inspector_Prop_System.h"
#include "Modules/Editor/Systems/Inspector_LightColor_System.h"
#include "Modules/Editor/Systems/Inspector_LightRadius_System.h"
#include "Modules/Editor/Systems/Inspector_LightCutoff_System.h"


Inspector::Inspector(Engine* engine, LevelEditor_Module* editor)
	: m_engine(engine), m_editor(editor)
{
	m_inspectorSystems.addSystem(new Inspector_Transform_System(editor));
	m_inspectorSystems.addSystem(new Inspector_Prop_System(editor));
	m_inspectorSystems.addSystem(new Inspector_LightColor_System(editor));
	m_inspectorSystems.addSystem(new Inspector_LightRadius_System(editor));
	m_inspectorSystems.addSystem(new Inspector_LightCutoff_System(editor));
}

void Inspector::tick(const float& deltaTime)
{
	auto& world = m_engine->getModule_World();
	const auto& worldEntities = world.getEntities();
	auto& entityNames = world.getEntityNames();
	const auto& selectedEntities = m_editor->getSelection();
	ImGui::SetNextWindowDockID(ImGui::GetID("RightDock"), ImGuiCond_FirstUseEver);	
	if (ImGui::Begin("Scene Inspector", NULL)) {			
		static ImGuiTextFilter filter;
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
		filter.Draw("Search");
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));

		size_t entityCount(0ull), displayCount(0ull);
		for each (const auto & entity in worldEntities) {
			bool entity_or_components_pass_filter = false;
			auto& entityName = entityNames.at(entityCount);
			const auto& components = world.handleToEntity(entity);
			entity_or_components_pass_filter += filter.PassFilter(entityName.c_str());
			for each (const auto & component in components)
				entity_or_components_pass_filter += filter.PassFilter(BaseECSComponent::findName(component.first));	

			// Check if the entity or its components matched search criteria
			if (entity_or_components_pass_filter) {
				ImGui::PushID(entity);
				ImGui::AlignTextToFramePadding();
				ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
				if (std::find(selectedEntities.cbegin(), selectedEntities.cend(), entity) != selectedEntities.cend())
					node_flags |= ImGuiTreeNodeFlags_Selected;

				auto tryLeftClickElement = [&]() {
					if (ImGui::IsItemClicked())
						if (ImGui::GetIO().KeyCtrl)
							m_editor->toggleAddToSelection(entity);
						else
							m_editor->setSelection({ entity });
				};
				auto tryRightClickElement = [&]() {
					if (ImGui::BeginPopupContextItem("Entity Controls")) {
						// Force selection to single item so operations will occur on just IT
						m_editor->setSelection({ entity });
						char entityNameChars[256];
						for (int x = 0; x < entityName.size() && x < 256; ++x)
							entityNameChars[x] = entityName[x];
						entityNameChars[entityName.size()] = '\0';
						if (ImGui::InputText("Rename", entityNameChars, 256, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue)) {
							entityName = entityNameChars;
							ImGui::CloseCurrentPopup();
						}
						ImGui::Separator();
						if (ImGui::MenuItem("Cut", "CTRL+X")) { m_editor->cutSelection(); }
						if (ImGui::MenuItem("Copy", "CTRL+C")) { m_editor->copySelection(); }
						if (ImGui::MenuItem("Paste", "CTRL+V")) { m_editor->paste(); }
						ImGui::Separator();
						if (ImGui::MenuItem("Delete", "DEL")) { m_editor->deleteSelection(); }
						ImGui::EndPopup();
					}
				};

				// Check if the entity is expanded
				if (ImGui::TreeNodeEx(entity, node_flags, "%s", entityName.c_str())) {
					tryLeftClickElement();
					tryRightClickElement();
					for (int x = 0; x < components.size(); ++x) {
						const auto& component = components[x];
						ImGui::PushID(&component);
						ImGui::AlignTextToFramePadding();
						ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0, 0.6f, 0.6f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0, 0.7f, 0.7f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0, 0.8f, 0.8f));
						const auto buttonPressed = ImGui::Button("-");
						ImGui::PopStyleColor(3);
						ImGui::SameLine();
						ImGui::Text(BaseECSComponent::findName(component.first));
						ImGui::PopID();
						if (buttonPressed)
							m_editor->deleteComponent(entity, component.first);
					}
					ImGui::AlignTextToFramePadding();
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2.0f / 7.0f, 0.6f, 0.6f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(2.0f / 7.0f, 0.7f, 0.7f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(2.0f / 7.0f, 0.8f, 0.8f));
					ImGui::Button("Add New Component");
					ImGui::PopStyleColor(3);
					if (ImGui::BeginPopupContextItem("Add New Component", 0)) {
						ImGui::Text("Choose a new component type");
						ImGui::Separator();
						ImGui::Spacing();
						constexpr const char* items[] = {
							Transform_Component::NAME(),
							PlayerSpawn_Component::NAME(),
							Player3D_Component::NAME(),
							Renderable_Component::NAME(),
							Camera_Component::NAME(),
							CameraArray_Component::NAME(),
							BoundingSphere_Component::NAME(),
							Prop_Component::NAME(),
							Skeleton_Component::NAME(),
							Shadow_Component::NAME(),
							LightColor_Component::NAME(),
							LightRadius_Component::NAME(),
							LightCutoff_Component::NAME(),
							LightDirectional_Component::NAME(),
							LightPoint_Component::NAME(),
							LightSpot_Component::NAME(),
							Reflector_Component::NAME(),
							Collider_Component::NAME()
						};
						static int item_current = 0;
						ImGui::Combo("", &item_current, items, IM_ARRAYSIZE(items));
						ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2.0f / 7.0f, 0.6f, 0.6f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(2.0f / 7.0f, 0.7f, 0.7f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(2.0f / 7.0f, 0.8f, 0.8f));
						ImGui::Spacing();
						ImGui::Spacing();
						bool isOk = ImGui::Button("Add Type"); ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();
						ImGui::PopStyleColor(3);
						if (ImGui::Button("Cancel"))
							ImGui::CloseCurrentPopup();
						if (isOk) {
							m_editor->addComponent(entity, items[item_current]);
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}
					ImGui::TreePop();

				}

				ImGui::SameLine();
				tryLeftClickElement();
				tryRightClickElement();
				ImGui::NewLine();
				ImGui::PopID();
				displayCount++;
			}
			entityCount++;
		}
		if (displayCount == 0ull) {
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::TextWrapped("No entities/components found!\nTry different search criteria...");
			ImGui::Spacing();
			ImGui::Separator();
		}
		ImGui::PopStyleVar();
	}
	ImGui::End();
	ImGui::SetNextWindowDockID(ImGui::GetID("RightDock"), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Entity Inspector", NULL)) {
		// Render the selected component attributes that we have widgets for
		const auto text = std::string("Entities Selected: (" + std::to_string(selectedEntities.size()) + ")");
		ImGui::Text(text.c_str());
		world.updateSystems(m_inspectorSystems, deltaTime);		
	}
	ImGui::End();
}