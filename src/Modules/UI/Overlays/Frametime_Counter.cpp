#include "Modules/UI/Overlays/Frametime_Counter.h"
#include "Engine.h"


Frametime_Counter::~Frametime_Counter()
{
	// Update indicator
	*m_aliveIndicator = false;
}

Frametime_Counter::Frametime_Counter(Engine& engine) :
	m_engine(engine),
	m_shader(Shared_Shader(engine, "Utilities\\numberPrint")),
	m_numberTexture(Shared_Texture(engine, "numbers.png", GL_TEXTURE_2D, false, false)),
	m_shapeQuad(Shared_Auto_Model(engine, "quad"))
{
	// Preferences
	auto& preferences = engine.getPreferenceState();
	preferences.getOrSetValue(PreferenceState::Preference::C_WINDOW_WIDTH, m_renderSize.x);
	preferences.getOrSetValue(PreferenceState::Preference::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::Preference::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float& f) { resize(glm::vec2(f, m_renderSize.y)); });
	preferences.addCallback(PreferenceState::Preference::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float& f) { resize(glm::vec2(m_renderSize.x, f)); });
	resize(m_renderSize);
}

void Frametime_Counter::applyEffect(const float& deltaTime)
{
	if (!Asset::All_Ready(m_shapeQuad, m_shader, m_numberTexture))
		return;
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, m_renderSize.x, m_renderSize.y);
	glBindVertexArray(m_shapeQuad->m_vaoID);
	m_numberTexture->bind(0);
	m_shader->bind();
	m_shader->setUniform(1, m_projMatrix);
	const glm::mat4 scale = glm::translate(glm::mat4(1.0F), glm::vec3(12, 12, 0)) * glm::scale(glm::mat4(1.0F), glm::vec3(12));

	const float dt_seconds = deltaTime * 1000.0F;
	std::string test = std::to_string(dt_seconds);
	bool foundDecimal = false;
	int decimalCount = 0;
	bool repeat = true;
	const auto length = test.size();
	for (size_t x = 0; x < length && repeat; ++x) {
		if (foundDecimal)
			decimalCount++;
		if (decimalCount >= 2)
			repeat = false;
		const int number = test[x];
		if (number == 46) {
			foundDecimal = true;
			m_shader->setUniform(3, 10); // set texture index to 11
		}
		else
			m_shader->setUniform(3, number - 48); // remove the ASCII encoding, convert to int

		m_shader->setUniform(2, glm::translate(glm::mat4(1.0F), glm::vec3(x * 24, 24, 0)) * scale);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	glDisable(GL_BLEND);
	Shader::Release();
}

void Frametime_Counter::resize(const glm::ivec2& size)
{
	m_renderSize = size;
	m_projMatrix = glm::ortho(0.0F, static_cast<float>(size.x), 0.0F, static_cast<float>(size.y));
}