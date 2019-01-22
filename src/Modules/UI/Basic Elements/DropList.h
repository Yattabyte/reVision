#pragma once
#ifndef DROPLIST_H
#define DROPLIST_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Label.h"
#include "Engine.h"
#include <memory>
#include <string>


/** UI drop-down list class. */
class DropList : public UI_Element
{
public:
	// (de)Constructors
	~DropList() {
		// Update indicator
		m_aliveIndicator = false;
	}
	DropList(Engine * engine) {
		m_strings = { "qwerty", "assdeeqwe", "fwebfvhdsbjh" };
		m_label = std::make_shared<Label>(engine);
		m_label->setColor(glm::vec3(0.0f));
		m_label->setAlignment(Label::align_center);
		addElement(m_label);
		setIndex(0);

		// Asset Loading
		m_shader = Shared_Shader(engine, "UI\\DropList");

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

		// Callbacks
		addCallback(on_resize, [&]() {m_label->setScale(getScale()); });

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

		UI_Element::update();
	}


	// Interface Implementation
	virtual void update() override {
		constexpr auto num_data = 3 * 3;
		std::vector<glm::vec3> m_data(num_data);
		std::vector<int> m_objIndices(num_data);

		// Center
		m_data[0] = { -1, -1, 0 };
		m_data[1] = { 1, -1, 0 };
		m_data[2] = { 1,  1, 0 };
		m_data[3] = { 1,  1, 0 };
		m_data[4] = { -1,  1, 0 };
		m_data[5] = { -1, -1, 0 };
		for (int x = 0; x < 6; ++x) {
			m_data[x] *= glm::vec3(m_scale, 0.0f);
			m_objIndices[x] = 0;
		}

		// Arrow
		m_data[6] = glm::vec3(0, -5, 0) + glm::vec3(m_scale.x - 25, 0, 0);
		m_data[7] = glm::vec3(10, 5, 0) + glm::vec3(m_scale.x - 25, 0, 0);
		m_data[8] = glm::vec3(-10, 5, 0) + glm::vec3(m_scale.x - 25, 0, 0);

		for (int x = 6; x < 9; ++x)
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
			if (m_startAnimating) {
				m_animateTime += deltaTime;
				if (m_animateTime >= 0.2f)
					m_startAnimating = false;
				m_animateTime = std::clamp<float>(m_animateTime, 0.0f, 1.0f);
			}
			m_shader->bind();
			m_shader->setUniform(0, m_orthoProj);
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
		UI_Element::renderElement(deltaTime, position, newScale);
	}


	// Public Methods
	/** Set the index to display as selected in the list.
	@param		index		the new integer index to use. */
	void setIndex(const int & index) {
		m_index = std::clamp<int>(index, 0, m_strings.size());
		m_label->setText(m_strings.size() > 0 ? m_strings[m_index] : "<Empty List>");
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
	}
	/** Get the current list of strings used in this list.
	@return		currently active list of strings. */
	std::vector<std::string> getStrings() const {
		return m_strings;
	}


protected:
	std::vector<std::string> m_strings;
	int m_index = 0;
	bool m_open = false;
	bool m_startAnimating = false;
	float m_animateTime = 0.2f;


private:
	// Private Attributes
	std::shared_ptr<Label> m_label;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	glm::ivec2 m_renderSize = glm::ivec2(1);
	glm::mat4 m_orthoProj = glm::mat4(1.0f);
	GLuint m_vaoID = 0, m_vboID[2] = { 0, 0 };
	Shared_Shader m_shader;
	StaticBuffer m_indirect;
};

#endif // DROPLIST_H