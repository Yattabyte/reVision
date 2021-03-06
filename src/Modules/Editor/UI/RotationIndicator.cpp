#include "Modules/Editor/UI/RotationIndicator.h"
#include "Modules/Editor/Editor_M.h"
#include "Engine.h"
#include "imgui.h"


RotationIndicator::~RotationIndicator()
{
	// Update indicator
	*m_aliveIndicator = false;

	glDeleteFramebuffers(1, &m_fboID);
	glDeleteTextures(1, &m_texID);
	glDeleteTextures(1, &m_depthID);
}

RotationIndicator::RotationIndicator(Engine& engine) :
	m_engine(engine)
{
	m_open = true;

	auto& preferences = engine.getPreferenceState();
	preferences.getOrSetValue(PreferenceState::Preference::C_WINDOW_WIDTH, m_renderSize.x);
	preferences.getOrSetValue(PreferenceState::Preference::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::Preference::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float& f) noexcept {
		m_renderSize.x = static_cast<int>(f);
		});
	preferences.addCallback(PreferenceState::Preference::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float& f) noexcept {
		m_renderSize.y = static_cast<int>(f);
		});

	// Assets
	m_colorPalette = Shared_Texture(engine, "Editor\\colors.png");
	m_3dIndicator = Shared_Auto_Model(engine, "Editor\\3dind");
	m_shader = Shared_Shader(engine, "Editor\\3dindShader");

	// Asset-Finished Callbacks
	m_3dIndicator->addCallback(m_aliveIndicator, [&]() noexcept {
		m_indirectIndicator = IndirectDraw<1>(static_cast<GLuint>(m_3dIndicator->getSize()), 1, 0, GL_CLIENT_STORAGE_BIT);
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

void RotationIndicator::tick(const float& /*deltaTime*/)
{
	if (m_open && Asset::All_Ready(m_3dIndicator, m_colorPalette, m_shader)) {
		// Set up state
		m_shader->bind();
		m_3dIndicator->bind();
		m_colorPalette->bind(0);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(true);

		glViewport(0, 0, 256, 256);
		constexpr GLfloat clearColor[] = { 0.0F, 0.0F, 0.0F, 0.0F };
		constexpr GLfloat clearDepth = 1.0F;
		glClearNamedFramebufferfv(m_fboID, GL_COLOR, 0, clearColor);
		glClearNamedFramebufferfv(m_fboID, GL_DEPTH, 0, &clearDepth);
		GLint previousFBO(0);
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previousFBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);

		// Generate matrices
		const auto pMatrix = glm::ortho(-4.2F, 4.2F, -4.2F, 4.2F, -10.0F, 10.0F);
		const auto camMatrix = glm::mat4_cast(glm::quat_cast(m_engine.getModule_Graphics().getClientCamera()->vMatrix));
		const auto vMatrix = glm::translate(glm::mat4(1.0F), glm::vec3(0, 0, -5.0F)) * camMatrix;
		m_shader->setUniform(0, pMatrix * vMatrix);

		m_indirectIndicator.drawCall();

		Shader::Release();
		glDepthMask(false);
		glDisable(GL_DEPTH_TEST);
		glViewport(0, 0, m_renderSize.x, m_renderSize.y);
		glBindFramebuffer(GL_FRAMEBUFFER, previousFBO);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0F);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0F, 0.0F));
		ImGui::SetNextWindowPos(ImVec2((static_cast<float>(m_renderSize.x) / 5.0F), 18.0F), ImGuiCond_Appearing);
		if (ImGui::Begin("Rotation Indicator", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground))
			ImGui::Image((ImTextureID)static_cast<uintptr_t>(m_texID), { 128.0F, 128.0F }, { 0.0F, 1.0F }, { 1.0F, 0.0F });
		ImGui::End();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
	}
}