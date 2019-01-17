#pragma once
#ifndef LABEL_H
#define LABEL_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Assets/Primitive.h"
#include "Assets/Shader.h"
#include "Assets/Texture.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Engine.h"
#include <string>


/** UI text label class, affords displaying text on the screen. */
class Label : public UI_Element
{
public:
	~Label() {
		// Update indicator
		m_aliveIndicator = false;
	}
	Label(Engine * engine) {
		// Asset Loading
		m_shader = Shared_Shader(engine, "UI\\Label");
		m_shapeQuad = Shared_Primitive(engine, "quad");
		m_textureFont = Shared_Texture(engine, "font.tga", GL_TEXTURE_2D, true, true);

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			if (!m_aliveIndicator) return;
			const GLuint count = (GLuint)m_text.size();
			const GLuint quad[4] = { (GLuint)m_shapeQuad->getSize(), count, 0, 0 };
			m_indirect = StaticBuffer(sizeof(GLuint) * 4, quad, GL_DYNAMIC_STORAGE_BIT);
		});

		// Preferences
		auto & preferences = engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
		preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
		constexpr static auto calcOthoProj = [](const glm::ivec2 & renderSize) {
			return glm::ortho<float>(0.0f, renderSize.x, 0.0f, renderSize.y, -1.0f, 1.0f);
		};
		preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) {
			m_renderSize.x = f;
			m_orthoProj = calcOthoProj(m_renderSize);
		});
		preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {
			m_renderSize.y = f;
			m_orthoProj = calcOthoProj(m_renderSize);
		});
		m_orthoProj = calcOthoProj(m_renderSize);

		setText("Label");
	}


	// Interface Implementation
	virtual void update() override {
		// Write letters to a buffer
		const GLuint count = (GLuint)m_text.size();
		std::vector<int> data(count + 1);
		data[0] = count;
		for (int x = 0; x < (int)count; ++x)
			data[x + 1] = (int)(m_text[x]) - 32;
		m_bufferString.write(0, sizeof(int)*(count + 1), data.data());
		if (m_shapeQuad->existsYet())
			m_indirect.write(GLsizeiptr(sizeof(GLuint)), GLsizeiptr(sizeof(GLuint)), &count);

		UI_Element::update();
	}
	virtual void renderElement(const glm::vec2 & position, const glm::vec2 & scale) override {
		if (!getVisible()) return;
		const auto newPosition = position + m_position;
		const auto newScale = glm::min(glm::vec2(m_scale.x, 15.0f), scale);
		if (m_shader->existsYet() && m_shapeQuad->existsYet() && m_textureFont->existsYet()) {
			m_shader->bind();
			m_shader->setUniform(0, m_orthoProj);
			m_shader->setUniform(1, newPosition);
			m_shader->setUniform(2, newScale);
			m_textureFont->bind(0);
			glBindVertexArray(m_shapeQuad->m_vaoID);
			m_indirect.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			m_bufferString.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
		}
		UI_Element::renderElement(position, newScale);
	}


	// Public Methods
	/** Set this label element's text. 
	@param	text	the text to use. */
	void setText(const std::string & text) {
		m_text = text;
		update();
	}


protected:
	// Protected Attributes
	std::string m_text;


private:
	// Private Attributes
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	glm::ivec2 m_renderSize = glm::ivec2(1);
	glm::mat4 m_orthoProj = glm::mat4(1.0f);
	Shared_Shader m_shader;
	Shared_Primitive m_shapeQuad;
	Shared_Texture m_textureFont;
	StaticBuffer m_indirect;
	DynamicBuffer m_bufferString;
};

#endif // PANEL_H