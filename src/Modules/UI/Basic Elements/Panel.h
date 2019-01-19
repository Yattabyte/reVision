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

		// Generate vertex array
		glCreateVertexArrays(1, &m_vaoID);
		glEnableVertexArrayAttrib(m_vaoID, 0);
		glEnableVertexArrayAttrib(m_vaoID, 1);
		glVertexArrayAttribBinding(m_vaoID, 0, 0);
		glVertexArrayAttribBinding(m_vaoID, 1, 1);
		glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribIFormat(m_vaoID, 1, 1, GL_INT, 0);
		glCreateBuffers(2, m_vboID);
		glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID[0], 0, sizeof(glm::vec3));
		glVertexArrayVertexBuffer(m_vaoID, 1, m_vboID[1], 0, sizeof(int));
		constexpr auto num_tri = (5 * 2);
		constexpr auto num_data = num_tri * 3;
		glNamedBufferStorage(m_vboID[0], num_data * sizeof(glm::vec3), 0, GL_DYNAMIC_STORAGE_BIT);
		glNamedBufferStorage(m_vboID[1], num_data * sizeof(int), 0, GL_DYNAMIC_STORAGE_BIT);
		const GLuint quad[4] = { (GLuint)num_data, 1, 0, 0 };
		m_indirect = StaticBuffer(sizeof(GLuint) * 4, quad, GL_CLIENT_STORAGE_BIT);
	}
	

	// Interface Implementation
	virtual void update() override {
		constexpr auto num_tri = (5 * 2);
		constexpr auto num_data = num_tri * 3;
		std::vector<glm::vec3> m_data(num_data);
		std::vector<int> m_objIndices(num_data);

		// Center
		m_data[0] = { -1, -1, 0 };
		m_data[1] = { 1, -1, 0 };
		m_data[2] = { 1,  1, 0 };
		m_data[3] = { 1,  1, 0 };
		m_data[4] = { -1,  1, 0 };
		m_data[5] = { -1, -1, 0 };
		for (int x = 0; x < 6; ++x)
			m_data[x] *= glm::vec3(m_scale - m_borderSize, 0.0f);
		for (int x = 0; x < 6; ++x)
			m_objIndices[x] = 0;

		// Bottom Bar
		m_data[6] = { -m_scale.x, -m_scale.y, 0 };
		m_data[7] = { m_scale.x, -m_scale.y, 0 };
		m_data[8] = { m_scale.x, -m_scale.y + m_borderSize, 0 };
		m_data[9] = { m_scale.x, -m_scale.y + m_borderSize, 0 };
		m_data[10] = { -m_scale.x, -m_scale.y + m_borderSize, 0 };
		m_data[11] = { -m_scale.x, -m_scale.y, 0 };

		// Left Bar
		m_data[12] = { -m_scale.x, -m_scale.y + m_borderSize, 0 };
		m_data[13] = { -m_scale.x + m_borderSize, -m_scale.y + m_borderSize, 0 };
		m_data[14] = { -m_scale.x + m_borderSize, m_scale.y - m_borderSize, 0 };
		m_data[15] = { -m_scale.x + m_borderSize, m_scale.y - m_borderSize, 0 };
		m_data[16] = { -m_scale.x, m_scale.y - m_borderSize, 0 };
		m_data[17] = { -m_scale.x, -m_scale.y + m_borderSize, 0 };

		// Top Bar
		m_data[18] = { -m_scale.x, m_scale.y - m_borderSize, 0 };
		m_data[19] = { m_scale.x, m_scale.y - m_borderSize, 0 };
		m_data[20] = { m_scale.x, m_scale.y, 0 };
		m_data[21] = { m_scale.x, m_scale.y, 0 };
		m_data[22] = { -m_scale.x, m_scale.y, 0 };
		m_data[23] = { -m_scale.x, m_scale.y - m_borderSize, 0 };

		// Right Bar
		m_data[24] = { m_scale.x - m_borderSize, -m_scale.y + m_borderSize, 0 };
		m_data[25] = { m_scale.x, -m_scale.y + m_borderSize, 0 };
		m_data[26] = { m_scale.x, m_scale.y - m_borderSize, 0 };
		m_data[27] = { m_scale.x, m_scale.y - m_borderSize, 0 };
		m_data[28] = { m_scale.x - m_borderSize, m_scale.y - m_borderSize, 0 };
		m_data[29] = { m_scale.x - m_borderSize, -m_scale.y + m_borderSize, 0 };
		for (int x = 6; x < 30; ++x)
			m_objIndices[x] = 1;

		glNamedBufferSubData(m_vboID[0], 0, num_data * sizeof(glm::vec3), &m_data[0]);
		glNamedBufferSubData(m_vboID[1], 0, num_data * sizeof(int), &m_objIndices[0]);

		UI_Element::update();
	}
	virtual void renderElement(const float & deltaTime, const glm::vec2 & position, const glm::vec2 & scale) override {
		if (!getVisible()) return;
		const auto newPosition = position + m_position;
		const auto newScale = glm::min(m_scale, scale);
		if (m_shader->existsYet()) {
			m_shader->bind();
			m_shader->setUniform(0, m_orthoProj);
			m_shader->setUniform(1, newPosition);
			glm::vec3 colors[2] = { UIColor_Background, UIColor_Border };
			m_shader->setUniformArray(3, colors, 2);
			glBindVertexArray(m_vaoID);
			m_indirect.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
		}
		UI_Element::renderElement(deltaTime, position, newScale);
	}


	// Public Methods
	/** Set the border size for this panel.
	@param		size		the new border size to use. */
	void setBorderSize(const float & size) {
		m_borderSize = size;
	}
	/** Get the border size from this panel.
	@return		radius		this panels' border size. */
	float getBorderSize() const {
		return m_borderSize;
	}


protected:
	// Protected Attributes
	float m_borderSize = 1.0f;


private:
	// Private Attributes
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	glm::ivec2 m_renderSize = glm::ivec2(1);
	glm::mat4 m_orthoProj = glm::mat4(1.0f);
	GLuint m_vaoID = 0, m_vboID[2] = { 0, 0 };
	Shared_Shader m_shader;
	StaticBuffer m_indirect;
};

#endif // PANEL_H