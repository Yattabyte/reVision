#pragma once
#ifndef UI_DROPLIST_H
#define UI_DROPLIST_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/List.h"
#include <string>
#include <vector>


/** UI drop list class, press to expose the list, select an element to close it. */
class DropList : public UI_Element 
{
public:
	// Public interaction enums
	enum interact {
		on_index_changed = List::on_index_changed
	};


	// (de)Constructors
	~DropList() {
		// Delete geometry
		glDeleteBuffers(2, m_vboID);
		glDeleteVertexArrays(1, &m_vaoID);
	}
	DropList(Engine * engine) : m_engine(engine) {
		// Asset Loading
		m_shader = Shared_Shader(engine, "UI\\DropList");

		// Other UI elements
		// List
		m_list = std::make_shared<List>(engine);
		m_list->setVisible(false);
		m_list->addCallback(List::on_index_changed, [&]() {
			const auto index = m_list->getIndex();
			if (m_strings.size() && index < m_strings.size())
				m_label->setText(m_strings[m_list->getIndex()]);
			m_list->setVisible(false);
			enactCallback(on_index_changed);
		});
		// Label
		m_label = std::make_shared<Label>(engine);
		m_label->setAlignment(Label::align_center);
		m_label->setColor(glm::vec3(0.0f));
		addElement(m_label);
		addCallback(on_mouse_enter, [&]() {m_highlighted = true; });
		addCallback(on_mouse_exit, [&]() {m_highlighted = false; });
		addCallback(on_mouse_press, [&]() {m_pressed = true; });
		addCallback(on_mouse_release, [&]() {
			m_pressed = false; 
			m_list->setVisible(true);
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
		constexpr auto num_data = 3 * 3;
		glNamedBufferStorage(m_vboID[0], num_data * sizeof(glm::vec3), 0, GL_DYNAMIC_STORAGE_BIT);
		glNamedBufferStorage(m_vboID[1], num_data * sizeof(int), 0, GL_DYNAMIC_STORAGE_BIT);
		const GLuint quad[4] = { (GLuint)num_data, 1, 0, 0 };
		m_indirect = StaticBuffer(sizeof(GLuint) * 4, quad, GL_CLIENT_STORAGE_BIT);

		setIndex(0);
	}


	// Interface Implementation
	virtual void setScale(const glm::vec2 & scale) override {
		UI_Element::setScale(scale);
		m_list->setScale(glm::vec2(scale.x, 75.0f));
		m_label->setScale(scale);
	}
	virtual void update() override {
		constexpr auto num_data = 3 * 3;
		std::vector<glm::vec3> data(num_data);
		std::vector<int> objIndices(num_data);

		// Main
		data[0] = { -1, -1, 0 };
		data[1] = { 1, -1, 0 };
		data[2] = { 1,  1, 0 };
		data[3] = { 1,  1, 0 };
		data[4] = { -1,  1, 0 };
		data[5] = { -1, -1, 0 };		
		for (int x = 0; x < 6; ++x)
			objIndices[x] = 0;

		// Triangle
		data[6] = { 1,  1, 0 };
		data[7] = { -1,  1, 0 };
		data[8] = { 0, -1, 0 };
		for (int x = 6; x < 9; ++x)
			objIndices[x] = 1;

		glNamedBufferSubData(m_vboID[0], 0, num_data * sizeof(glm::vec3), &data[0]);
		glNamedBufferSubData(m_vboID[1], 0, num_data * sizeof(int), &objIndices[0]);

		UI_Element::update();
		m_list->update();
	}
	virtual bool mouseMove(const MouseEvent & mouseEvent) override {
		if (!getVisible() || !getEnabled()) return false;
		if (m_list->getVisible()) {
			MouseEvent subEvent = mouseEvent;
			subEvent.m_xPos = mouseEvent.m_xPos - m_position.x;
			subEvent.m_yPos = mouseEvent.m_yPos - m_position.y;
			if (m_list->mouseMove(subEvent))
				return true;
		}
		return (UI_Element::mouseMove(mouseEvent));
	}
	virtual bool mouseButton(const MouseEvent & mouseEvent) override {
		if (!getVisible() || !getEnabled()) return false;
		if (m_list->getVisible()) {
			MouseEvent subEvent = mouseEvent;
			subEvent.m_xPos = mouseEvent.m_xPos - m_position.x;
			subEvent.m_yPos = mouseEvent.m_yPos - m_position.y;
			if (m_list->mouseButton(subEvent))
				return true;
		}
		return (UI_Element::mouseButton(mouseEvent));
	}
	virtual void renderElement(const float & deltaTime, const glm::vec2 & position, const glm::vec2 & scale) override {
		if (!getVisible()) return;
		const auto newPosition = position + m_position;
		const auto newScale = glm::min(m_scale, scale);
		if (m_shader->existsYet() && !m_list->getVisible()) {
			// Render Background
			m_shader->bind();
			m_shader->setUniform(0, newPosition);
			m_shader->setUniform(1, glm::vec4(m_scale.x, 12.5f, 0, 0));
			m_shader->setUniform(2, glm::vec4(12.5f, 6.25f, m_scale.x - 25.0f, 0));
			m_shader->setUniform(3, m_highlighted);
			m_shader->setUniform(4, m_pressed);
			glBindVertexArray(m_vaoID);
			m_indirect.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);

			// Render Text
			UI_Element::renderElement(deltaTime, position, newScale);		
		}
		else if (m_list->getVisible()) {
			// Render List
			glScissor(
				newPosition.x - (newScale.x),
				newPosition.y - (75),
				(newScale.x * 2.0f),
				(75 * 2.0f)
			);
			m_list->renderElement(deltaTime, newPosition, glm::vec2(newScale.x, 75.0f));
		}
	}


	// Public Methods
	/** Set the index to display as selected in the list.
	@param		index		the new integer index to use. */
	void setIndex(const int & index) {
		m_list->setIndex(index);
	}
	/** Get the index currently used in this list.
	@return		currently active index. */
	int getIndex() const {
		return m_list->getIndex();
	}
	/** Set the strings to display in this list.
	@param		strings		the new strings to use in this list. */
	void setStrings(const std::vector<std::string> & strings) {
		m_strings = strings;
		m_list->clearListElements();
		for each (const auto & string in strings) {
			auto & label = std::make_shared<Label>(m_engine, string);
			label->setColor(glm::vec3(0.0f));
			label->setAlignment(Label::align_center);
			m_list->addListElement(label);
		}
		const auto index = getIndex();
		if (index < strings.size())
			m_label->setText(strings[index]);
	}


protected:
	// Protected Attributes
	std::shared_ptr<Label> m_label;
	std::shared_ptr<List> m_list;
	std::vector<std::string> m_strings;
	bool m_highlighted = false, m_pressed = false;
	

private:
	// Private Attributes
	Engine * m_engine = nullptr;
	GLuint m_vaoID = 0, m_vboID[2] = { 0, 0 };
	Shared_Shader m_shader;
	StaticBuffer m_indirect;
};

#endif // UI_DROPLIST_H