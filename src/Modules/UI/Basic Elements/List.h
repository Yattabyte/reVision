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
		glDeleteBuffers(2, m_vboID);
		glDeleteVertexArrays(1, &m_vaoID);
	}
	List(Engine * engine) {
		m_container = std::make_shared<UI_Element>();
		m_scrollbar = std::make_shared<Scrollbar_V>(engine, m_container);
		m_scrollbar->addCallback(Scrollbar_V::on_scroll_change, [&]() {
			updateListElements();
			updateSelection();
		});

		// List
		setIndex(0);

		// Asset Loading
		m_shader = Shared_Shader(engine, "UI\\List");

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
		constexpr auto num_data = 6 * 3;
		glNamedBufferStorage(m_vboID[0], num_data * sizeof(glm::vec3), 0, GL_DYNAMIC_STORAGE_BIT);
		glNamedBufferStorage(m_vboID[1], num_data * sizeof(int), 0, GL_DYNAMIC_STORAGE_BIT);
		const GLuint quad1[4] = { GLuint(2 * 3), 1, 0, 0 };
		const GLuint quad2[4] = { GLuint(4 * 3), 1, GLuint(2 * 3), 0 };
		m_indirectBackground = StaticBuffer(sizeof(GLuint) * 4, quad1, GL_CLIENT_STORAGE_BIT);
		m_indirectHighlights = StaticBuffer(sizeof(GLuint) * 4, quad2, GL_CLIENT_STORAGE_BIT);

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
		constexpr auto num_data = 6 * 3;
		std::vector<glm::vec3> m_data(num_data);
		std::vector<int> m_objIndices(num_data);

		// Center
		for (int x = 0; x < 18; x+=6) {
			m_data[x + 0] = { -1, -1, 0 };
			m_data[x + 1] = { 1, -1, 0 };
			m_data[x + 2] = { 1,  1, 0 };
			m_data[x + 3] = { 1,  1, 0 };
			m_data[x + 4] = { -1,  1, 0 };
			m_data[x + 5] = { -1, -1, 0 };
		}
		for (int x = 0; x < 6; ++x) 
			m_objIndices[x] = 0;
		for (int x = 6; x < 12; ++x) 
			m_objIndices[x] = 1;		
		for (int x = 12; x < 18; ++x)
			m_objIndices[x] = 2;

		glNamedBufferSubData(m_vboID[0], 0, num_data * sizeof(glm::vec3), &m_data[0]);
		glNamedBufferSubData(m_vboID[1], 0, num_data * sizeof(int), &m_objIndices[0]);

		updateListElements();
		updateSelection();

		UI_Element::update();
	}
	virtual void renderElement(const float & deltaTime, const glm::vec2 & position, const glm::vec2 & scale) override {
		if (!getVisible()) return;
		const auto newPosition = position + m_position;
		const auto newScale = glm::min(m_scale, scale);
		if (m_shader->existsYet()) {
			m_shader->bind();
			m_shader->setUniform(0, glm::vec4(m_backgroundTransform.x, m_backgroundTransform.y, newPosition));
			glBindVertexArray(m_vaoID);
			m_indirectBackground.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);

			m_scrollbar->renderElement(deltaTime, newPosition, newScale);
			UI_Element::renderElement(deltaTime, position, newScale);

			m_shader->bind();
			m_shader->setUniform(0, newPosition);
			m_shader->setUniform(1, m_backgroundTransform);
			m_shader->setUniform(2, m_highlightTransform);
			m_shader->setUniform(3, m_selectionTransform);
			glBindVertexArray(m_vaoID);
			m_indirectHighlights.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
		}
	}
	virtual bool mouseMove(const MouseEvent & mouseEvent) override {
		if (!getVisible() || !getEnabled()) return false;
		if (withinBBox(m_position - m_scale, m_position + m_scale, glm::vec2(mouseEvent.m_xPos, mouseEvent.m_yPos))) {
			MouseEvent subsubEvent = mouseEvent;
			subsubEvent.m_xPos = ((mouseEvent.m_xPos - m_position.x) - m_scrollbar->getPosition().x) - m_container->getPosition().x;
			subsubEvent.m_yPos = ((mouseEvent.m_yPos - m_position.y) - m_scrollbar->getPosition().y) - m_container->getPosition().y;
			int counter = 0;
			for each (const auto & element in m_listElements) {
				if (element->mouseWithin(subsubEvent)) {
					m_indexHighlight = counter;	
					updateSelection();
					return true;
				}
				counter++;
			}
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
		m_indexSelection = std::clamp<int>(index, 0, m_listElements.size());
		updateSelection();
		enactCallback(on_index_changed);
	}
	/** Get the index currently used in this list.
	@return		currently active index. */
	int getIndex() const {
		return m_indexSelection;
	}
	void addListElement(const std::shared_ptr<UI_Element> & element) {
		element->setScale(glm::vec2(getScale().x, 25.0f));
		m_container->addElement(element);
		m_listElements.push_back(element);
	}


protected:
	// Protected Methods
	/** Update the position of all list elements. */
	void updateListElements() {
		const auto visible_height = getScale().y * 2.0f;
		const auto all_elements_height = ((25.0f * m_listElements.size()));
		const auto diff = visible_height - all_elements_height;
		const auto offset = (1.0f - (0.5f * m_scrollbar->getLinear() + 0.5f)) * diff;
		int counter = 0;	
		for each (auto & element in m_listElements) {
			element->setPosition(glm::vec2(0.0f, ((counter * -25.0f) + (m_scale.y) -12.5f) - offset));
			counter++;
		}
	}
	/** Update the position of the selection effects. */
	void updateSelection() {
		const auto visible_height = getScale().y * 2.0f;
		const auto all_elements_height = ((25.0f * m_listElements.size()));
		const auto diff = visible_height - all_elements_height;
		const auto offset = (1.0f - (0.5f * m_scrollbar->getLinear() + 0.5f)) * diff;
		m_backgroundTransform = glm::vec4(m_scale.x, m_scale.y, 0, 0),
		m_highlightTransform = glm::vec4(m_scale.x, 12.5f, 0, ((m_indexHighlight * -25.0f) + (m_scale.y) - 12.5f) - offset);
		m_selectionTransform = glm::vec4(m_scale.x, 12.5f, 0, ((m_indexSelection * -25.0f) + (m_scale.y) - 12.5f) - offset);
	}


	// Protected Attributes
	int 
		m_indexSelection = 0, 
		m_indexHighlight = 0;
	glm::vec4 
		m_backgroundTransform = glm::vec4(0), 
		m_highlightTransform = glm::vec4(0),
		m_selectionTransform = glm::vec4(0);
	bool m_open = false;


private:
	// Private Attributes
	GLuint m_vaoID = 0, m_vboID[2] = { 0, 0 };
	Shared_Shader m_shader;
	StaticBuffer m_indirectBackground, m_indirectHighlights;
	std::shared_ptr<UI_Element> m_container;
	std::shared_ptr<Scrollbar_V> m_scrollbar;
	std::vector<std::shared_ptr<UI_Element>> m_listElements;
};

#endif // UI_LIST_H