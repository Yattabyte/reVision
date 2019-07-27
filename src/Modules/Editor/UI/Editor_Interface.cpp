#include "Modules/Editor/UI/Editor_Interface.h"
#include "Modules/Editor/UI/CameraController.h"
#include "Modules/Editor/UI/RotationIndicator.h"
#include "Modules/Editor/UI/TitleBar.h"
#include "Modules/Editor/UI/Prefabs.h"
#include "Modules/Editor/UI/Inspector.h"
#include "Engine.h"


Editor_Interface::Editor_Interface(Engine * engine, LevelEditor_Module * editor)
	: m_engine(engine)
{
	m_elements.push_back(new CameraController(engine, editor));
	m_elements.push_back(new RotationIndicator(engine, editor));
	m_elements.push_back(new TitleBar(engine, editor));
	m_elements.push_back(new Prefabs(engine, editor));
	m_elements.push_back(new Inspector(engine, editor));
}

void Editor_Interface::tick(const float & deltaTime)
{
	for each (auto & element in m_elements)
		element->tick(deltaTime);
}