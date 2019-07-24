#include "Modules/Editor/UI/Editor_Interface.h"
#include "Engine.h"


Editor_Interface::Editor_Interface(Engine * engine, LevelEditor_Module * editor)
	: m_engine(engine)
{
	m_elements.push_back(new TitleBar(engine, editor));
}

void Editor_Interface::render(const float & deltaTime) 
{
	for each (auto & element in m_elements)
		element->render(deltaTime);
}