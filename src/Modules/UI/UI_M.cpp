#include "Modules/UI/UI_M.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "Modules/UI/dear imgui/imgui_impl_glfw.h"
#include "Modules/UI/dear imgui/imgui_impl_opengl3.h"
#include "Modules/UI/Macro Elements/StartMenu.h"
#include "Engine.h"


void UI_Module::initialize(Engine * engine)
{
	Engine_Module::initialize(engine);
	m_engine->getManager_Messages().statement("Loading Module: User Interface...");

	// Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Gamepad Controls
	io.ConfigWindowsMoveFromTitleBarOnly = true;
	io.ConfigDockingWithShift = true;
	ImGuiStyle& style = ImGui::GetStyle();
	style.FrameRounding = 6.0f;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(engine->getContext(), true);
	ImGui_ImplOpenGL3_Init("#version 150");

	// Preferences
	auto & preferences = m_engine->getPreferenceState();
	constexpr static auto calcOthoProj = [](const glm::ivec2 & renderSize, StaticBuffer & projectionBuffer) {
		const glm::mat4 proj = glm::ortho<float>(0.0f, (float)renderSize.x, 0.0f, (float)renderSize.y, -1.0f, 1.0f);
		projectionBuffer.write(0, sizeof(glm::mat4), &proj[0][0]);
	};
	m_projectionBuffer = StaticBuffer(sizeof(glm::mat4), 0, GL_DYNAMIC_STORAGE_BIT);
	preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
	preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) {
		m_renderSize.x = (int)f;
		calcOthoProj(m_renderSize, m_projectionBuffer);
		for each (auto element in m_rootElement)
			element->setScale(m_renderSize);
	});
	preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {
		m_renderSize.y = (int)f;
		calcOthoProj(m_renderSize, m_projectionBuffer);
		for each (auto element in m_rootElement)
			element->setScale(m_renderSize);
	});
	calcOthoProj(m_renderSize, m_projectionBuffer);	
}

void UI_Module::deinitialize()
{
	m_engine->getManager_Messages().statement("Unloading Module: User Interface...");

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Update indicator
	*m_aliveIndicator = false;
}

void UI_Module::frameTick(const float & deltaTime)
{
	// Copy the list of callbacks, execute a copy of them
	// We use a copy because any callback may alter the list, 
	auto copySelection = m_callbacks;
	m_callbacks.clear();
	for each (const auto & func in copySelection)
		func();

	if (m_rootElement.size() && m_rootElement.back()) {
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);
		glViewport(0, 0, (GLsizei)m_renderSize.x, (GLsizei)m_renderSize.y);

		m_projectionBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2);
		m_rootElement.back()->renderElement(deltaTime, glm::vec2(0.0f), m_renderSize);

		glDisable(GL_BLEND);
		Shader::Release();
	}

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	bool show_demo_window = true;
	ImGui::ShowDemoWindow(&show_demo_window);

	if (m_rootUIElement)
		m_rootUIElement->tick(deltaTime);

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UI_Module::pushRootElement(const std::shared_ptr<UI_Element>& rootElement)
{
	m_rootElement.push_back(rootElement);
	if (rootElement)
		rootElement->setScale(m_renderSize);
}

void UI_Module::popRootElement()
{
	if (m_rootElement.size() > 1)
		m_rootElement.pop_back();
}

void UI_Module::setFocusMap(const std::shared_ptr<FocusMap> & focusMap)
{
	m_focusMap = focusMap;
}

std::shared_ptr<FocusMap> UI_Module::getFocusMap() const
{
	return m_focusMap;
}

void UI_Module::clear()
{
	m_rootElement.clear();
	m_rootUIElement.reset();
	m_focusMap.reset();
}

void UI_Module::applyCursorPos(const double & xPos, const double & yPos)
{
	m_mouseEvent.m_xPos = xPos;
	m_mouseEvent.m_yPos = m_renderSize.y - yPos;
	m_mouseEvent.m_action = MouseEvent::MOVE;

	if (m_rootElement.size())
		m_rootElement.back()->mouseAction(m_mouseEvent);
}

void UI_Module::applyCursorButton(const int & button, const int & action, const int & mods)
{
	m_mouseEvent.m_button = button;
	m_mouseEvent.m_action = action;
	m_mouseEvent.m_mods = mods;

	if (m_rootElement.size())
		m_rootElement.back()->mouseAction(m_mouseEvent);
}

void UI_Module::applyChar(const unsigned int & character)
{
	m_keyboardEvent.setChar(character);
	if (m_rootElement.size())
		m_rootElement.back()->keyboardAction(m_keyboardEvent);
	m_keyboardEvent.setChar(0);
}

void UI_Module::applyKey(const int & key, const int &, const int & action, const int &)
{
	m_keyboardEvent.setState(KeyboardEvent::Key((unsigned int)key), KeyboardEvent::Action(action));
	if (m_rootElement.size())
		m_rootElement.back()->keyboardAction(m_keyboardEvent);
}

void UI_Module::applyActionState(ActionState & actionState)
{
	if (m_focusMap)
		m_focusMap->applyActionState(actionState);
}

void UI_Module::pushCallback(const std::function<void()> & callback)
{
	m_callbacks.push_back(callback);
}
