#pragma once
#ifndef UI_DROPLIST_H
#define UI_DROPLIST_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Label.h"
#include "Modules/UI/Decorators/Scrollbar_V.h"
#include "Engine.h"
#include <memory>
#include <string>


/** UI drop-down list class. */
class DropList : public UI_Element
{
public:
	// (de)Constructors
	~DropList() {
		// Delete geometry
		glDeleteBuffers(1, &m_vboID);
		glDeleteVertexArrays(1, &m_vaoID);
	}
	DropList(Engine * engine) : m_engine(engine) {
		// List
		setIndex(0);

		// Asset Loading
		m_shader = Shared_Shader(engine, "UI\\DropList");

		// Generate vertex array
		glCreateVertexArrays(1, &m_vaoID);
		glEnableVertexArrayAttrib(m_vaoID, 0);
		glVertexArrayAttribBinding(m_vaoID, 0, 0);
		glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glCreateBuffers(1, &m_vboID);
		glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(glm::vec3));
		constexpr auto num_data = 2 * 3;
		glNamedBufferStorage(m_vboID, num_data * sizeof(glm::vec3), 0, GL_DYNAMIC_STORAGE_BIT);
		const GLuint quad[4] = { (GLuint)num_data, 1, 0, 0 };
		m_indirect = StaticBuffer(sizeof(GLuint) * 4, quad, GL_CLIENT_STORAGE_BIT);

		update();
	}


	// Interface Implementation
	virtual void update() override {
		constexpr auto num_data = 2 * 3;
		std::vector<glm::vec3> m_data(num_data);

		// Center
		m_data[0] = { -1, -1, 0 };
		m_data[1] = { 1, -1, 0 };
		m_data[2] = { 1,  1, 0 };
		m_data[3] = { 1,  1, 0 };
		m_data[4] = { -1,  1, 0 };
		m_data[5] = { -1, -1, 0 };
		for (int x = 0; x < 6; ++x) 
			m_data[x] = (m_data[x] * glm::vec3(m_scale.x, m_maxScale.y, 0.0f)) - glm::vec3(0, 35.0f, 0);

		glNamedBufferSubData(m_vboID, 0, num_data * sizeof(glm::vec3), &m_data[0]);

		UI_Element::update();
	}
	virtual void renderElement(const float & deltaTime, const glm::vec2 & position, const glm::vec2 & scale) override {
		if (!getVisible()) return;
		const auto newPosition = position + m_position;
		const auto newScale = glm::min(m_scale, scale);
		if (m_shader->existsYet()) {
			if (m_startAnimating) {
				m_animateTime += deltaTime;
				if (m_animateTime >= 0.2f)
					m_startAnimating = false;
				m_animateTime = std::clamp<float>(m_animateTime, 0.0f, 1.0f);
			}
			m_shader->bind();
			m_shader->setUniform(1, newPosition);
			m_shader->setUniform(2, (2.0f * (m_animateTime / 0.2f) - 1.0f));
			glm::vec3 colors[2];
			colors[0] = glm::vec3(1.0f);
			colors[1] = glm::vec3(0.0f);
			m_shader->setUniformArray(3, colors, 2);
			glBindVertexArray(m_vaoID);
			m_indirect.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
		}
		for each (auto & label in m_labels) 
			label->renderElement(deltaTime, newPosition, newScale);
		UI_Element::renderElement(deltaTime, position, newScale);
	}


	// Public Methods
	/** Set the index to display as selected in the list.
	@param		index		the new integer index to use. */
	void setIndex(const int & index) {
		m_index = std::clamp<int>(index, 0, m_strings.size());
	}
	/** Get the index currently used in this list.
	@return		currently active index. */
	int getIndex() const {
		return m_index;
	}
	/** Set the list of strings to display in this list.
	@param		strings		the new list of strings to use. */
	void setStrings(const std::vector<std::string> & strings) {
		m_strings = strings;
		m_labels.clear();
		int counter = 1;
		for each (const auto & string in strings) {
			auto & label = std::make_shared<Label>(m_engine, string);
			label->setColor(glm::vec3(0.0f));
			label->setAlignment(Label::align_center);
			label->setPosition(glm::vec2(0.0f, (counter * -25.0f) + ((25.0f * 8.0f) / 2.0f) - 25.0f));
			m_labels.push_back(label);
			counter++;
		}
		setMinScale(glm::vec2(getMinScale().x, 25.0f));
		setMaxScale(glm::vec2(getMaxScale().x, (25.0f * float(strings.size())) / 2.0f));
	}
	/** Get the current list of strings used in this list.
	@return		currently active list of strings. */
	std::vector<std::string> getStrings() const {
		return m_strings;
	}


protected:
	// Protected Attributes
	std::vector<std::string> m_strings;
	int m_index = 0;
	bool m_open = false;
	bool m_startAnimating = false;
	float m_animateTime = 0.2f;


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	GLuint m_vaoID = 0, m_vboID = 0;
	Shared_Shader m_shader;
	StaticBuffer m_indirect;
	std::vector<std::shared_ptr<Label>> m_labels;
};

#endif // UI_DROPLIST_H