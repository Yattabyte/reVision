#pragma once
#ifndef BUTTON_H
#define BUTTON_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Label.h"
#include "Assets/Primitive.h"
#include "Assets/Shader.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Engine.h"
#include <memory>


/** UI button class, affords being pushed and released. */
class Button : public UI_Element
{
public:
	// (de)Constructors
	~Button() {
		// Update indicator
		m_aliveIndicator = false;
	}
	Button(Engine * engine) {
		// Asset Loading
		m_shader = Shared_Shader(engine, "UI\\Button");

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

		// All buttons have labels, but it isn't interactive
		m_label = std::make_shared<Label>(engine);
		m_label->setText("");

		// Callbacks
		addCallback(on_resize, [&]() {m_label->setScale(getScale()); });
		addCallback(on_mouse_press, [&]() {m_pressed = true; });
		addCallback(on_mouse_release, [&]() {m_pressed = false; });
		addCallback(on_mouse_enter, [&]() {m_highlighted = true; });
		addCallback(on_mouse_exit, [&]() {m_highlighted = false; });

		// Generate vertex array
		glCreateVertexArrays(1, &m_vaoID);
		glEnableVertexArrayAttrib(m_vaoID, 0);
		glVertexArrayAttribBinding(m_vaoID, 0, 0);
		glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glCreateBuffers(1, &m_vboID);
		glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(glm::vec3));
		constexpr auto num_tri = (5 * 2) + (4 * 5);
		constexpr auto num_data = num_tri * 3;
		glNamedBufferStorage(m_vboID, num_data * sizeof(glm::vec3), 0, GL_DYNAMIC_STORAGE_BIT);
		const GLuint quad[4] = { (GLuint)num_data, 1, 0, 0 };
		m_indirect = StaticBuffer(sizeof(GLuint) * 4, quad, GL_CLIENT_STORAGE_BIT);
	}


	// Interface Implementation
	virtual void update() override {
		constexpr auto num_tri = (5 * 2) + (4 * 5);
		constexpr auto num_data = num_tri * 3;
		std::vector<glm::vec3> m_data(num_data);
		m_data[0] = { -1, -1, 0 };
		m_data[1] = {  1, -1, 0 };
		m_data[2] = {  1,  1, 0 };
		m_data[3] = {  1,  1, 0 };
		m_data[4] = { -1,  1, 0 };
		m_data[5] = { -1, -1, 0 };

		for (int x = 0; x < 6; ++x)
			m_data[x] *= glm::vec3(m_scale - m_bevelRadius, 0.0f);

		m_data[6] = { -m_scale.x + m_bevelRadius, -m_scale.y, 0 };
		m_data[7] = {  m_scale.x - m_bevelRadius, -m_scale.y, 0 };
		m_data[8] = {  m_scale.x - m_bevelRadius, -m_scale.y + m_bevelRadius, 0 };
		m_data[9] = {  m_scale.x - m_bevelRadius, -m_scale.y + m_bevelRadius, 0 };
		m_data[10] = {  -m_scale.x + m_bevelRadius, -m_scale.y + m_bevelRadius, 0 };
		m_data[11] = { -m_scale.x + m_bevelRadius, -m_scale.y, 0 };

		m_data[12] = { -m_scale.x, -m_scale.y + m_bevelRadius, 0 };
		m_data[13] = { -m_scale.x + m_bevelRadius, -m_scale.y + m_bevelRadius, 0 };
		m_data[14] = { -m_scale.x + m_bevelRadius, m_scale.y - m_bevelRadius, 0 };
		m_data[15] = { -m_scale.x + m_bevelRadius, m_scale.y - m_bevelRadius, 0 };
		m_data[16] = { -m_scale.x, m_scale.y - m_bevelRadius, 0 };
		m_data[17] = { -m_scale.x, -m_scale.y + m_bevelRadius, 0 };

		m_data[18] = { -m_scale.x + m_bevelRadius, m_scale.y - m_bevelRadius, 0 };
		m_data[19] = { m_scale.x - m_bevelRadius, m_scale.y - m_bevelRadius, 0 };
		m_data[20] = { m_scale.x - m_bevelRadius, m_scale.y, 0 };
		m_data[21] = { m_scale.x - m_bevelRadius, m_scale.y, 0 };
		m_data[22] = { -m_scale.x + m_bevelRadius, m_scale.y, 0 };
		m_data[23] = { -m_scale.x + m_bevelRadius, m_scale.y - m_bevelRadius, 0 };

		m_data[24] = { m_scale.x - m_bevelRadius, -m_scale.y + m_bevelRadius, 0 };
		m_data[25] = { m_scale.x, -m_scale.y + m_bevelRadius, 0 };
		m_data[26] = { m_scale.x, m_scale.y - m_bevelRadius, 0 };
		m_data[27] = { m_scale.x, m_scale.y - m_bevelRadius, 0 };
		m_data[28] = { m_scale.x - m_bevelRadius, m_scale.y - m_bevelRadius, 0 };
		m_data[29] = { m_scale.x - m_bevelRadius, -m_scale.y + m_bevelRadius, 0 };

		glm::vec3 circlePoints[20];
		for (int x = 0; x < 20; ++x)
			circlePoints[x] = m_bevelRadius * glm::vec3(
				cosf((float(x) / 10.0f) * glm::pi<float>()),
				sinf((float(x) / 10.0f) * glm::pi<float>()),
				0
			);
		m_data[30] = { m_scale.x - m_bevelRadius, m_scale.y - m_bevelRadius, 0 };
		m_data[31] = circlePoints[0] + m_data[30];
		m_data[32] = circlePoints[1] + m_data[30];
		m_data[33] = m_data[30];
		m_data[34] = circlePoints[1] + m_data[30];
		m_data[35] = circlePoints[2] + m_data[30];
		m_data[36] = m_data[30];
		m_data[37] = circlePoints[2] + m_data[30];
		m_data[38] = circlePoints[3] + m_data[30];
		m_data[39] = m_data[30];
		m_data[40] = circlePoints[3] + m_data[30];
		m_data[41] = circlePoints[4] + m_data[30];
		m_data[42] = m_data[30];
		m_data[43] = circlePoints[4] + m_data[30];
		m_data[44] = circlePoints[5] + m_data[30];

		m_data[45] = { -m_scale.x + m_bevelRadius, m_scale.y - m_bevelRadius, 0 };
		m_data[46] = circlePoints[5] + m_data[45];
		m_data[47] = circlePoints[6] + m_data[45];
		m_data[48] = m_data[45];
		m_data[49] = circlePoints[6] + m_data[45];
		m_data[50] = circlePoints[7] + m_data[45];
		m_data[51] = m_data[45];
		m_data[52] = circlePoints[7] + m_data[45];
		m_data[53] = circlePoints[8] + m_data[45];
		m_data[54] = m_data[45];
		m_data[55] = circlePoints[8] + m_data[45];
		m_data[56] = circlePoints[9] + m_data[45];
		m_data[57] = m_data[45];
		m_data[58] = circlePoints[9] + m_data[45];
		m_data[59] = circlePoints[10] + m_data[45];

		m_data[60] = { -m_scale.x + m_bevelRadius, -m_scale.y + m_bevelRadius, 0 };
		m_data[61] = circlePoints[10] + m_data[60];
		m_data[62] = circlePoints[11] + m_data[60];
		m_data[63] = m_data[60];
		m_data[64] = circlePoints[11] + m_data[60];
		m_data[65] = circlePoints[12] + m_data[60];
		m_data[66] = m_data[60];
		m_data[67] = circlePoints[12] + m_data[60];
		m_data[68] = circlePoints[13] + m_data[60];
		m_data[69] = m_data[60];
		m_data[70] = circlePoints[13] + m_data[60];
		m_data[71] = circlePoints[14] + m_data[60];
		m_data[72] = m_data[60];
		m_data[73] = circlePoints[14] + m_data[60];
		m_data[74] = circlePoints[15] + m_data[60];

		m_data[75] = { m_scale.x - m_bevelRadius, -m_scale.y + m_bevelRadius, 0 };
		m_data[76] = circlePoints[15] + m_data[75];
		m_data[77] = circlePoints[16] + m_data[75];
		m_data[78] = m_data[75];
		m_data[79] = circlePoints[16] + m_data[75];
		m_data[80] = circlePoints[17] + m_data[75];
		m_data[81] = m_data[75];
		m_data[82] = circlePoints[17] + m_data[75];
		m_data[83] = circlePoints[18] + m_data[75];
		m_data[84] = m_data[75];
		m_data[85] = circlePoints[18] + m_data[75];
		m_data[86] = circlePoints[19] + m_data[75];
		m_data[87] = m_data[75];
		m_data[88] = circlePoints[19] + m_data[75];
		m_data[89] = circlePoints[0] + m_data[75];		
		
		glNamedBufferSubData(m_vboID, 0, num_data * sizeof(glm::vec3), &m_data[0]);

		UI_Element::update();
	}
	virtual void renderElement(const glm::vec2 & position, const glm::vec2 & scale) override {
		if (!getVisible()) return;
		const auto newPosition = position + m_position;
		const auto newScale = glm::min(m_scale, scale);
		if (m_shader->existsYet()) {
			m_shader->bind();
			m_shader->setUniform(0, m_orthoProj);
			m_shader->setUniform(1, newPosition);
			m_shader->setUniform(2, newScale);
			m_shader->setUniform(3, m_highlighted);
			m_shader->setUniform(4, m_pressed);
			glBindVertexArray(m_vaoID);
			m_indirect.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
		}
		m_label->renderElement(newPosition, newScale);
		UI_Element::renderElement(position, newScale);
	}


	// Public Methods
	/** Set this label element's text.
	@param	text	the text to use. */
	void setText(const std::string & text) {
		m_label->setText(text);
		update();
	}
	/** Set the bevel radius for this button.
	@param radius	the new radius to use. */
	void setBevelRadius(const float & radius) {
		m_bevelRadius = radius;
	}


protected:
	// Protected Attributes
	std::shared_ptr<Label> m_label;
	bool m_highlighted = false, m_pressed = false;
	float m_bevelRadius = 5.0f;


private:
	// Private Attributes
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	glm::ivec2 m_renderSize = glm::ivec2(1);
	glm::mat4 m_orthoProj = glm::mat4(1.0f);
	GLuint m_vaoID = 0, m_vboID = 0;
	Shared_Shader m_shader;
	StaticBuffer m_indirect;
};

#endif // BUTTON_H