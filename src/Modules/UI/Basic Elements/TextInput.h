#pragma once
#ifndef UI_TEXTINPUT_H
#define UI_TEXTINPUT_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Label.h"
#include "Assets/Shader.h"
#include "Utilities/GL/StaticBuffer.h"
#include "GLFW/glfw3.h"
#include <string>


/** UI Text input element. */
class TextInput : public UI_Element
{
public:
	// Interaction enums
	enum interact {
		on_text_change = UI_Element::last_interact_index
	};


	// (de)Constructors
	inline ~TextInput() {
		// Delete geometry
		glDeleteBuffers(2, m_vboID);
		glDeleteVertexArrays(1, &m_vaoID);
	}
	inline TextInput(Engine * engine) {
		// Asset Loading
		m_shader = Shared_Shader(engine, "UI\\TextInput");

		// Label
		m_label = std::make_shared<Label>(engine);
		m_label->setAlignment(Label::align_left);
		m_label->setColor(glm::vec3(0.0f));
		addElement(m_label);

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
		constexpr auto num_data = 4 * 3;
		glNamedBufferStorage(m_vboID[0], num_data * sizeof(glm::vec3), 0, GL_DYNAMIC_STORAGE_BIT);
		glNamedBufferStorage(m_vboID[1], num_data * sizeof(int), 0, GL_DYNAMIC_STORAGE_BIT);
		const GLuint quad[4] = { (GLuint)num_data, 1, 0, 0 };
		m_indirect = StaticBuffer(sizeof(GLuint) * 4, quad, GL_CLIENT_STORAGE_BIT);
	}


	// Interface Implementation
	inline virtual void setScale(const glm::vec2 & scale) override {
		m_label->setScale(scale);
		UI_Element::setScale(scale);
	}
	inline virtual void update() override {
		constexpr auto num_data = 4 * 3;
		std::vector<glm::vec3> data(num_data);
		std::vector<int> objIndices(num_data);

		for (int x = 0; x < 12; x += 6) {
			data[x + 0] = { -1, -1, 0 };
			data[x + 1] = { 1, -1, 0 };
			data[x + 2] = { 1,  1, 0 };
			data[x + 3] = { 1,  1, 0 };
			data[x + 4] = { -1,  1, 0 };
			data[x + 5] = { -1, -1, 0 };
		}
		for (int x = 0; x < 6; ++x) {
			data[x] *= glm::vec3(m_scale, 0.0f);
			objIndices[x] = 0;
		}
		for (int x = 6; x < 12; ++x) {
			data[x] *= glm::vec3(1.0, 10, 1);
			data[x].x = (data[x].x - m_scale.x) + (10.0f * m_caretIndex);
			objIndices[x] = 1;
		}

		glNamedBufferSubData(m_vboID[0], 0, num_data * sizeof(glm::vec3), &data[0]);
		glNamedBufferSubData(m_vboID[1], 0, num_data * sizeof(int), &objIndices[0]);

		UI_Element::update();
	}
	inline virtual void mouseAction(const MouseEvent & mouseEvent) {
		if (getVisible() && getEnabled()) {
			if (mouseWithin(mouseEvent)) {
				if (!m_pressed && mouseEvent.m_action == MouseEvent::PRESS) {
					m_pressed = true;
					enactCallback(on_mouse_press);
					return;
				}
				else if (m_pressed && mouseEvent.m_action == MouseEvent::RELEASE) {
					m_pressed = false;
					// If already editing, move caret to mouse position
					if (m_edit) {
						const int mx = int(float(mouseEvent.m_xPos) - m_position.x + m_scale.x);
						setCaret((int)std::roundf(float(mx) / 10.0f));
					}
					m_edit = true;
					return;
				}
			}
			m_edit = false;
			UI_Element::mouseAction(mouseEvent);
		}
	}				
	inline virtual void keyChar(const unsigned int & character) override {
		if (m_edit) {
			setText(m_text.substr(0, m_caretIndex) + char(character) + m_text.substr(m_caretIndex, m_text.size()));
			setCaret(m_caretIndex + 1);
			enactCallback(on_text_change);
		}
	}
	inline virtual void keyboardAction(const int & key, const int & scancode, const int & action, const int & mods) override {
		if (m_edit) {
			if (key == GLFW_KEY_ENTER || key == GLFW_KEY_ESCAPE)
				m_edit = false;
			if (action == GLFW_PRESS) {
				if (key == GLFW_KEY_BACKSPACE) {
					if (m_caretIndex > 0) {
						setText(m_text.substr(0, m_caretIndex - 1) + m_text.substr(m_caretIndex, m_text.size()));
						setCaret(m_caretIndex - 1);
						enactCallback(on_text_change);
					}
				}
				else if (key == GLFW_KEY_DELETE) {
					if (m_caretIndex + 1 <= m_text.size()) {
						setText(m_text.substr(0, m_caretIndex) + m_text.substr(m_caretIndex + 1, m_text.size()));
						enactCallback(on_text_change);
					}
				}
				else if (key == GLFW_KEY_LEFT)
					setCaret(m_caretIndex - 1);
				else if (key == GLFW_KEY_RIGHT)
					setCaret(m_caretIndex + 1);
			}
		}
	}
	inline virtual void renderElement(const float & deltaTime, const glm::vec2 & position, const glm::vec2 & scale) override {
		m_blinkTime += deltaTime;
		if (!getVisible()) return;
		const glm::vec2 newPosition = position + m_position;
		const glm::vec2 newScale = glm::min(m_scale, scale);
		if (m_shader->existsYet()) {
			// Render Background
			m_shader->bind();
			m_shader->setUniform(0, newPosition);
			m_shader->setUniform(1, m_enabled);
			m_shader->setUniform(2, m_edit);
			m_shader->setUniform(3, m_blinkTime);
			glBindVertexArray(m_vaoID);
			m_indirect.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
		}
		// Render Text
		UI_Element::renderElement(deltaTime, position, newScale);
	}


	// Public Methods
	/** Set the text to display in this field.
	@param		string		the new text to display. */
	inline void setText(const std::string & text) {
		m_text = text;
		m_label->setText(text);
		update();
	}
	/** Get the text displayed in this field.
	@return					the text displayed in this field. */
	inline std::string getText() const {
		return m_text;
	}
	/** Set this field's alignment.
	@param	text			the alignment (left, center, right). */
	inline void setAlignment(const Label::Alignment & alignment) {
		m_label->setAlignment(alignment);
	}
	/** Retrieve this field's alignment.
	@return					the alignment. */
	inline Label::Alignment getAlignment() const {
		return m_label->getAlignment();
	}


protected:
	// Protected Methods
	inline void setCaret(const int & index) {
		m_caretIndex = std::clamp<int>(index, 0, (int)m_text.size());
		update();
	}


	// Protected Attributes
	std::shared_ptr<Label> m_label;
	std::string m_text;
	bool m_edit = false;
	int m_caretIndex = 0;


private:
	// Private Attributes
	GLuint
		m_vaoID = 0,
		m_vboID[2] = { 0, 0 };
	float m_blinkTime = 0.0f;
	Shared_Shader m_shader;
	StaticBuffer m_indirect;
};

#endif // UI_DROPLIST_H