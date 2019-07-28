#include "Modules/Editor/Gizmos/Selection.h"
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
}

void Selection_Gizmo::setPosition(const glm::vec3 & position)
{
	m_position = position;
}

void Selection_Gizmo::frameTick(const float & deltaTime)
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
		auto mMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(glm::distance(m_position, m_engine->getModule_Graphics().getClientCamera()->get()->EyePosition) * 0.033f));
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