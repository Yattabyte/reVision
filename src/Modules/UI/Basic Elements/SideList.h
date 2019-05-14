#pragma once
#ifndef UI_SIDELIST_H
#define UI_SIDELIST_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include <string>
#include <vector>


/** UI drop list class, press to expose the list, select an element to close it. */
class SideList : public UI_Element
{
public:
	// Public interaction enums
	enum interact {
		on_index_changed = UI_Element::last_interact_index
	};


	// (de)Constructors
	inline ~SideList() {
		// Delete geometry
		glDeleteBuffers(2, m_vboID);
		glDeleteVertexArrays(1, &m_vaoID);
	}
	inline SideList(Engine * engine) {
		// Asset Loading
		m_shader = Shared_Shader(engine, "UI\\SideList");

		// Other UI elements
		// Label
		m_label = std::make_shared<Label>(engine);
		m_label->setAlignment(Label::align_center);
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

		setIndex(0);
	}


	// Interface Implementation
	inline virtual void setScale(const glm::vec2 & scale) {
		UI_Element::setScale(scale);
		m_label->setScale(scale);
	}
	inline virtual void update() override {
		constexpr auto num_data = 4 * 3;
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
			data[x] *= glm::vec3(m_scale.x - (m_scale.y * 2.0f), m_scale.y, 0);
			objIndices[x] = 0;
		}

		// Arrows
		data[6] = { -1,  0, 0 };
		data[7] = { 0, -1, 0 };
		data[8] = { 0, 1, 0 };
		for (int x = 6; x < 9; ++x) {
			data[x] = (data[x] * glm::vec3(m_scale.y, m_scale.y, 0)) - glm::vec3(m_scale.x - m_scale.y, 0, 0);
			objIndices[x] = 1;
		}
		data[9] = { 1,  0, 0 };
		data[10] = { 0, 1, 0 };
		data[11] = { 0, -1, 0 };
		for (int x = 9; x < 12; ++x) {
			data[x] = (data[x] * glm::vec3(m_scale.y, m_scale.y, 0)) + glm::vec3(m_scale.x - m_scale.y, 0, 0);
			objIndices[x] = 2;
		}

		glNamedBufferSubData(m_vboID[0], 0, num_data * sizeof(glm::vec3), &data[0]);
		glNamedBufferSubData(m_vboID[1], 0, num_data * sizeof(int), &objIndices[0]);

		UI_Element::update();
	}
	inline virtual bool mouseAction(const MouseEvent & mouseEvent) override {
		if (!getVisible() || !getEnabled()) return false;
		if (mouseWithin(mouseEvent)) {
			const float mx = float(mouseEvent.m_xPos) - m_position.x;
			// Left button
			if (mx >= -m_scale.x && mx <= (-m_scale.x + (m_scale.y * 2.0f))) {
				m_lhighlighted = true;
				if (!m_lpressed && mouseEvent.m_action == MouseEvent::PRESS) 
					m_lpressed = true;				
				else if (m_lpressed && mouseEvent.m_action == MouseEvent::RELEASE) {
					m_lpressed = false;
					int index = getIndex() - 1;
					if (index < 0)
						index = int(m_strings.size()) - 1;
					setIndex(index);
					enactCallback(on_index_changed);
				}
			}
			// Right button
			if (mx >= (m_scale.x - (m_scale.y * 2.0f)) && mx <= m_scale.x) {
				m_rhighlighted = true;
				if (!m_rpressed && mouseEvent.m_action == MouseEvent::PRESS)
					m_rpressed = true;
				else if (m_rpressed && mouseEvent.m_action == MouseEvent::RELEASE) {
					m_rpressed = false;
					int index = getIndex() + 1;
					if (index >= m_strings.size())
						index = 0;
					setIndex(index);
					enactCallback(on_index_changed);
				}
			}
		}
		else {
			m_lhighlighted = false;
			m_rhighlighted = false;
		}
		return (UI_Element::mouseAction(mouseEvent));
	}
	inline virtual void renderElement(const float & deltaTime, const glm::vec2 & position, const glm::vec2 & scale) override {
		if (!getVisible()) return;
		const glm::vec2 newPosition = position + m_position;
		const glm::vec2 newScale = glm::min(m_scale, scale);
		if (m_shader->existsYet()) {
			// Render Background
			m_shader->bind();
			m_shader->setUniform(0, newPosition);
			m_shader->setUniform(1, m_enabled);
			m_shader->setUniform(2, m_lhighlighted);
			m_shader->setUniform(3, m_rhighlighted);
			m_shader->setUniform(4, m_lpressed);
			m_shader->setUniform(5, m_rpressed);
			glBindVertexArray(m_vaoID);
			m_indirect.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);

			// Render Text
			UI_Element::renderElement(deltaTime, position, newScale);
		}
	}


	// Public Methods
	/** Set the index to display as selected in the list.
	@param		index		the new integer index to use. */
	inline void setIndex(const int & index) {
		m_index = index;
		if (m_index < m_strings.size())
			m_label->setText(m_strings[m_index]);
	}
	/** Get the index currently used in this list.
	@return		currently active index. */
	inline int getIndex() const {
		return m_index;
	}
	/** Set the strings to display in this list.
	@param		strings		the new strings to use in this list. */
	inline void setStrings(const std::vector<std::string> & strings) {
		m_strings = strings;
		setIndex(getIndex());
	}


protected:
	// Protected Attributes
	std::shared_ptr<Label> m_label;
	std::vector<std::string> m_strings;
	int m_index = 0;
	bool
		m_lhighlighted = false,
		m_rhighlighted = false,
		m_lpressed = false,
		m_rpressed = false;


private:
	// Private Attributes
	GLuint
		m_vaoID = 0,
		m_vboID[2] = { 0, 0 };
	Shared_Shader m_shader;
	StaticBuffer m_indirect;
};

#endif // UI_SIDELIST_H