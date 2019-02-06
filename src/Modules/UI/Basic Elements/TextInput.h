#pragma once
#ifndef UI_TEXTINPUT_H
#define UI_TEXTINPUT_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Label.h"
#include "Assets/Shader.h"
#include "Utilities/GL/StaticBuffer.h"
#include <string>


/** UI Text input element. */
class TextInput : public UI_Element
{
public:
	// (de)Constructors
	~TextInput() {
		// Delete geometry
		glDeleteBuffers(2, m_vboID);
		glDeleteVertexArrays(1, &m_vaoID);
	}
	TextInput(Engine * engine) {
		// Asset Loading
		m_shader = Shared_Shader(engine, "UI\\TextInput");

		// Label
		m_label = std::make_shared<Label>(engine);
		m_label->setAlignment(Label::align_center);
		m_label->setColor(glm::vec3(0.0f));
		addElement(m_label);

		addCallback(on_mouse_release, [&]() {
			m_edit = !m_edit;
			if (m_edit) {
				setText("");
			}
		});

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
		constexpr auto num_data = 2 * 3;
		glNamedBufferStorage(m_vboID[0], num_data * sizeof(glm::vec3), 0, GL_DYNAMIC_STORAGE_BIT);
		glNamedBufferStorage(m_vboID[1], num_data * sizeof(int), 0, GL_DYNAMIC_STORAGE_BIT);
		const GLuint quad[4] = { (GLuint)num_data, 1, 0, 0 };
		m_indirect = StaticBuffer(sizeof(GLuint) * 4, quad, GL_CLIENT_STORAGE_BIT);
	}


	// Interface Implementation
	virtual void setScale(const glm::vec2 & scale) override {
		UI_Element::setScale(scale);
		m_label->setScale(scale);
	}
	virtual void update() override {
		constexpr auto num_data = 2 * 3;
		std::vector<glm::vec3> data(num_data);
		std::vector<int> objIndices(num_data);

		// Main
		data[0] = { -1, -1, 0 };
		data[1] = { 1, -1, 0 };
		data[2] = { 1,  1, 0 };
		data[3] = { 1,  1, 0 };
		data[4] = { -1,  1, 0 };
		data[5] = { -1, -1, 0 };
		for (int x = 0; x < 6; ++x) {
			data[x] *= glm::vec3(m_scale, 0.0f);
			objIndices[x] = 0;
		}

		glNamedBufferSubData(m_vboID[0], 0, num_data * sizeof(glm::vec3), &data[0]);
		glNamedBufferSubData(m_vboID[1], 0, num_data * sizeof(int), &objIndices[0]);

		UI_Element::update();
	}
	virtual bool mouseButton(const MouseEvent & mouseEvent) {
		if (!getVisible() || !getEnabled()) return false;
		if (!mouseWithin(mouseEvent) && m_edit) 
			m_edit = false;			
		return UI_Element::mouseButton(mouseEvent);
	}
	virtual void keyButton(const unsigned int & character) override {
		if (m_edit) {
			setText(getText() + char(character));
		}
	}
	virtual void renderElement(const float & deltaTime, const glm::vec2 & position, const glm::vec2 & scale) override {
		if (!getVisible()) return;
		const auto newPosition = position + m_position;
		const auto newScale = glm::min(m_scale, scale);
		if (m_shader->existsYet()) {
			// Render Background
			m_shader->bind();
			m_shader->setUniform(0, glm::vec3(newPosition, m_depth));
			m_shader->setUniform(1, m_enabled);
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
	void setText(const std::string & text) {
		m_text = text;
		m_label->setText(text);
	}
	/** Get the text displayed in this field.
	@return					the text displayed in this field. */
	std::string getText() const {
		return m_text;
	}
	/** Set this field's alignment.
	@param	text			the alignment (left, center, right). */
	void setAlignment(const Label::Alignment & alignment) {
		m_label->setAlignment(alignment);
	}
	/** Retrieve this field's alignment.
	@return					the alignment. */
	Label::Alignment getAlignment() const {
		return m_label->getAlignment();
	}


protected:
	// Protected Attributes
	std::shared_ptr<Label> m_label;
	std::string m_text;
	bool m_edit = false;


private:
	// Private Attributes
	GLuint
		m_vaoID = 0,
		m_vboID[2] = { 0, 0 };
	Shared_Shader m_shader;
	StaticBuffer m_indirect;
};

#endif // UI_DROPLIST_H