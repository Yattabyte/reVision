#include "Engine.h"
#include "imgui.h"
#include "LinearMath/btScalar.h"
#include <direct.h>

// Importers Used //
#include "Utilities/IO/Image_IO.h"
#include "Utilities/IO/Mesh_IO.h"


Engine::~Engine()
{
	m_modulePhysics.deinitialize();
	m_moduleUI.deinitialize();
	m_moduleGraphics.deinitialize();
	m_moduleEditor.deinitialize();
	m_moduleGame.deinitialize();
	m_moduleStartScreen.deinitialize();
	m_moduleECS.deinitialize();

	Image_IO::Deinitialize();
	m_messageManager.statement("Shutting down...");
}

Engine::Engine() noexcept :
	// Initialize engine-dependent members first
	m_preferenceState(*this),
	m_inputBindings(*this),
	m_window(*this),
	m_moduleECS(*this),
	m_moduleStartScreen(*this),
	m_moduleGame(*this),
	m_moduleEditor(*this),
	m_moduleGraphics(*this),
	m_moduleUI(*this),
	m_modulePhysics(*this)
{
	Image_IO::Initialize();
	m_inputBindings.loadFile("binds");

	printBoilerPlate();
	m_moduleECS.initialize();
	m_moduleGraphics.initialize();
	m_moduleUI.initialize();
	m_modulePhysics.initialize();
	m_moduleStartScreen.initialize();
	m_moduleEditor.initialize();
	m_moduleGame.initialize();

	goToMainMenu();
}

void Engine::printBoilerPlate()
{
	m_messageManager.statement("+~-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-~\\");
	m_messageManager.statement("  > reVision Engine:");
	m_messageManager.statement("  -------------------------------");
#ifdef NDEBUG
	m_messageManager.statement("  " + std::string(ENGINE_VERSION) + " (RELEASE)");
#else
	m_messageManager.statement("  " + std::string(ENGINE_VERSION) + " (DEBUG)");
#endif // DEBUG
	m_messageManager.statement("  " + std::string(__TIMESTAMP__));
	m_messageManager.statement("");
	m_messageManager.statement("  > Library Info:");
	m_messageManager.statement("  -------------------------------");
	m_messageManager.statement("  ASSIMP       " + Mesh_IO::Get_Version());
	m_messageManager.statement("  Bullet       " + std::to_string(BT_BULLET_VERSION));
	m_messageManager.statement("  Dear ImGui   " + std::string(ImGui::GetVersion()));
	m_messageManager.statement("  FreeImage    " + Image_IO::Get_Version());
	m_messageManager.statement("  GLAD         " + std::to_string(GLVersion.major) + "." + std::to_string(GLVersion.minor));
	m_messageManager.statement("  GLFW         " + Window::GetVersion());
	m_messageManager.statement("  GLM          " + std::to_string(GLM_VERSION_MAJOR) + "." + std::to_string(GLM_VERSION_MINOR) + "." + std::to_string(GLM_VERSION_PATCH) + "." + std::to_string(GLM_VERSION_REVISION));
	m_messageManager.statement("  SoLoud       " + std::to_string(SoundManager::GetVersion()));
	m_messageManager.statement("");
	m_messageManager.statement("  > Graphics Info:");
	m_messageManager.statement("  -------------------------------");
	m_messageManager.statement("  " + std::string(reinterpret_cast<char const*>(glGetString(GL_RENDERER))));
	m_messageManager.statement("  OpenGL " + std::string(reinterpret_cast<char const*>(glGetString(GL_VERSION))));
	m_messageManager.statement("  GLSL " + std::string(reinterpret_cast<char const*>(glGetString(GL_SHADING_LANGUAGE_VERSION))));
	m_messageManager.statement("+~-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-~/");
}

void Engine::tick()
{
	const float thisTime = GetSystemTime();
	const float deltaTime = thisTime - m_lastTime;
	m_lastTime = thisTime;

	processInputs();
	m_assetManager.notifyObservers();
	m_moduleUI.applyActionState(m_actionState);

	// Tick relevant systems
	if (m_engineState == Engine_State::in_startMenu)
		m_moduleStartScreen.frameTick(deltaTime);
	else if (m_engineState == Engine_State::in_game)
		m_moduleGame.frameTick(deltaTime);
	else if (m_engineState == Engine_State::in_editor)
		m_moduleEditor.frameTick(deltaTime);
	m_moduleUI.frameTick(deltaTime);

	m_window.swapBuffers();
}

