#include "Modules/Editor/Gizmos/Mouse.h"
#include "Modules/Editor/Systems/MousePicker_System.h"
#include "Engine.h"
#include "imgui.h"


Mouse_Gizmo::~Mouse_Gizmo() noexcept
{
	// Update indicator
	*m_aliveIndicator = false;
}

Mouse_Gizmo::Mouse_Gizmo(Engine& engine, LevelEditor_Module& editor) noexcept :
	m_engine(engine),
	m_editor(editor),
	m_pickerSystem(std::make_shared<MousePicker_System>(engine)),
	m_translationGizmo(engine, editor),
	m_scalingGizmo(engine, editor),
	m_rotationGizmo(engine, editor),
	m_spawnModel(Shared_Auto_Model(engine, "Editor\\spawn")),
	m_spawnShader(Shared_Shader(engine, "Editor\\spawnShader"))
{
	// Update indicator
	*m_aliveIndicator = true;

	// Asset-Finished Callbacks
	m_spawnModel->addCallback(m_aliveIndicator, [&]() mutable {
		m_spawnIndirect = IndirectDraw<1>((GLuint)m_spawnModel->getSize(), 1, 0, GL_CLIENT_STORAGE_BIT);
		});
}

void Mouse_Gizmo::frameTick(const float& deltaTime) noexcept
{
	checkInput(deltaTime);
	render(deltaTime);
}

bool Mouse_Gizmo::checkInput(const float& deltaTime) noexcept
{
	if (!ImGui::GetIO().WantCaptureMouse) {
		if (ImGui::IsKeyPressed('t') || ImGui::IsKeyPressed('T'))
			m_inputMode = 0;
		else if (ImGui::IsKeyPressed('r') || ImGui::IsKeyPressed('R'))
			m_inputMode = 1;
		else if (ImGui::IsKeyPressed('e') || ImGui::IsKeyPressed('E'))
			m_inputMode = 2;

		if (m_inputMode == 0 && m_translationGizmo.checkMouseInput(deltaTime))
			return true;
		else if (m_inputMode == 1 && m_rotationGizmo.checkMouseInput(deltaTime))
			return true;
		else if (m_inputMode == 2 && m_scalingGizmo.checkMouseInput(deltaTime))
			return true;

		// Set selection LAST, allow attempts at other gizmo's first
		if (ImGui::IsMouseClicked(0)) {
			m_editor.getWorld().updateSystem(m_pickerSystem.get(), deltaTime);
			const auto& [entityHandle, selectionTransform, intersectionTransform] = (std::dynamic_pointer_cast<MousePicker_System>(m_pickerSystem))->getSelection();

			// Set selection to all tools that need it
			if (ImGui::GetIO().KeyCtrl)
				m_editor.toggleAddToSelection(entityHandle);
			else
				if (!entityHandle.isValid()) {
					m_editor.setSelection({});
					setTransform(selectionTransform);
				}
				else
					m_editor.setSelection({ entityHandle });
			return m_editor.getSelection().size();
		}
		else if (ImGui::IsMouseClicked(2)) {
			m_editor.getWorld().updateSystem(m_pickerSystem.get(), deltaTime);
			const auto& [entityHandle, selectionTransform, intersectionTransform] = (std::dynamic_pointer_cast<MousePicker_System>(m_pickerSystem))->getSelection();
			m_spawnTransform = intersectionTransform;
			return true;
		}
	}
	return false;
}

void Mouse_Gizmo::render(const float& deltaTime) noexcept
{
	if (m_inputMode == 0)
		m_translationGizmo.render(deltaTime);
	else if (m_inputMode == 1)
		m_rotationGizmo.render(deltaTime);
	else if (m_inputMode == 2)
		m_scalingGizmo.render(deltaTime);

	if (Asset::All_Ready(m_spawnModel, m_spawnShader)) {
		// Get camera matrices
		const auto& clientCamera = m_engine.getModule_Graphics().getClientCamera();
		const auto pMatrix = clientCamera->pMatrix;
		const auto vMatrix = clientCamera->vMatrix;
		const auto trans = m_spawnTransform.m_modelMatrix;
		const auto mScale = glm::scale(glm::mat4(1.0f), glm::vec3(glm::distance(m_spawnTransform.m_position, clientCamera->EyePosition) * 0.02f));

		// Render Gizmo Model
		m_spawnModel->bind();
		m_spawnShader->bind();
		m_spawnShader->setUniform(0, pMatrix * vMatrix * trans * mScale);
		m_spawnShader->setUniform(4, glm::vec3(0.2, 1, 0.2));
		m_spawnIndirect.drawCall();
		Shader::Release();
	}
}

void Mouse_Gizmo::setTransform(const Transform& transform) noexcept
{
	m_selectionTransform = transform;
	m_translationGizmo.setTransform(transform);
	m_rotationGizmo.setTransform(transform);
	m_scalingGizmo.setTransform(transform);
}

Transform Mouse_Gizmo::getSelectionTransform() const noexcept
{
	return m_selectionTransform;
}

Transform Mouse_Gizmo::getSpawnTransform() const noexcept
{
	return m_spawnTransform;
}

void Mouse_Gizmo::setSelection(const std::vector<EntityHandle>& entityHandles) noexcept
{
	m_selection = entityHandles;
}

std::vector<EntityHandle>& Mouse_Gizmo::getSelection() noexcept
{
	return m_selection;
}