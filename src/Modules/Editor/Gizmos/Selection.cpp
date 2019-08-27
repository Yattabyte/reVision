#include "Modules/Editor/Gizmos/Selection.h"
#include "Modules/Editor/Gizmos/Translation.h"
#include "Modules/Editor/Gizmos/Scaling.h"
#include "Modules/Editor/Gizmos/Rotation.h"
#include "Modules/Editor/Systems/MousePicker_System.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "Engine.h"


Selection_Gizmo::~Selection_Gizmo()
{
	delete m_pickerSystem;
}

Selection_Gizmo::Selection_Gizmo(Engine* engine, LevelEditor_Module* editor)
	: m_engine(engine), m_editor(editor)
{
	// Create mouse picker system
	m_pickerSystem = new MousePicker_System(engine);

	m_translationGizmo = std::make_shared<Translation_Gizmo>(engine, editor);
	m_scalingGizmo = std::make_shared<Scaling_Gizmo>(engine, editor);
	m_rotationGizmo = std::make_shared<Rotation_Gizmo>(engine, editor);
}

void Selection_Gizmo::frameTick(const float& deltaTime)
{
	checkInput(deltaTime);
	render(deltaTime);
}

bool Selection_Gizmo::checkInput(const float& deltaTime)
{
	if (ImGui::IsKeyPressed('t') || ImGui::IsKeyPressed('T'))
		m_inputMode = 0;
	else if (ImGui::IsKeyPressed('r') || ImGui::IsKeyPressed('R'))
		m_inputMode = 1;
	else if (ImGui::IsKeyPressed('e') || ImGui::IsKeyPressed('E'))
		m_inputMode = 2;

	if (m_inputMode == 0 && m_translationGizmo->checkMouseInput(deltaTime))
		return true;
	else if (m_inputMode == 1 && m_rotationGizmo->checkMouseInput(deltaTime))
		return true;
	else if(m_inputMode == 2 && m_scalingGizmo->checkMouseInput(deltaTime))
		return true;

	// See if the mouse intersects any entities.
	// In any case move the selection gizmo to where the mouse is.
	if (!ImGui::GetIO().WantCaptureMouse && ImGui::IsMouseReleased(0) && !m_clicked) {
		m_clicked = true;
		return rayCastMouse(deltaTime);
	}

	m_clicked = false;
	return false;
}

void Selection_Gizmo::render(const float& deltaTime)
{

	if (m_inputMode == 0)
		m_translationGizmo->render(deltaTime);
	else if (m_inputMode == 1)
		m_rotationGizmo->render(deltaTime);
	else if (m_inputMode == 2)
		m_scalingGizmo->render(deltaTime);
}

void Selection_Gizmo::setTransform(const Transform& transform)
{
	m_transform = transform;
	m_translationGizmo->setTransform(transform);
	m_rotationGizmo->setTransform(transform);
	m_scalingGizmo->setTransform(transform);
}

Transform Selection_Gizmo::getTransform() const
{
	return m_transform;
}

void Selection_Gizmo::setSelection(const std::vector<ecsEntity*>& entities)
{
	m_selection = entities;

	// Find FIRST transform in the selection
	auto world = m_engine->getModule_World();
	for each (const auto entity in m_selection)
		if (entity)
			if (auto transform = world.getComponent<Transform_Component>(entity); transform != nullptr) {
				setTransform(transform->m_worldTransform);
				break;
			}
}

std::vector<ecsEntity*>& Selection_Gizmo::getSelection()
{
	return m_selection;
}

bool Selection_Gizmo::rayCastMouse(const float& deltaTime)
{
	m_engine->getModule_World().updateSystem(m_pickerSystem, deltaTime);
	const auto& [entity, transform] = ((MousePicker_System*)m_pickerSystem)->getSelection();

	// Set selection to all tools that need it	
	if (ImGui::GetIO().KeyCtrl)
		m_editor->toggleAddToSelection(entity);
	else
		if (entity == NULL_ENTITY_HANDLE) {
			m_editor->setSelection({});
			setTransform(transform);
		}
		else
			m_editor->setSelection({ entity });
	return m_editor->getSelection().size();
}