#include "Modules/Editor/Editor_M.h"
#include "Modules/Editor/UI/Editor_Interface.h"
#include "Engine.h"


void LevelEditor_Module::initialize(Engine * engine)
{
	Engine_Module::initialize(engine);
	m_engine->getManager_Messages().statement("Loading Module: Level Edtior...");
	m_editorInterface = std::make_shared<Editor_Interface>(engine);
}

void LevelEditor_Module::deinitialize()
{
	m_engine->getManager_Messages().statement("Closing Module: Level Edtior...");	
}

void LevelEditor_Module::frameTick(const float & deltaTime)
{
	// Update our own ECS systems

	// Render UI
}

void LevelEditor_Module::start()
{
	m_engine->getModule_UI().clear();
	m_engine->getModule_UI().pushRootElement(m_editorInterface);
}
