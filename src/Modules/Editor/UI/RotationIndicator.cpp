#include "Modules/Editor/UI/RotationIndicator.h"
#include "Modules/Editor/Editor_M.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "Engine.h"
#include "glm/gtc/matrix_transform.hpp"


RotationIndicator::~RotationIndicator()
{
	// Update indicator
	*m_aliveIndicator = false;

	glDeleteFramebuffers(1, &m_fboID);
	glDeleteTextures(1, &m_texID);
	glDeleteTextures(1, &m_depthID);
}

RotationIndicator::RotationIndicator(Engine * engine, LevelEditor_Module * editor)
	: m_engine(engine), m_editor(editor)
{
	auto & preferences = engine->getPreferenceState();
	preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
	preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) {
		m_renderSize.x = (int)f;
	});
	preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {
		m_renderSize.y = (int)f;
	});

	// Assets
	m_colorPalette = Shared_Texture(engine, "Editor\\colors.png");
	m_3dIndicator = Shared_Auto_Model(engine, "Editor\\3dind");
	m_gizmoShader = Shared_Shader(engine, "Editor\\gizmoShader");

	// Asset-Finished Callbacks
	m_3dIndicator->addCallback(m_aliveIndicator, [&]() mutable {
		const GLuint data[4] = { (GLuint)m_3dIndicator->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
		m_indicatorIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, data, GL_CLIENT_STORAGE_BIT);
	});

	// GL structures
	glCreateFramebuffers(1, &m_fboID);
	glCreateTextures(GL_TEXTURE_2D, 1, &m_texID);
	glTextureParameteri(m_texID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_texID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_texID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(m_texID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureStorage2D(m_texID, 1, GL_RGBA16F, 256, 256);
	glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_texID, 0);
	glCreateTextures(GL_TEXTURE_2D, 1, &m_depthID);
	glTextureParameteri(m_depthID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_depthID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_depthID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_depthID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureStorage2D(m_depthID, 1, GL_DEPTH_COMPONENT16, 256, 256);
	glNamedFramebufferTexture(m_fboID, GL_DEPTH_ATTACHMENT, m_depthID, 0);
	glNamedFramebufferDrawBuffer(m_fboID, GL_COLOR_ATTACHMENT0);
}

void RotationIndicator::tick(const float & deltaTime)
{
	if (m_3dIndicator->existsYet() && m_colorPalette->existsYet() && m_gizmoShader->existsYet()) {
		glViewport(0, 0, 256, 256);
		constexpr GLfloat clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		constexpr GLfloat clearDepth = 1.0f;
		glClearNamedFramebufferfv(m_fboID, GL_COLOR, 0, clearColor);
		glClearNamedFramebufferfv(m_fboID, GL_DEPTH, 0, &clearDepth);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);

		// Set up state
		m_gizmoShader->bind();
		m_3dIndicator->bind();
		m_colorPalette->bind(0);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(true);

		// Generate matrices
		auto pMatrix = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, -10.0f, 10.0f);
		auto camMatrix = m_engine->getModule_Graphics().getClientCamera()->get()->vMatrix;
		camMatrix[3][0] = 0.0f;
		camMatrix[3][1] = 0.0f;
		camMatrix[3][2] = 0.0f;
		camMatrix[3][3] = 1.0f;
		auto vMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -6.0f)) * camMatrix;
		m_gizmoShader->setUniform(0, pMatrix * vMatrix);

		m_indicatorIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		m_gizmoShader->Release();
		glDepthMask(false);
		glDisable(GL_DEPTH_TEST);
		glViewport(0, 0, m_renderSize.x, m_renderSize.y);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

	ImGui::SetNextWindowSize({ 128,128 }, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos({ m_renderSize.x - 128.0f, m_renderSize.y - 128.0f }, ImGuiCond_Appearing);
	if (ImGui::Begin("Rotation Indicator", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground)) {
		ImGui::Image((ImTextureID)static_cast<uintptr_t>(m_texID), { 128.0f, 128.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f });
	}
	ImGui::End();
}