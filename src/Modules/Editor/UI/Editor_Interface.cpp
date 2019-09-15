#include "Modules/Editor/UI/Editor_Interface.h"
#include "Modules/Editor/UI/CameraController.h"
#include "Modules/Editor/UI/RotationIndicator.h"
#include "Modules/Editor/UI/TitleBar.h"
#include "Modules/Editor/UI/Prefabs.h"
#include "Modules/Editor/UI/Inspector.h"
#include "Modules/Editor/UI/RecoverDialogue.h"
#include "Modules/Editor/UI/OpenDialogue.h"
#include "Modules/Editor/UI/SaveDialogue.h"
#include "Modules/Editor/UI/UnsavedChangesDialogue.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "Engine.h"
#include "GLFW/glfw3.h"


Editor_Interface::Editor_Interface(Engine * engine, LevelEditor_Module * editor)
	: m_engine(engine), m_editor(editor)
{
	m_uiCamController = std::make_shared<CameraController>(m_engine);
	m_uiTitlebar = std::make_shared<TitleBar>(engine, editor);
	m_uiPrefabs = std::make_shared<Prefabs>(engine, editor);
	m_uiInspector = std::make_shared<Inspector>(engine, editor);
	m_uiRotIndicator = std::make_shared<RotationIndicator>(m_engine);
	m_uiRecoverDialogue = std::make_shared<RecoverDialogue>(engine, editor);
	m_uiOpenDialogue = std::make_shared<OpenDialogue>(engine, editor);
	m_uiSaveDialogue = std::make_shared<SaveDialogue>(engine, editor);
	m_uiUnsavedDialogue = std::make_shared<UnsavedChangesDialogue>(engine, editor);

	m_shader = Shared_Shader(engine, "Editor\\editorCopy");
	m_shapeQuad = Shared_Auto_Model(engine, "quad");

	// Asset-Finished Callbacks
	m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
		m_indirectQuad = IndirectDraw((GLuint)m_shapeQuad->getSize(), 1, 0, GL_CLIENT_STORAGE_BIT);
	});

	auto& preferences = engine->getPreferenceState();
	preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
	preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float& f) {
		m_renderSize.x = (int)f;
	});
	preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float& f) {
		m_renderSize.y = (int)f;
	});
}

void Editor_Interface::tick(const float & deltaTime)
{
	// Container for left side of the screen
	ImGui::SetNextWindowSize({ 300.0f, m_renderSize.y - 18.0f }, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos({ 0, 18.0f }, ImGuiCond_Appearing);
	ImGui::Begin("Left Panel", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);
	ImGui::DockSpace(ImGui::GetID("LeftDock"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::End();

	//ImGui::DockSpace(0, { 300.0f, m_renderSize.y - 18.0f }, 0);

	// Container for right side of the screen
	ImGui::SetNextWindowSize({ 300.0f, m_renderSize.y - 18.0f }, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos({ m_renderSize.x - 300.0f, 18.0f }, ImGuiCond_Appearing);
	ImGui::Begin("Right Panel", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);
	ImGui::DockSpace(ImGui::GetID("RightDock"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::End();

	// Process all UI elements
	const std::shared_ptr<ImGUI_Element> elements[] = {
		m_uiCamController,m_uiTitlebar,m_uiPrefabs,m_uiInspector,m_uiRotIndicator,m_uiRecoverDialogue,m_uiOpenDialogue,m_uiSaveDialogue,m_uiUnsavedDialogue
	};
	for each (auto & element in elements)
		element->tick(deltaTime);

	// Render overlay
	if (m_shapeQuad->existsYet() && m_shader->existsYet()) {
		// Set up state
		m_editor->bindTexture();
		m_shader->bind();
		m_shapeQuad->bind();
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		m_indirectQuad.drawCall();
	}
}