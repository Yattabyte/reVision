#include "Modules/Editor/Editor_M.h"
#include "Modules/Editor/UI/Editor_Interface.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "Engine.h"


void LevelEditor_Module::initialize(Engine * engine)
{
	Engine_Module::initialize(engine);
	m_engine->getManager_Messages().statement("Loading Module: Level Edtior...");
	
	// UI
	m_editorInterface = std::make_shared<Editor_Interface>(engine, this);	
}

void LevelEditor_Module::deinitialize()
{
	m_engine->getManager_Messages().statement("Unloading Module: Level Edtior...");		
}

void LevelEditor_Module::frameTick(const float & deltaTime)
{		
}

void LevelEditor_Module::showEditor()
{
	m_engine->getModule_UI().clear();
	m_engine->getModule_UI().setRootElement(m_editorInterface);
	openLevel("a.bmap");
}

void LevelEditor_Module::exit()
{
	m_engine->goToMainMenu();
	m_currentLevelName = "";
}

void LevelEditor_Module::newLevel()
{
	m_engine->getModule_World().unloadWorld();
	m_currentLevelName = "";
}

void LevelEditor_Module::openLevel(const std::string & name)
{
	m_engine->getModule_World().loadWorld(name);
	m_currentLevelName = name;
}

void LevelEditor_Module::openLevelDialog()
{
	openLevel(m_currentLevelName);
}

void LevelEditor_Module::saveLevel(const std::string & name)
{
	m_engine->getModule_World().saveWorld(name);
	m_currentLevelName = name;
}

void LevelEditor_Module::saveLevel()
{
	saveLevel(m_currentLevelName);
}

void LevelEditor_Module::saveLevelDialog()
{
}

void LevelEditor_Module::undo()
{
}

void LevelEditor_Module::redo()
{
}

void LevelEditor_Module::cut()
{
}

void LevelEditor_Module::copy()
{
}

void LevelEditor_Module::paste()
{
}

void LevelEditor_Module::deleteObject()
{
}