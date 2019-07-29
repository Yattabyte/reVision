#include "Modules/Editor/Gizmos/Selection.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "Engine.h"


Selection_Gizmo::~Selection_Gizmo()
{
	// Update indicator
	*m_aliveIndicator = false;
}

Selection_Gizmo::Selection_Gizmo(Engine * engine, LevelEditor_Module * editor)
	: m_engine(engine), m_editor(editor)
{
	// Update indicator
	*m_aliveIndicator = true;

	// Assets
	m_colorPalette = Shared_Texture(engine, "Editor\\colors.png");
	m_selIndicator = Shared_Auto_Model(engine, "Editor\\selind");
	m_gizmoShader = Shared_Shader(engine, "Editor\\gizmoShader");
	m_wireframeShader = Shared_Shader(engine, "Editor\\wireframe");

	// Asset-Finished Callbacks
	m_selIndicator->addCallback(m_aliveIndicator, [&]() mutable {
		const GLuint data[4] = { (GLuint)m_selIndicator->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
		m_indicatorIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, data, GL_CLIENT_STORAGE_BIT);
	});

	// Preferences
	auto & preferences = engine->getPreferenceState();
	preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
	preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) {
		m_renderSize.x = (int)f;
	});
	preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {
		m_renderSize.y = (int)f;
	});
}

void Selection_Gizmo::frameTick(const float & deltaTime)
{	
	checkMouseInput();
	render(deltaTime);
}

void Selection_Gizmo::checkMouseInput()
{	
	// See if the mouse intersects any entities.
	// In any case move the selection gizmo to where the mouse is.
	if (!ImGui::GetIO().WantCaptureMouse && ImGui::IsMouseDown(0) && !m_clicked) {
		m_clicked = true;
		glm::vec3 newPosition(0);
		if (auto entity = rayCastMouse(newPosition)) {
			// Change the selected entity so all UI elements that need this info are updated
			//m_editor->setSelection(entity);
		}
		setPosition(newPosition);
		m_editor->setGizmoPosition(newPosition);
	}
	else
		m_clicked = false;
}

void Selection_Gizmo::render(const float & deltaTime)
{
	// Safety check first
	if (m_selIndicator->existsYet() && m_colorPalette->existsYet() && m_gizmoShader->existsYet() && m_wireframeShader->existsYet()) {
		// Set up state
		m_editor->bindFBO();
		m_selIndicator->bind();
		m_colorPalette->bind(0);
		m_indicatorIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);

		// Generate matrices
		auto pMatrix = m_engine->getModule_Graphics().getClientCamera()->get()->pMatrix;
		auto vMatrix = m_engine->getModule_Graphics().getClientCamera()->get()->vMatrix;
		auto mMatrix = glm::translate(glm::mat4(1.0f), m_position) * glm::scale(glm::mat4(1.0f), glm::vec3(glm::distance(m_position, m_engine->getModule_Graphics().getClientCamera()->get()->EyePosition) * 0.033f));
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
}

void Selection_Gizmo::setPosition(const glm::vec3 & position)
{
	m_position = position;
}

EntityHandle Selection_Gizmo::rayCastMouse(glm::vec3 & positionOut) const
{
	const auto & actionState = m_engine->getActionState();
	const auto & clientCamera = *m_engine->getModule_Graphics().getClientCamera()->get();
	const auto ray_nds = glm::vec2(2.0f * actionState.at(ActionState::MOUSE_X) / m_renderSize.x - 1.0f, 1.0f - (2.0f * actionState.at(ActionState::MOUSE_Y)) / m_renderSize.y);
	const auto ray_eye = glm::vec4(glm::vec2(clientCamera.pMatrixInverse * glm::vec4(ray_nds, -1.0f, 1.0F)), -1.0f, 0.0f);
	const auto ray_world = glm::normalize(glm::vec3(clientCamera.vMatrixInverse * ray_eye));
	positionOut = clientCamera.EyePosition + (ray_world * glm::vec3(50.0f));
	return NULL_ENTITY_HANDLE;
}