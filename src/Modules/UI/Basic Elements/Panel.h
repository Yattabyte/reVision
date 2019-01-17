#pragma once
#ifndef PANEL_H
#define PANEL_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Assets/Primitive.h"
#include "Assets/Shader.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Engine.h"


/** UI panel class, affords containing other elements only. */
class Panel : public UI_Element
{
public:
	~Panel() {
		// Update indicator
		m_aliveIndicator = false;
	}
	Panel(Engine * engine) {
		// Asset Loading
		m_shader = Shared_Shader(engine, "UI\\Panel");
		m_shapeQuad = Shared_Primitive(engine, "quad");

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			if (!m_aliveIndicator) return;
			const GLuint quadSize = (GLuint)m_shapeQuad->getSize();
			const GLuint quad[4] = { quadSize, 1, 0, 0 };
			m_indirect = StaticBuffer(sizeof(GLuint) * 4, quad, GL_CLIENT_STORAGE_BIT);
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
	}
	

	// Interface Implementation
	virtual void renderElement(const glm::vec2 & position, const glm::vec2 & scale) override {
		if (!getVisible()) return;
		const auto newPosition = position + m_position;
		const auto newScale = glm::min(m_scale, scale);
		if (m_shader->existsYet() && m_shapeQuad->existsYet()) {
			m_shader->bind();
			m_shader->setUniform(0, m_orthoProj);
			m_shader->setUniform(1, newPosition);
			m_shader->setUniform(2, newScale);
			glBindVertexArray(m_shapeQuad->m_vaoID);
			m_indirect.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
		}
		UI_Element::renderElement(position, newScale);
	}


private:
	// Private Attributes
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	glm::ivec2 m_renderSize = glm::ivec2(1);
	glm::mat4 m_orthoProj = glm::mat4(1.0f);
	Shared_Shader m_shader;
	Shared_Primitive m_shapeQuad;
	StaticBuffer m_indirect;
};

#endif // PANEL_H