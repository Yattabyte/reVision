#include "Modules/Game/Overlays/LoadingIndicator.h"
#include "Engine.h"


LoadingIndicator::~LoadingIndicator()
{
	// Update indicator
	*m_aliveIndicator = false;
}

LoadingIndicator::LoadingIndicator(Engine& engine) :
	m_engine(engine),
	m_shader(Shared_Shader(engine, "Effects\\LoadingIndicator")),
	m_texture(Shared_Texture(engine, "spinner.png", GL_TEXTURE_2D)),
	m_shapeQuad(Shared_Auto_Model(engine, "quad"))
{
	// Preferences
	auto& preferences = engine.getPreferenceState();
	preferences.getOrSetValue(PreferenceState::Preference::C_WINDOW_WIDTH, m_renderSize.x);
	preferences.getOrSetValue(PreferenceState::Preference::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::Preference::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float& f) { resize(glm::vec2(f, m_renderSize.y)); });
	preferences.addCallback(PreferenceState::Preference::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float& f) { resize(glm::vec2(m_renderSize.x, f)); });
	resize(m_renderSize);

	// Asset-Finished Callbacks
	m_shapeQuad->addCallback(m_aliveIndicator, [&]() noexcept {
		m_indirectQuad = IndirectDraw<1>((GLuint)m_shapeQuad->getSize(), 1, 0, GL_CLIENT_STORAGE_BIT);
		});
}

void LoadingIndicator::applyEffect(const float& deltaTime) 
{
	if (!Asset::All_Ready(m_shapeQuad, m_shader, m_texture))
		return;
	if (m_show)
		m_blendAmt += deltaTime;
	else
		m_blendAmt -= deltaTime;
	if (!m_show && m_blendAmt <= 0.0f)
		return;
	m_blendAmt = std::max<float>(0.0f, std::min<float>(1.0f, m_blendAmt));
	if (m_blendAmt > -0.0001f || m_blendAmt < 1.0001f) {
		m_time += deltaTime;
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glViewport(0, 0, m_renderSize.x, m_renderSize.y);
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_texture->bind(0);
		m_shader->bind();
		m_shader->setUniform(1, m_projMatrix);
		m_shader->setUniform(2, glm::translate(glm::mat4(1.0f), glm::vec3(m_renderSize.x - 64, 64, 0)) * glm::scale(glm::mat4(1.0f), glm::vec3(32)));
		m_shader->setUniform(3, m_time);
		m_shader->setUniform(4, m_blendAmt);
		m_indirectQuad.drawCall();
		glDisable(GL_BLEND);
		Shader::Release();
	}
}

void LoadingIndicator::resize(const glm::ivec2& size) 
{
	m_renderSize = size;
	m_projMatrix = glm::ortho(0.0f, (float)size.x, 0.0f, (float)size.y);
}