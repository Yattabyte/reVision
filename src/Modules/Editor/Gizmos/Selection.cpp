#include "Modules/Editor/Gizmos/Selection.h"
#include "Modules/Editor/Gizmos/Translation.h"
#include "Modules/Editor/Gizmos/Scaling.h"
#include "Modules/Editor/Systems/MousePicker_System.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "Engine.h"


Selection_Gizmo::~Selection_Gizmo()
{
	// Update indicator
	*m_aliveIndicator = false;
	delete m_pickerSystem;
}

Selection_Gizmo::Selection_Gizmo(Engine* engine, LevelEditor_Module* editor)
	: m_engine(engine), m_editor(editor)
{
	// Update indicator
	*m_aliveIndicator = true;

	// Create mouse picker system
	m_pickerSystem = new MousePicker_System(engine);

	// Assets
	m_colorPalette = Shared_Texture(engine, "Editor\\colors.png");
	m_model = Shared_Auto_Model(engine, "Editor\\selind");
	m_gizmoShader = Shared_Shader(engine, "Editor\\gizmoShader");
	m_wireframeShader = Shared_Shader(engine, "Editor\\wireframe");

	// Asset-Finished Callbacks
	m_model->addCallback(m_aliveIndicator, [&]() mutable {
		const GLuint data[4] = { (GLuint)m_model->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
		m_indicatorIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, data, GL_CLIENT_STORAGE_BIT);
		});

	m_translationGizmo = std::make_shared<Translation_Gizmo>(engine, editor);
	m_scalingGizmo = std::make_shared<Scaling_Gizmo>(engine, editor);
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
	else if (ImGui::IsKeyPressed('e') || ImGui::IsKeyPressed('E'))
		m_inputMode = 2;

	if (m_inputMode == 0 && m_translationGizmo->checkMouseInput(deltaTime))
		return true;
	else if(m_inputMode == 2 && m_scalingGizmo->checkMouseInput(deltaTime))
		return true;

	// See if the mouse intersects any entities.
	// In any case move the selection gizmo to where the mouse is.
	if (!ImGui::GetIO().WantCaptureMouse && ImGui::IsMouseClicked(0) && !m_clicked) {
		m_clicked = true;
		return rayCastMouse(deltaTime);
	}

	m_clicked = false;
	return false;
}

void Selection_Gizmo::render(const float& deltaTime)
{
	// Safety check first
	if (m_model->existsYet() && m_colorPalette->existsYet() && m_gizmoShader->existsYet() && m_wireframeShader->existsYet() && m_editor->getSelection().size() == 0) {
		// Set up state
		m_editor->bindFBO();
		m_model->bind();
		m_colorPalette->bind(0);
		m_indicatorIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);

		// Generate matrices
		auto pMatrix = m_engine->getModule_Graphics().getClientCamera()->get()->pMatrix;
		auto vMatrix = m_engine->getModule_Graphics().getClientCamera()->get()->vMatrix;
		auto mMatrix = glm::translate(glm::mat4(1.0f), m_transform.m_position) * glm::scale(glm::mat4(1.0f), glm::vec3(glm::distance(m_transform.m_position, m_engine->getModule_Graphics().getClientCamera()->get()->EyePosition) * 0.033f));
		m_gizmoShader->setUniform(0, pMatrix * vMatrix * mMatrix);
		m_wireframeShader->setUniform(0, pMatrix * vMatrix * mMatrix);
		m_wireframeShader->setUniform(4, glm::vec3(1, 0, 0));

		// Render
		m_gizmoShader->bind();
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glFrontFace(GL_CW);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		m_wireframeShader->bind();
		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Revert State
		m_gizmoShader->Release();
	}

	if (m_inputMode == 0)
		m_translationGizmo->render(deltaTime);
	else if (m_inputMode == 2)
		m_scalingGizmo->render(deltaTime);
}

void Selection_Gizmo::setTransform(const Transform& transform)
{
	m_transform = transform;
	m_translationGizmo->setTransform(transform);
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