void Engine::tickThreaded(std::future<void> exitObject, GLFWwindow* const auxContext)
{
	Window::MakeCurrent(auxContext);

	// Check if thread should shutdown
	while (exitObject.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout)
		m_assetManager.beginWorkOrder();	
}

bool Engine::shouldClose() const noexcept
{
	return m_window.shouldClose();
}

void Engine::shutDown() noexcept
{
	m_window.close();
}

void Engine::setMouseInputMode(const MouseInputMode& mode)
{
	m_mouseInputMode = mode;
	switch (mode) {
	case MouseInputMode::NORMAL:
		m_window.setMouseMode3D(false);
		break;
	case MouseInputMode::FREE_LOOK:
		m_window.setMouseMode3D(true);
		m_window.setMousePos({ m_actionState[ActionState::Action::LOOK_X], m_actionState[ActionState::Action::LOOK_Y] });
		m_actionState[ActionState::Action::LOOK_X] = m_actionState[ActionState::Action::MOUSE_X];
		m_actionState[ActionState::Action::LOOK_Y] = m_actionState[ActionState::Action::MOUSE_Y];
		break;
	}
}

void Engine::goToMainMenu()
{
	m_engineState = Engine_State::in_startMenu;
	m_moduleStartScreen.showStartMenu();
}

void Engine::goToGame()
{
	m_engineState = Engine_State::in_game;
	m_moduleGame.showGame();
}

void Engine::goToEditor()
{
	m_engineState = Engine_State::in_editor;
	m_moduleEditor.showEditor();
}

Engine::MouseInputMode Engine::getMouseInputMode() const noexcept 
{ 
	return m_mouseInputMode;
}

Window& Engine::getWindow() noexcept
{
	return m_window;
}

float Engine::GetSystemTime() noexcept
{
	return Window::GetSystemTime();
}

ActionState& Engine::getActionState() noexcept 
{
	return m_actionState; 
}

PreferenceState& Engine::getPreferenceState() noexcept 
{ 
	return m_preferenceState; 
}

AssetManager& Engine::getManager_Assets() noexcept 
{ 
	return m_assetManager;
}

MessageManager& Engine::getManager_Messages() noexcept
{
	return m_messageManager; 
}

SoundManager& Engine::getManager_Sounds() noexcept 
{
	return m_soundManager;
}

ECS_Module& Engine::getModule_ECS() noexcept 
{
	return m_moduleECS;
}

Game_Module& Engine::getModule_Game() noexcept 
{
	return m_moduleGame; 
}

LevelEditor_Module& Engine::getModule_LevelEditor() noexcept
{ 
	return m_moduleEditor;
}

Graphics_Module& Engine::getModule_Graphics() noexcept
{ 
	return m_moduleGraphics; 
}

UI_Module& Engine::getModule_UI() noexcept 
{ 
	return m_moduleUI;
}

Physics_Module& Engine::getModule_Physics() noexcept 
{
	return m_modulePhysics; 
}

std::string Engine::Get_Current_Dir()
{
	// Technique to return the running directory of the application
	char cCurrentPath[FILENAME_MAX];
	if (_getcwd(cCurrentPath, sizeof(cCurrentPath)) != nullptr)
		cCurrentPath[sizeof(cCurrentPath) - 1ULL] = '\0';
	return std::string(cCurrentPath);
}

bool Engine::File_Exists(const std::string& name)
{
	// Technique to return whether or not a given file or folder exists
	struct stat buffer{};
	return (stat((Engine::Get_Current_Dir() + name).c_str(), &buffer) == 0);
}

void Engine::processInputs()
{
	// Updated mouse states, manually
	auto& actionState = getActionState();
	const auto cursorPos = m_window.getMousePos();
	actionState[ActionState::Action::MOUSE_L] = static_cast<float>(m_window.getMouseKey(0));
	actionState[ActionState::Action::MOUSE_R] = static_cast<float>(m_window.getMouseKey(1));
	actionState[ActionState::Action::MOUSE_X] = cursorPos.x;
	actionState[ActionState::Action::MOUSE_Y] = cursorPos.y;
	if (m_mouseInputMode == MouseInputMode::FREE_LOOK)
		m_window.setMousePos({ 0, 0 });

	// Update key binding states, manually
	if (const auto& bindings = m_inputBindings.getBindings())
		if (bindings->ready())
			for (const auto& pair : bindings.get()->m_configuration)
				m_actionState[ActionState::Action(pair.first)] = static_cast<float>(m_window.getKey(static_cast<int>(pair.second)));
}