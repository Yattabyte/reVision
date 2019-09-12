#pragma once
#ifndef FRAMETIME_COUNTER_H
#define FRAMETIME_COUNTER_H

#include "Modules/Game/Overlays/Overlay.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Assets/Texture.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Engine.h"


/** A post-processing technique for writing the frame time to the screen. */
class Frametime_Counter : public Overlay {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline ~Frametime_Counter() {
		// Update indicator
		*m_aliveIndicator = false;
	}
	/** Constructor. */
	inline Frametime_Counter(Engine * engine)
		: m_engine(engine) {
		// Asset Loading
		m_numberTexture = Shared_Texture(engine, "numbers.png", GL_TEXTURE_2D, false, false);
		m_shader = Shared_Shader(engine, "Utilities\\numberPrint");
		m_shapeQuad = Shared_Auto_Model(engine, "quad");

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
		preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
		preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) { resize(glm::vec2(f, m_renderSize.y)); });
		preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) { resize(glm::vec2(m_renderSize.x, f)); });		
		resize(m_renderSize);
	}


	// Public Interface Implementations.
	inline virtual void applyEffect(const float & deltaTime) override {
		if (!m_shapeQuad->existsYet() || !m_shader->existsYet() || !m_numberTexture->existsYet())
			return;
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glViewport(0, 0, m_renderSize.x, m_renderSize.y);
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_numberTexture->bind(0);
		m_shader->bind();
		m_shader->setUniform(1, m_projMatrix);
		const glm::mat4 scale = glm::translate(glm::mat4(1.0f), glm::vec3(12, 12, 0)) * glm::scale(glm::mat4(1.0f), glm::vec3(12));
		
		float dt_seconds = deltaTime * 1000;
		std::string test = std::to_string(dt_seconds);
		bool foundDecimal = false;
		int decimalCount = 0;
		bool repeat = true;
		for (size_t x = 0, length = test.size(); x < length && repeat; ++x) {
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
				m_shader->setUniform(3, number - 48); // remove the ascii encoding, convert to int		

			m_shader->setUniform(2, glm::translate(glm::mat4(1.0f), glm::vec3(x * 24, 24, 0)) * scale);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		m_shader->Release();		
		glDisable(GL_BLEND);
	}


private:
	// Private Methods
	inline void resize(const glm::ivec2 &s) {
		m_renderSize = s;
		m_projMatrix = glm::ortho(0.0f, (float)s.x, 0.0f, (float)s.y);
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Shader m_shader;
	Shared_Texture m_numberTexture;
	Shared_Auto_Model m_shapeQuad;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	glm::mat4 m_projMatrix = glm::mat4(1.0f);
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // FRAMETIME_COUNTER_H