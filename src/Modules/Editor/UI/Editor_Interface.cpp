#include "Modules/Editor/UI/Editor_Interface.h"
#include "Modules/Editor/UI/Hotkeys.h"
#include "Modules/Editor/UI/CameraController.h"
#include "Modules/Editor/UI/RotationIndicator.h"
#include "Modules/Editor/UI/TitleBar.h"
#include "Modules/Editor/UI/Prefabs.h"
#include "Modules/Editor/UI/SceneInspector.h"
#include "Modules/Editor/UI/EntityInspector.h"
#include "Modules/Editor/UI/RecoverDialogue.h"
#include "Modules/Editor/UI/OpenDialogue.h"
#include "Modules/Editor/UI/SaveDialogue.h"
#include "Modules/Editor/UI/UnsavedChangesDialogue.h"
#include "Modules/Editor/UI/MissingFileDialogue.h"
#include "Modules/Editor/UI/Settings.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"
#include "Engine.h"
#include "GLFW/glfw3.h"


Editor_Interface::~Editor_Interface()
{
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

Editor_Interface::Editor_Interface(Engine* engine, LevelEditor_Module* editor)
	: m_engine(engine), m_editor(editor)
{
	m_uiHotkeys = std::make_shared<Hotkeys>(engine, editor);
	m_uiCamController = std::make_shared<CameraController>(engine);
	m_uiRotIndicator = std::make_shared<RotationIndicator>(engine);
	m_uiTitlebar = std::make_shared<TitleBar>(engine, editor);
	m_uiPrefabs = std::make_shared<Prefabs>(engine, editor);
	m_uiSceneInspector = std::make_shared<SceneInspector>(engine, editor);
	m_uiEntityInspector = std::make_shared<EntityInspector>(engine, editor);
	m_uiSettings = std::make_shared<Settings>(engine, editor);
	m_uiRecoverDialogue = std::make_shared<RecoverDialogue>(engine, editor);
	m_uiOpenDialogue = std::make_shared<OpenDialogue>(engine, editor);
	m_uiSaveDialogue = std::make_shared<SaveDialogue>(engine, editor);
	m_uiUnsavedDialogue = std::make_shared<UnsavedChangesDialogue>(engine, editor);
	m_uiMissingDialogue = std::make_shared<MissingFileDialogue>(engine, editor);

	// Preferences
	auto& preferences = engine->getPreferenceState();
	preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
	preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float& f) {
		m_renderSize.x = (int)f;
		});
	preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float& f) {
		m_renderSize.y = (int)f;
		});


	// Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;	// Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;	// Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;		// Enable Gamepad Controls
	io.ConfigWindowsMoveFromTitleBarOnly = true;
	io.ConfigDockingWithShift = true;
	ImGuiStyle& style = ImGui::GetStyle();
	style.FrameRounding = 6.0f;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(engine->getContext(), true);
	ImGui_ImplOpenGL3_Init("#version 150");
}

void Editor_Interface::tick(const float& deltaTime)
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	bool show_demo_window = true;
	ImGui::ShowDemoWindow(&show_demo_window);

	// Prepare the docking regions
	const auto dockspace_size = ImVec2(m_renderSize.x / 5.0F, m_renderSize.y);
	const auto window_flags = ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoNavFocus |
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Appearing);
	ImGui::SetNextWindowSize(dockspace_size, ImGuiCond_Appearing);
	ImGui::Begin("Docking Area A", NULL, window_flags);
	auto dockspace_id_a = ImGui::GetID("Docking Space A");
	if (ImGui::DockBuilderGetNode(dockspace_id_a) == NULL) {
		ImGui::DockBuilderRemoveNode(dockspace_id_a); // Clear out existing layout
		ImGui::DockBuilderAddNode(dockspace_id_a, ImGuiDockNodeFlags_DockSpace); // Add empty node
		ImGui::DockBuilderSetNodeSize(dockspace_id_a, dockspace_size);

		auto dock_id_top = dockspace_id_a;
		auto dock_id_bottom = ImGui::DockBuilderSplitNode(dock_id_top, ImGuiDir_Down, 0.50f, NULL, &dock_id_top);

		ImGui::DockBuilderDockWindow("Prefabs", dock_id_top);
		ImGui::DockBuilderDockWindow("Preferences", dock_id_bottom);
		ImGui::DockBuilderFinish(dockspace_id_a);
	}
	ImGui::DockSpace(dockspace_id_a, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(m_renderSize.x - (m_renderSize.x / 5.0F), 0), ImGuiCond_Appearing);
	ImGui::SetNextWindowSize(dockspace_size, ImGuiCond_Appearing);
	ImGui::Begin("Docking Area B", NULL, window_flags);
	auto dockspace_id_b = ImGui::GetID("Docking Space B");
	if (ImGui::DockBuilderGetNode(dockspace_id_b) == NULL) {
		ImGui::DockBuilderRemoveNode(dockspace_id_b); // Clear out existing layout
		ImGui::DockBuilderAddNode(dockspace_id_b, ImGuiDockNodeFlags_DockSpace); // Add empty node
		ImGui::DockBuilderSetNodeSize(dockspace_id_b, dockspace_size);

		auto dock_id_top = dockspace_id_b;
		auto dock_id_bottom = ImGui::DockBuilderSplitNode(dock_id_top, ImGuiDir_Down, 0.50f, NULL, &dock_id_top);

		ImGui::DockBuilderDockWindow("Scene Inspector", dock_id_top);
		ImGui::DockBuilderDockWindow("Entity Inspector", dock_id_bottom);
		ImGui::DockBuilderFinish(dockspace_id_b);
	}
	ImGui::DockSpace(dockspace_id_b, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();

	// Process all UI elements
	const auto elements = {
		m_uiHotkeys,m_uiCamController,m_uiRotIndicator,m_uiTitlebar,m_uiPrefabs,m_uiSceneInspector,m_uiEntityInspector,m_uiSettings,m_uiRecoverDialogue,m_uiOpenDialogue,m_uiSaveDialogue,m_uiUnsavedDialogue,m_uiMissingDialogue,
	};
	for each (auto & element in elements)
		element->tick(deltaTime);

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}