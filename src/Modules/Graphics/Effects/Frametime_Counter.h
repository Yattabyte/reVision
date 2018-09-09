#pragma once
#ifndef FRAMETIME_COUNTER_H
#define FRAMETIME_COUNTER_H

#include "Modules\Graphics\Effects\Effect_Base.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Assets\Asset_Texture.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Utilities\GL\FBO.h"
#include "Engine.h"


/** A post-processing technique for writing the frame time to the screen. */
class Frametime_Counter : public Effect_Base {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~Frametime_Counter() {
		m_engine->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
		m_engine->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);
		m_numberTexture->removeCallback(this);
		m_shader->removeCallback(this);
	}
	/** Constructor. */
	Frametime_Counter(Engine * engine)
	: m_engine(engine) {
		// Asset Loading
		m_numberTexture = Asset_Texture::Create(m_engine, "numbers.png", GL_TEXTURE_2D, false, false);
		m_shader = Asset_Shader::Create(m_engine, "Utilities\\numberPrint");
		m_shapeQuad = Asset_Primitive::Create(m_engine, "quad");

		// Preference Callbacks
		m_renderSize.x = m_engine->addPrefCallback<int>(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) { resize(glm::vec2(f, m_renderSize.y)); });
		m_renderSize.y = m_engine->addPrefCallback<int>(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) { resize(glm::vec2(m_renderSize.x, f)); });
		resize(m_renderSize);

		m_numberTexture->addCallback(this, [&] {
			glMakeTextureHandleResidentARB(m_numberTexture->m_glTexHandle);
			if (m_shader->existsYet())
				m_shader->setUniform(0, m_numberTexture->m_glTexHandle);
		});
		m_shader->addCallback(this, [&] {
			if (m_numberTexture->existsYet())
				m_shader->setUniform(0, m_numberTexture->m_glTexHandle);
		});
	}


	// Interface Implementations.
	virtual void applyEffect(const float & deltaTime) override {
		if (!m_shapeQuad->existsYet() || !m_shader->existsYet() || !m_numberTexture->existsYet())
			return;
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glViewport(0, 0, m_renderSize.x, m_renderSize.y);
		glBindVertexArray(m_shapeQuad->m_vaoID);
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
	void resize(const glm::ivec2 &s) {
		m_renderSize = s;
		m_projMatrix = glm::ortho(0.0f, (float)s.x, 0.0f, (float)s.y);
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Asset_Shader m_shader;
	Shared_Asset_Texture m_numberTexture;
	Shared_Asset_Primitive m_shapeQuad;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	glm::mat4 m_projMatrix = glm::mat4(1.0f);
};

#endif // FRAMETIME_COUNTER_H