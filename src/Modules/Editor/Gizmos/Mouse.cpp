#include "Modules/Editor/Gizmos/Mouse.h"
#include "Modules/Editor/Gizmos/Translation.h"
#include "Modules/Editor/Gizmos/Scaling.h"
#include "Modules/Editor/Gizmos/Rotation.h"
#include "Modules/Editor/Systems/MousePicker_System.h"
#include "imgui.h"
#include "Engine.h"


Mouse_Gizmo::~Mouse_Gizmo()
{
	// Update indicator
	*m_aliveIndicator = false;
}

Mouse_Gizmo::Mouse_Gizmo(Engine* engine, LevelEditor_Module* editor)
	: m_engine(engine), m_editor(editor)
{
	// Update indicator
	*m_aliveIndicator = true;

	// Create mouse picker system
	m_pickerSystem = std::make_shared<MousePicker_System>(m_engine);

	// Create Sub-Gizmos
	m_translationGizmo = std::make_shared<Translation_Gizmo>(engine, editor);
	m_scalingGizmo = std::make_shared<Scaling_Gizmo>(engine, editor);
	m_rotationGizmo = std::make_shared<Rotation_Gizmo>(engine, editor);

	// Assets
	m_spawnModel = Shared_Auto_Model(engine, "Editor\\spawn");
	m_spawnShader = Shared_Shader(engine, "Editor\\spawnShader");
	m_spawnModel->addCallback(m_aliveIndicator, [&]() mutable {
		m_spawnIndirect = IndirectDraw((GLuint)m_spawnModel->getSize(), 1, 0, GL_CLIENT_STORAGE_BIT);
		});
}

void Mouse_Gizmo::frameTick(const float& deltaTime)
{
	checkInput(deltaTime);
	render(deltaTime);
}

bool Mouse_Gizmo::checkInput(const float& deltaTime)
{
	if (!ImGui::GetIO().WantCaptureMouse) {
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
		else if (m_inputMode == 2 && m_scalingGizmo->checkMouseInput(deltaTime))
			return true;

		// Set selection LAST, allow attempts at other gizmo's first
		if (ImGui::IsMouseClicked(0)) {
			m_editor->getActiveWorld().updateSystem(m_pickerSystem.get(), deltaTime);
			const auto& [entityHandle, selectionTransform, intersectionTransform] = (std::dynamic_pointer_cast<MousePicker_System>(m_pickerSystem))->getSelection();

			// Set selection to all tools that need it
			if (ImGui::GetIO().KeyCtrl)
				m_editor->toggleAddToSelection(entityHandle);
			else
				if (!entityHandle.isValid()) {
					m_editor->setSelection({});
					setTransform(selectionTransform);
				}
				else
					m_editor->setSelection({ entityHandle });
			return m_editor->getSelection().size();
		}
		else if (ImGui::IsMouseClicked(2)) {
			m_editor->getActiveWorld().updateSystem(m_pickerSystem.get(), deltaTime);
			const auto& [entityHandle, selectionTransform, intersectionTransform] = (std::dynamic_pointer_cast<MousePicker_System>(m_pickerSystem))->getSelection();
			m_spawnTransform = intersectionTransform;
			return true;
		}
	}
	return false;
}

void Mouse_Gizmo::render(const float& deltaTime)
{
	if (m_inputMode == 0)
		m_translationGizmo->render(deltaTime);
	else if (m_inputMode == 1)
		m_rotationGizmo->render(deltaTime);
	else if (m_inputMode == 2)
		m_scalingGizmo->render(deltaTime);

	if (m_spawnModel->existsYet() && m_spawnShader->existsYet()) {
		// Get camera matrices
		const auto pMatrix = m_engine->getModule_Graphics().getClientCamera()->get()->pMatrix;
		const auto vMatrix = m_engine->getModule_Graphics().getClientCamera()->get()->vMatrix;
		const auto trans = m_spawnTransform.m_modelMatrix;
		const auto mScale = glm::scale(glm::mat4(1.0f), glm::vec3(glm::distance(m_spawnTransform.m_position, m_engine->getModule_Graphics().getClientCamera()->get()->EyePosition) * 0.02f));

		// Render Gizmo Model
		m_spawnModel->bind();
		m_spawnShader->bind();
		m_spawnShader->setUniform(0, pMatrix * vMatrix * trans * mScale);
		m_spawnShader->setUniform(4, glm::vec3(0.2, 1, 0.2));
		m_spawnIndirect.drawCall();
	}
}

void Mouse_Gizmo::setTransform(const Transform& transform)
{
	m_selectionTransform = transform;
	m_translationGizmo->setTransform(transform);
	m_rotationGizmo->setTransform(transform);
	m_scalingGizmo->setTransform(transform);
}

Transform Mouse_Gizmo::getSelectionTransform() const
{
	return m_selectionTransform;
}

Transform Mouse_Gizmo::getSpawnTransform() const
{
	return m_spawnTransform;
}

void Mouse_Gizmo::setSelection(const std::vector<EntityHandle>& entityHandles)
{
	m_selection = entityHandles;
}

std::vector<EntityHandle>& Mouse_Gizmo::getSelection()
{
	return m_selection;
}