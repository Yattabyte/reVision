#pragma once
#ifndef UI_LIST_H
#define UI_LIST_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Assets/Shader.h"
#include "Utilities/GL/StaticBuffer.h"


/** A UI container class that laysout its children vertically, in a list.
Only modifies the position of its children, not their scale.
If children need to expand to fit inside a parent container, consider using a vertical layout. */
class List : public UI_Element
{
public:
	// Public Interaction Enums
	enum interact {
		on_hover = UI_Element::last_interact_index,
		on_selection
	};


	// Public (de)Constructors
	/** Destroy the list. */
	inline ~List() {
		// Delete geometry
		glDeleteBuffers(1, &m_vboID);
		glDeleteVertexArrays(1, &m_vaoID);
	}
	/** Constructs a list.
	@param	engine		the engine. */
	inline List(Engine * engine) {
		// Asset Loading
		m_shader = Shared_Shader(engine, "UI\\List");

		// Generate vertex array
		glCreateVertexArrays(1, &m_vaoID);
		glEnableVertexArrayAttrib(m_vaoID, 0);
		glVertexArrayAttribBinding(m_vaoID, 0, 0);
		glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glCreateBuffers(1, &m_vboID);
		glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(glm::vec3));
		constexpr auto num_data = 8 * 3;
		glNamedBufferStorage(m_vboID, num_data * sizeof(glm::vec3), 0, GL_DYNAMIC_STORAGE_BIT);
		const GLuint quad[4] = { (GLuint)num_data, 1, 0, 0 };
		m_indirect = StaticBuffer(sizeof(GLuint) * 4, quad, GL_CLIENT_STORAGE_BIT);
	}


	// Public Interface Implementation
	inline virtual void update() override {
		alignChildren();
		updateSelectionGeometry();
		UI_Element::update();
	}
	inline virtual void renderElement(const float & deltaTime, const glm::vec2 & position, const glm::vec2 & scale) override {
		if (!getVisible()) return;
		if (m_hoverIndex > -1) {
			const glm::vec2 newPosition = position + m_position + m_children[m_hoverIndex]->getPosition();
			const glm::vec2 newScale = glm::min(m_children[m_hoverIndex]->getScale(), scale);
			if (m_shader->existsYet()) {
				m_shader->bind();
				m_shader->setUniform(0, newPosition);
				m_shader->setUniform(1, newScale);
				glBindVertexArray(m_vaoID);
				m_indirect.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
				glDrawArraysIndirect(GL_TRIANGLES, 0);
			}
		}
		UI_Element::renderElement(deltaTime, position, glm::min(m_scale, scale));
	}
	inline virtual void mouseAction(const MouseEvent & mouseEvent) override {
		if (getVisible() & getEnabled()) {
			if (mouseWithin(mouseEvent)) {
				MouseEvent subEvent = mouseEvent;
				subEvent.m_xPos = mouseEvent.m_xPos - m_position.x;
				subEvent.m_yPos = mouseEvent.m_yPos - m_position.y;
				// Find which list item the mouse is over
				int index(0);
				bool interacted(false);
				for each (auto & child in m_children) {
					child->mouseAction(subEvent);
					if (child->mouseWithin(subEvent))
						interacted = true;
					if (!interacted)
						index++;
				}

				// Emit when the hovered item changes
				if (interacted && m_hoverIndex != index) {
					m_hoverIndex = index;
					updateSelectionGeometry();
					enactCallback(on_hover);
				}

				// Emit when the mouse enters the list (for the first time)
				if (!m_entered) {
					m_entered = true;
					enactCallback(on_mouse_enter);
				}

				// Emit when the selected item changes
				if (!m_pressed && mouseEvent.m_action == MouseEvent::PRESS) {
					if (interacted) {
						m_selectionIndex = m_hoverIndex;
						updateSelectionGeometry();
						enactCallback(on_selection);
					}
					enactCallback(on_mouse_press);
				}
				// Emit when the mouse releases
				else if (m_pressed && mouseEvent.m_action == MouseEvent::RELEASE) {
					m_pressed = false;
					enactCallback(on_mouse_release);
				}
			}
			else {
				for each (auto & child in m_children)
					child->clearFocus();
				clearFocus();
			}
		}
	}


	// Public Methods
	/** Get the selected item index.
	@return				index of the selected item in this list. */
	inline int getSelectionIndex() const {
		return m_selectionIndex;
	}
	/** Get the hovered item index.
	@return				index of the hovered item in this list. */
	inline int getHoveredIndex() const {
		return m_hoverIndex;
	}
	/** Set the margin distance between elements and the edge of this layout.
	@param	margin		the margin for this layout. */
	inline void setMargin(const float & margin) {
		m_margin = margin;
	}
	/** Get the margin distance between elements and the edge of this layout.
	@return the the margin for this layout. */
	inline float getMargin() const {
		return m_margin;
	}
	/** Set the spacing distance between elements in this layout.
	@param	spacing		the spacing distance between elements. */
	inline void setSpacing(const float & spacing) {
		m_spacing = spacing;
	}
	/** Get the spacing distance between elements in this layout.
	@return				the spacing distance between elements. */
	inline float getSpacing() const {
		return m_spacing;
	}
	/** Set the border size.
	@param		size		the new border size to use. */
	inline void setBorderSize(const float & size) {
		m_borderSize = size;
		update();
	}
	/** Get the border size.
	@return				border size. */
	inline float getBorderSize() const {
		return m_borderSize;
	}


