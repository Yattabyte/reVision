#pragma once
#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Label.h"
#include "Assets/Shader.h"
#include "Utilities/GL/StaticBuffer.h"
#include <memory>


/** UI button class, affords being pushed and released. */
class Button : public UI_Element
{
public:
	// (de)Constructors
	~Button() {
		// Delete geometry
		glDeleteBuffers(1, &m_vboID);
		glDeleteVertexArrays(1, &m_vaoID);
	}
	Button(Engine * engine, const std::string & text = "Button") {
		// Asset Loading
		m_shader = Shared_Shader(engine, "UI\\Button");

		// All buttons have labels, but it isn't interactive
		m_label = std::make_shared<Label>(engine, text);
		m_label->setAlignment(Label::align_center);
		addElement(m_label);

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
		constexpr auto num_tri = (5 * 2) + (4 * 10);
		constexpr auto num_data = num_tri * 3;
		glNamedBufferStorage(m_vboID, num_data * sizeof(glm::vec3), 0, GL_DYNAMIC_STORAGE_BIT);
		const GLuint quad[4] = { (GLuint)num_data, 1, 0, 0 };
		m_indirect = StaticBuffer(sizeof(GLuint) * 4, quad, GL_CLIENT_STORAGE_BIT);
	}


	// Interface Implementation
	virtual void update() override {
		constexpr auto num_tri = (5 * 2) + (4 * 10);
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

		glm::vec3 circlePoints[40];
		for (int x = 0; x < 40; ++x)
			circlePoints[x] = m_bevelRadius * glm::vec3(
				cosf((float(x) / 20.0f) * glm::pi<float>()),
				sinf((float(x) / 20.0f) * glm::pi<float>()),
				0
			);
		glm::vec3 centers[4] = { 
			{ m_scale.x - m_bevelRadius, m_scale.y - m_bevelRadius, 0 },
			{ -m_scale.x + m_bevelRadius, m_scale.y - m_bevelRadius, 0 },
			{ -m_scale.x + m_bevelRadius, -m_scale.y + m_bevelRadius, 0 },
			{ m_scale.x - m_bevelRadius, -m_scale.y + m_bevelRadius, 0 }
		};
		int cIndices[40];
		for (int a = 0, b = 0, c = 0; a < 40; ++a, ++b) {
			if (b == 10) {
				b = 0;
				c++;
			}
			cIndices[a] = c;
		}
		for (int cIndex = 0, cpx = 0, dx = 30; dx < num_data; cpx++, dx += 3) {
			const auto center = centers[cIndices[cpx]];
			m_data[dx] = center;
			m_data[dx + 1] = circlePoints[(cpx + 0) % 40] + center;
			m_data[dx + 2] = circlePoints[(cpx + 1) % 40] + center;
		}		
		glNamedBufferSubData(m_vboID, 0, num_data * sizeof(glm::vec3), &m_data[0]);

		UI_Element::update();
	}
	virtual void renderElement(const float & deltaTime, const glm::vec2 & position, const glm::vec2 & scale) override {
		if (!getVisible()) return;
		const auto newPosition = position + m_position;
		const auto newScale = glm::min(m_scale, scale);
		if (m_shader->existsYet()) {
			m_shader->bind();
			m_shader->setUniform(0, glm::vec3(newPosition, m_depth));
			m_shader->setUniform(1, m_enabled);
			m_shader->setUniform(2, m_highlighted);
			m_shader->setUniform(3, m_pressed);
			glBindVertexArray(m_vaoID);
			m_indirect.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
		}
		UI_Element::renderElement(deltaTime, position, newScale);
	}


	// Public Methods
	/** Set this label element's text.
	@param	text	the text to use. */
	void setText(const std::string & text) {
		m_label->setText(text);
		update();
	}
	/** Retrieve this buttons' labels text.
	@return	the text this label uses. */
	std::string getText() const {
		return m_label->getText();
	}
	/** Set the bevel radius for this button.
	@param radius	the new radius to use. */
	void setBevelRadius(const float & radius) {
		m_bevelRadius = radius;
	}
	/** Get the bevel radius from this button.
	@return radius	this buttons' bevel radius. */
	float getBevelRadius() const {
		return m_bevelRadius;
	}
	/** Get if this button is pressed. */
	bool getPressed() const {
		return m_pressed;
	}


protected:
	// Protected Attributes
	bool 
		m_highlighted = false, 
		m_pressed = false;
	float m_bevelRadius = 10.0f;


private:
	// Private Attributes
	std::shared_ptr<Label> m_label;
	GLuint 
		m_vaoID = 0, 
		m_vboID = 0;
	Shared_Shader m_shader;
	StaticBuffer m_indirect;
};

#endif // UI_BUTTON_H