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
	m_open = true;
	m_inspectorSystems.makeSystem<Inspector_Transform_System>(engine, editor);
	m_inspectorSystems.makeSystem<Inspector_Prop_System>(engine, editor);
	m_inspectorSystems.makeSystem<Inspector_LightColor_System>(engine, editor);
	m_inspectorSystems.makeSystem<Inspector_LightRadius_System>(engine, editor);
	m_inspectorSystems.makeSystem<Inspector_LightCutoff_System>(engine, editor);
}

void Inspector::tick(const float& deltaTime)
{
	if (m_open) {
		auto& world = m_engine->getModule_World();
		const auto& selectedEntities = m_editor->getSelection();
		ImGui::SetNextWindowDockID(ImGui::GetID("RightDock"), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Scene Inspector", &m_open)) {
			static ImGuiTextFilter filter;
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
			filter.Draw("Search");
			ImGui::PopStyleVar();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));

			size_t displayCount(0ull);
			std::function<void(const ecsHandle&)> displayEntity = [&](const ecsHandle& entityHandle) {
				bool entity_or_components_pass_filter = false;
				if (auto * entity = world.getEntity(entityHandle)) {
					auto& entityName = entity->m_name;
					const auto& components = entity->m_components;
					entity_or_components_pass_filter += filter.PassFilter(entityName.c_str());
					for each (const auto & component in components)
						entity_or_components_pass_filter += filter.PassFilter(BaseECSComponent::findName(component.first));

					// Check if the entity or its components matched search criteria
					if (entity_or_components_pass_filter) {
						ImGui::PushID(entity);
						ImGui::AlignTextToFramePadding();
						ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
						if (std::find(selectedEntities.cbegin(), selectedEntities.cend(), entityHandle) != selectedEntities.cend())
							node_flags |= ImGuiTreeNodeFlags_Selected;

						auto tryLeftClickElement = [&]() {
							if (ImGui::IsItemClicked())
								if (ImGui::GetIO().KeyCtrl)
									m_editor->toggleAddToSelection(entityHandle);
								else
									m_editor->setSelection({ entityHandle });
						};
						auto tryRightClickElement = [&]() {
							if (ImGui::BeginPopupContextItem("Entity Controls")) {
								char entityNameChars[256];
								for (int x = 0; x < entityName.size() && x < 256; ++x)
									entityNameChars[x] = entityName[x];
								entityNameChars[entityName.size()] = '\0';
								if (ImGui::InputText("Rename", entityNameChars, 256, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue)) {
									entityName = entityNameChars;
									ImGui::CloseCurrentPopup();
								}
								ImGui::Separator();
								if (const auto selectionSize = selectedEntities.size()) {
									if (selectionSize >= 2ull) {
										const auto text = "Join to \"" + world.getEntity(selectedEntities[0])->m_name + "\"";
										if (ImGui::MenuItem(text.c_str())) { m_editor->mergeSelection(); }
										if (ImGui::MenuItem("Group Selection")) { m_editor->groupSelection(); }
										ImGui::Separator();
									}
									if (ImGui::MenuItem("Make Prefab", "CTRL+G", nullptr, selectedEntities.size())) { m_editor->makePrefab(); }
									for each (const auto & entityHandle in selectedEntities)
										if (world.getEntity(entityHandle)->m_children.size()) {
											if (ImGui::MenuItem("Ungroup")) { m_editor->ungroupSelection(); }
											ImGui::Separator();
											break;
										}
									if (ImGui::MenuItem("Cut")) { m_editor->cutSelection(); }
									if (ImGui::MenuItem("Copy")) { m_editor->copySelection(); }
									if (ImGui::MenuItem("Paste")) { m_editor->paste(); }
									ImGui::Separator();
									if (ImGui::MenuItem("Delete")) { m_editor->deleteSelection(); }
									ImGui::EndPopup();
								}
							}
						};
						auto tryDragElement = [&]() {
							if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
								ImGui::SetDragDropPayload("Entity", &entityHandle, sizeof(ecsHandle*));        // Set payload to carry the index of our item (could be anything)
								const auto text = "Move \"" + entityName + "\" into...";
								ImGui::Text(text.c_str());
								ImGui::EndDragDropSource();
							}
							if (ImGui::BeginDragDropTarget()) {
								if (const ImGuiPayload * payload = ImGui::AcceptDragDropPayload("Entity")) {
									IM_ASSERT(payload->DataSize == sizeof(ecsHandle*));
									m_editor->setSelection({ entityHandle, (*(ecsHandle*)(payload->Data)) });
									m_editor->mergeSelection();
									m_editor->setSelection({ entityHandle });
								}
								ImGui::EndDragDropTarget();
							}
						};

						// Check if the entity is expanded
						if (ImGui::TreeNodeEx(entity, node_flags, "%s", entityName.c_str())) {
							tryLeftClickElement();
							tryRightClickElement();
							tryDragElement();
							for each (const auto & subEntityHandle in world.getEntityHandles(entityHandle))
								displayEntity(subEntityHandle);
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
									m_editor->deleteComponent(entityHandle, component.first);
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
									Camera_Component::NAME(),
									CameraArray_Component::NAME(),
									BoundingSphere_Component::NAME(),
									BoundingBox_Component::NAME(),
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
									m_editor->addComponent(entityHandle, items[item_current]);
									ImGui::CloseCurrentPopup();
								}
								ImGui::EndPopup();
							}
							ImGui::TreePop();
						}

						ImGui::SameLine();
						tryLeftClickElement();
						tryRightClickElement();
						tryDragElement();
						ImGui::NewLine();
						ImGui::PopID();
						ImGui::Separator();
						displayCount++;
					}
				}
			};

			ImGui::Separator();
			for each (const auto & entityHandle in world.getEntityHandles())
				displayEntity(entityHandle);

			// Special case to allow dragging to end of scene list
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload * payload = ImGui::AcceptDragDropPayload("Entity")) {
					IM_ASSERT(payload->DataSize == sizeof(ecsHandle*));
					m_editor->setSelection({ ecsHandle(), (*(ecsHandle*)(payload->Data)) });
					m_editor->mergeSelection();
					m_editor->clearSelection();
				}
				ImGui::EndDragDropTarget();
			}

			// Display message when no filtered results
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
		if (ImGui::Begin("Entity Inspector", &m_open)) {
			// Render the selected component attributes that we have widgets for
			const auto text = std::string("Entities Selected: (" + std::to_string(selectedEntities.size()) + ")");
			ImGui::Text(text.c_str());
			world.updateSystems(m_inspectorSystems, deltaTime);
		}
		ImGui::End();
	}
}