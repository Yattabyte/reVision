#pragma once
#ifndef UI_SIDELIST_H
#define UI_SIDELIST_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Decorators/Border.h"
#include <string>
#include <vector>


/** UI list class, scrolls horizontally, displays one element at a time.
Controllable by directional arrows. */
class SideList : public UI_Element {
public:
	// Public Interaction Enums
	const enum interact {
		on_index_changed = UI_Element::last_interact_index
	};


	// Public (de)Constructors
	/** Destroy the side list. */
	inline ~SideList() {
		// Delete geometry
		glDeleteBuffers(2, m_vboID);
		glDeleteVertexArrays(1, &m_vaoID);
	}
	/** Construct the side list.
	@param	engine		the engine to use. */
	inline SideList(Engine * engine) : UI_Element(engine) {
		// Asset Loading
		m_shader = Shared_Shader(engine, "UI\\SideList");

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

		// Make a background panel for cosemetic purposes
		auto panel = std::make_shared<Panel>(engine);
		panel->setColor(glm::vec4(0.3f));
		m_backPanel = std::make_shared<Border>(engine, panel);
		m_backPanel->setMaxScale({ 172, 14 });
		m_backPanel->setScale({ 172, 14 });

		// Other UI elements
		m_label = std::make_shared<Label>(engine);
		m_label->setAlignment(Label::align_center);
		m_label->setColor(glm::vec3(1.0f));
		m_label->setScale({ 200, 28 });

		setMaxScale({ 200, 28 });
		setMinScale({ 200, 28 });
		addElement(m_backPanel);
		addElement(m_label);
		setIndex(0);
	}


	// Public Interface Implementation
	inline virtual void update() override {
		UI_Element::update();
		constexpr auto num_data = 2 * 3;
		std::vector<glm::vec3> data(num_data);
		std::vector<int> objIndices(num_data);

		// Arrows
		const float arrowHeight = m_scale.y / 2.0f;
		data[0] = { -1,  0, 0 };
		data[1] = { 0, -1, 0 };
		data[2] = { 0, 1, 0 };
		for (int x = 0; x < 3; ++x) {
			data[x] = (data[x] * glm::vec3(arrowHeight)) - glm::vec3(m_backPanel->getScale().x + arrowHeight, 0, 0);
			objIndices[x] = 0;
		}
		data[3] = { 1,  0, 0 };
		data[4] = { 0, 1, 0 };
		data[5] = { 0, -1, 0 };
		for (int x = 3; x < 6; ++x) {
			data[x] = (data[x] * glm::vec3(arrowHeight)) + glm::vec3(m_backPanel->getScale().x + arrowHeight, 0, 0);
			objIndices[x] = 1;
		}

		glNamedBufferSubData(m_vboID[0], 0, num_data * sizeof(glm::vec3), &data[0]);
		glNamedBufferSubData(m_vboID[1], 0, num_data * sizeof(int), &objIndices[0]);
	}
	inline virtual void mouseAction(const MouseEvent & mouseEvent) override {
		UI_Element::mouseAction(mouseEvent);
		if (getVisible() && getEnabled() && mouseWithin(mouseEvent)) {
			const float mx = float(mouseEvent.m_xPos) - m_position.x;
			// Left button
			if (mx >= -m_scale.x && mx <= (-m_scale.x + 14) && m_lEnabled) {
				m_lhighlighted = true;
				if (!m_lpressed && mouseEvent.m_action == MouseEvent::PRESS)
					m_lpressed = true;
				else if (m_lpressed && mouseEvent.m_action == MouseEvent::RELEASE) {
					m_lpressed = false;
					setIndex(getIndex() - 1);
				}
			}
			// Right button
			if (mx >= (m_scale.x - 14) && mx <= m_scale.x && m_rEnabled) {
				m_rhighlighted = true;
				if (!m_rpressed && mouseEvent.m_action == MouseEvent::PRESS)
					m_rpressed = true;
				else if (m_rpressed && mouseEvent.m_action == MouseEvent::RELEASE) {
					m_rpressed = false;
					setIndex(getIndex() + 1);
				}
			}
		}
		else {
			m_lhighlighted = false;
			m_rhighlighted = false;
			m_lpressed = false;
			m_rpressed = false;			
		}
	}
	inline virtual void userAction(ActionState & actionState) override {
		// User can only change selection by using the left/right directional key actions
		if (actionState.isAction(ActionState::UI_LEFT) == ActionState::PRESS)
			setIndex(m_index - 1);
		else if (actionState.isAction(ActionState::UI_RIGHT) == ActionState::PRESS) 
			setIndex(m_index + 1);
	}
	inline virtual void renderElement(const float & deltaTime, const glm::vec2 & position, const glm::vec2 & scale) override {
		// Exit Early
		if (!getVisible() || !m_shader->existsYet()) return;
		const glm::vec2 newPosition = position + m_position;
		const glm::vec2 newScale = glm::min(m_scale, scale);
		
		// Render (background)
		m_shader->bind();
		m_shader->setUniform(0, newPosition);
		m_shader->setUniform(1, m_enabled);
		m_shader->setUniform(2, m_lEnabled);
		m_shader->setUniform(3, m_rEnabled);
		m_shader->setUniform(4, m_lhighlighted);
		m_shader->setUniform(5, m_rhighlighted);
		m_shader->setUniform(6, m_lpressed);
		m_shader->setUniform(7, m_rpressed);
		glBindVertexArray(m_vaoID);
		m_indirect.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Render children (text)
		UI_Element::renderElement(deltaTime, position, newScale);
	}


	// Public Methods
	/** Set the index to display as selected in the list.
	@param		index		the new integer index to use. */
	inline void setIndex(const int & index) {
		if (m_index != index) {
			m_index = std::clamp<int>(index, 0, m_strings.size() - 1ull);
			m_label->setText(m_strings[m_index]);

			m_lEnabled = (index > 0);
			m_rEnabled = (index < (int)(m_strings.size() - 1ull));
			enactCallback(on_index_changed);
		}
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
	/** Retrieve the strings this list uses for each item in this list. 
	@return					the list of strgings describing each item. */
	inline std::vector<std::string> getStrings() const {
		return m_strings;
	}


protected:
	// Protected Attributes
	std::shared_ptr<Label> m_label;
	std::vector<std::string> m_strings;
	int m_index = 0;
	bool
		m_lEnabled = true,
		m_rEnabled = true,
		m_lhighlighted = false,
		m_rhighlighted = false,
		m_lpressed = false,
		m_rpressed = false;
	GLuint
		m_vaoID = 0,
		m_vboID[2] = { 0, 0 };
	Shared_Shader m_shader;
	StaticBuffer m_indirect;
	std::shared_ptr<UI_Element> m_backPanel;
};

#endif // UI_SIDELIST_H