protected:
	// Protected Methods
	/** Update position of each child element. */
	inline void alignChildren() {
		float positionFromTop = m_scale.y - m_margin;
		for (size_t x = 0; x < m_children.size(); ++x) {
			const float size = m_children[x]->getScale().y;
			m_children[x]->setScale(glm::vec2(m_scale.x - m_margin, size));
			if (m_children.size() == 1) {
				m_children[x]->setPosition(glm::vec2(0.0f));
				continue;
			}
			positionFromTop -= size;
			m_children[x]->setPosition(glm::vec2(0, positionFromTop));
			positionFromTop -= size + (m_spacing * 2.0f);
		}
	}
	/** Update the geometry of the selection box. */
	inline void updateSelectionGeometry() {
		if (m_hoverIndex <= -1) return;
		const auto scale = glm::min(m_children[m_hoverIndex]->getScale() + m_spacing, m_scale - m_borderSize);
		constexpr auto num_data = 8 * 3;
		std::vector<glm::vec3> m_data(num_data);

		// Bottom Bar
		m_data[0] = { -scale.x - m_borderSize, -scale.y, 0 };
		m_data[1] = { scale.x + m_borderSize, -scale.y, 0 };
		m_data[2] = { scale.x + m_borderSize, -scale.y + m_borderSize, 0 };
		m_data[3] = { scale.x + m_borderSize, -scale.y + m_borderSize, 0 };
		m_data[4] = { -scale.x - m_borderSize, -scale.y + m_borderSize, 0 };
		m_data[5] = { -scale.x - m_borderSize, -scale.y, 0 };

		// Left Bar
		m_data[6] = { -scale.x, -scale.y - m_borderSize, 0 };
		m_data[7] = { -scale.x + m_borderSize, -scale.y - m_borderSize, 0 };
		m_data[8] = { -scale.x + m_borderSize, scale.y + m_borderSize, 0 };
		m_data[9] = { -scale.x + m_borderSize, scale.y + m_borderSize, 0 };
		m_data[10] = { -scale.x, scale.y + m_borderSize, 0 };
		m_data[11] = { -scale.x, -scale.y - m_borderSize, 0 };

		// Top Bar
		m_data[12] = { -scale.x - m_borderSize, scale.y - m_borderSize, 0 };
		m_data[13] = { scale.x + m_borderSize, scale.y - m_borderSize, 0 };
		m_data[14] = { scale.x + m_borderSize, scale.y, 0 };
		m_data[15] = { scale.x + m_borderSize, scale.y, 0 };
		m_data[16] = { -scale.x - m_borderSize, scale.y, 0 };
		m_data[17] = { -scale.x - m_borderSize, scale.y - m_borderSize, 0 };

		// Right Bar
		m_data[18] = { scale.x - m_borderSize, -scale.y - m_borderSize, 0 };
		m_data[19] = { scale.x, -scale.y - m_borderSize, 0 };
		m_data[20] = { scale.x, scale.y + m_borderSize, 0 };
		m_data[21] = { scale.x, scale.y + m_borderSize, 0 };
		m_data[22] = { scale.x - m_borderSize, scale.y + m_borderSize, 0 };
		m_data[23] = { scale.x - m_borderSize, -scale.y - m_borderSize, 0 };

		glNamedBufferSubData(m_vboID, 0, num_data * sizeof(glm::vec3), &m_data[0]);
	}


	// Protected Attributes
	float
		m_margin = 10.0f,
		m_spacing = 10.0f,
		m_borderSize = 2.0f;
	int
		m_hoverIndex = -1,
		m_selectionIndex = -1;
	GLuint
		m_vaoID = 0,
		m_vboID = 0;
	Shared_Shader m_shader;
	StaticBuffer m_indirect;
};

#endif // UI_LIST_H