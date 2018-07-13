#include "Systems\PerfCounter\PerfCounter.h"
#include "Engine.h"
#include "GLFW\glfw3.h"
#include "glm\gtc\matrix_transform.hpp"


System_PerfCounter::~System_PerfCounter()
{
	if (m_Initialized) {
		m_engine->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
		m_engine->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);
		if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
	}
}

System_PerfCounter::System_PerfCounter()
{
	// Default Parameters
	m_quadVAOLoaded = false;
}

void System_PerfCounter::initialize(Engine * engine)
{
	// Default Parameters
	m_engine = engine;

	// Asset Loading
	m_engine->createAsset(m_numberTexture, std::string("numbers.png"), GL_TEXTURE_2D, false, false, true);
	m_engine->createAsset(m_shapeQuad, std::string("quad"), true);
	m_engine->createAsset(m_shader, std::string("Utilities\\numberPrint"), true);

	// Primitive Construction
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_shapeQuad->addCallback(this, [&]() mutable {
		m_quadVAOLoaded = true;
		m_shapeQuad->updateVAO(m_quadVAO);
	});

	// Preference Callbacks
	m_renderSize.x = m_engine->addPrefCallback(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) { resize(glm::vec2(f, m_renderSize.y)); });
	m_renderSize.y = m_engine->addPrefCallback(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) { resize(glm::vec2(m_renderSize.x, f)); });
	resize(m_renderSize);
}

void System_PerfCounter::update(const float & deltaTime)
{
	if (m_quadVAOLoaded && m_shader->existsYet() && m_numberTexture->existsYet()) {
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glViewport(0, 0, m_renderSize.x, m_renderSize.y);
		glBindVertexArray(m_quadVAO);
		m_shader->bind();
		m_shader->Set_Uniform(1, m_projMatrix);
		m_numberTexture->bind(0);
		const glm::mat4 scale = glm::translate(glm::mat4(1.0f), glm::vec3(12, 12, 0)) * glm::scale(glm::mat4(1.0f), glm::vec3(12));

		
		float dt_seconds = deltaTime * 1000;
		std::string test = std::to_string(dt_seconds);
		bool foundDecimal = false;
		int decimalCount = 0;
		bool repeat = true;
		for (int x = 0, length = test.size(); x < length && repeat; ++x) {
			if (foundDecimal)
				decimalCount++;
			if (decimalCount >= 2)
				repeat = false;
			const int number = test[x];
			if (number == 46) {
				foundDecimal = true;
				m_shader->Set_Uniform(3, 10); // set texture index to 11
			}
			else 
				m_shader->Set_Uniform(3, number - 48); // remove the ascii encoding, convert to int		

			m_shader->Set_Uniform(2, glm::translate(glm::mat4(1.0f), glm::vec3(x * 24, 24, 0)) * scale);
			glDrawArrays(GL_TRIANGLES, 0, 6);		
		}
		m_shader->Release();
	}
}

void System_PerfCounter::resize(const glm::ivec2 s)
{
	m_renderSize = s;
	m_projMatrix = glm::ortho(0.0f, (float)s.x, 0.0f, (float)s.y);
}
