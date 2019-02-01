#pragma once
#ifndef UI_LIST_H
#define UI_LIST_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Label.h"
#include "Modules/UI/Decorators/Scrollbar_V.h"
#include <memory>
#include <string>


/** UI vertical list class. */
class List : public UI_Element
{
public:
	// Interaction enums
	enum interact {
		on_index_changed = UI_Element::last_interact_index
	};


	// (de)Constructors
	~List() {
		// Delete geometry
		glDeleteBuffers(1, &m_vboID);
		glDeleteVertexArrays(1, &m_vaoID);
	}
	List(Engine * engine) {
		m_container = std::make_shared<UI_Element>();
		m_scrollbar = std::make_shared<Scrollbar_V>(engine, m_container);
		m_scrollbar->addCallback(Scrollbar_V::on_scroll_change, [&]() {
			updateListElements();
		});

		// List
		setIndex(0);

		// Asset Loading
		m_shader = Shared_Shader(engine, "UI\\List");

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
	virtual void setScale(const glm::vec2 & scale) override {
		UI_Element::setScale(scale);
		m_scrollbar->setScale(scale);
		for each (const auto & element in m_listElements)
			element->setScale(glm::vec2(scale.x, 25.0f));
	}
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
			m_data[x] *= glm::vec3(m_scale.x, m_scale.y, 0.0f);

		glNamedBufferSubData(m_vboID, 0, num_data * sizeof(glm::vec3), &m_data[0]);

		updateListElements();

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
		m_scrollbar->renderElement(deltaTime, newPosition, newScale);
		UI_Element::renderElement(deltaTime, position, newScale);
	}
	virtual bool mouseMove(const MouseEvent & mouseEvent) override {
		if (!getVisible() || !getEnabled()) return false;
		if (withinBBox(m_position - m_scale, m_position + m_scale, glm::vec2(mouseEvent.m_xPos, mouseEvent.m_yPos))) {
			MouseEvent subEvent = mouseEvent;
			subEvent.m_xPos = mouseEvent.m_xPos - m_position.x;
			subEvent.m_yPos = mouseEvent.m_yPos - m_position.y;
			if (m_scrollbar->mouseMove(subEvent))
				return true;
		}
		return (UI_Element::mouseMove(mouseEvent));
	}
	virtual bool mouseButton(const MouseEvent & mouseEvent) {
		if (!getVisible() || !getEnabled()) return false;
		if (mouseWithin(mouseEvent)) {
			MouseEvent subsubEvent = mouseEvent;
			subsubEvent.m_xPos = ((mouseEvent.m_xPos - m_position.x) - m_scrollbar->getPosition().x) - m_container->getPosition().x;
			subsubEvent.m_yPos = ((mouseEvent.m_yPos - m_position.y) - m_scrollbar->getPosition().y) - m_container->getPosition().y;
			int counter = 0;
			for each (const auto & element in m_listElements) {
				if (element->mouseWithin(subsubEvent)) {
					setIndex(counter);
					return true;
				}
				counter++;
			}
			MouseEvent subEvent = mouseEvent;
			subEvent.m_xPos = mouseEvent.m_xPos - m_position.x;
			subEvent.m_yPos = mouseEvent.m_yPos - m_position.y;
			if (m_scrollbar->mouseButton(subEvent))
				return true;
		}
		return (UI_Element::mouseButton(mouseEvent));
	}


	// Public Methods
	/** Set the index to display as selected in the list.
	@param		index		the new integer index to use. */
	void setIndex(const int & index) {
		m_index = std::clamp<int>(index, 0, m_listElements.size());
		enactCallback(on_index_changed);
	}
	/** Get the index currently used in this list.
	@return		currently active index. */
	int getIndex() const {
		return m_index;
	}
	void addListElement(const std::shared_ptr<UI_Element> & element) {
		element->setScale(glm::vec2(getScale().x, 25.0f));
		m_container->addElement(element);
		m_listElements.push_back(element);
	}


protected:
	// Protected Methods
	void updateListElements() {
		auto visible_height = getScale().y * 2.0f;
		auto all_elements_height = ((25.0f * m_listElements.size()));
		auto diff = visible_height - all_elements_height;
		const auto offset = (1.0f - (0.5f * m_scrollbar->getLinear() + 0.5f)) * diff;
		int counter = 0;	
		for each (auto & element in m_listElements) {
			element->setPosition(glm::vec2(0.0f, ((counter * -25.0f) + (m_scale.y) -12.5f) - offset));
			counter++;
		}
	}


	// Protected Attributes
	int m_index = 0;
	bool m_open = false;
	bool m_startAnimating = false;
	float m_animateTime = 0.2f;


private:
	// Private Attributes
	GLuint m_vaoID = 0, m_vboID = 0;
	Shared_Shader m_shader;
	StaticBuffer m_indirect;
	std::shared_ptr<UI_Element> m_container;
	std::shared_ptr<Scrollbar_V> m_scrollbar;
	std::vector<std::shared_ptr<UI_Element>> m_listElements;
};

#endif // UI_LIST_H