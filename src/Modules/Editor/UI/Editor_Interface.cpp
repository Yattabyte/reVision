#include "Modules/Editor/UI/Editor_Interface.h"
#include "Modules/Editor/UI/CameraController.h"
#include "Modules/Editor/UI/RotationIndicator.h"
#include "Modules/Editor/UI/TitleBar.h"
#include "Modules/Editor/UI/Prefabs.h"
#include "Modules/Editor/UI/Inspector.h"
#include "Engine.h"


Editor_Interface::Editor_Interface(Engine * engine, LevelEditor_Module * editor)
	: m_engine(engine), m_editor(editor)
{
	m_elements.push_back(new CameraController(engine, editor));
	m_elements.push_back(new RotationIndicator(engine, editor));
	m_elements.push_back(new TitleBar(engine, editor));
	m_elements.push_back(new Prefabs(engine, editor));
	m_elements.push_back(new Inspector(engine, editor));

	m_shader = Shared_Shader(m_engine, "Editor\\editorCopy");
	m_shapeQuad = Shared_Auto_Model(m_engine, "quad");

	// Asset-Finished Callbacks
	m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
		const GLuint data[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
		m_indirectBuffer = StaticBuffer(sizeof(GLuint) * 4, data, GL_CLIENT_STORAGE_BIT);
	});
}

void Editor_Interface::tick(const float & deltaTime)
{
	for each (auto & element in m_elements)
		element->tick(deltaTime);

	if (m_shapeQuad->existsYet() && m_shader->existsYet()) {
		// Set up state
		m_editor->bindTexture();
		m_shader->bind();
		m_shapeQuad->bind();
		m_indirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Draw
		glDrawArraysIndirect(GL_TRIANGLES, 0);
	}
}