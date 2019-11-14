#pragma once
#ifndef UI_LIST_HORIZONTAL_H
#define UI_LIST_HORIZONTAL_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/FocusMap.h"
#include "Assets/Shader.h"


/** A UI container class that layout its children horizontally, in a list.
Only modifies the position of its children, not their scale.
If children need to expand to fit inside a parent container, consider using a horizontal layout. */
class List_Horizontal : public UI_Element {
public:
	// Public Interaction Enums
	enum class Interact : int {
		on_selection = (int)UI_Element::Interact::last_interact_index
	};


	// Public (De)Constructors
	/** Destroy the list_Horizontal. */
	inline ~List_Horizontal() noexcept {
		// Delete geometry
		glDeleteBuffers(1, &m_vboID);
		glDeleteVertexArrays(1, &m_vaoID);
	}
	/** Constructs a list_Horizontal.
	@param	engine		the engine. */
	inline List_Horizontal(Engine* engine) noexcept :
		UI_Element(engine)
	{
		// Asset Loading
		m_shader = Shared_Shader(engine, "UI\\List_Horizontal");

		// Generate vertex array
		glCreateVertexArrays(1, &m_vaoID);
		glEnableVertexArrayAttrib(m_vaoID, 0);
		glVertexArrayAttribBinding(m_vaoID, 0, 0);
		glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glCreateBuffers(1, &m_vboID);
		glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(glm::vec3));
		constexpr auto num_data = 8 * 6;
		glNamedBufferStorage(m_vboID, num_data * sizeof(glm::vec3), 0, GL_DYNAMIC_STORAGE_BIT);

		// Add Callbacks
		addCallback((int)UI_Element::Interact::on_resize, [&]() {
			alignChildren();
			updateSelectionGeometry();
			});
		addCallback((int)UI_Element::Interact::on_childrenChange, [&]() {
			alignChildren();
			});
	}


	// Public Interface Implementation
	inline virtual void renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) noexcept override {
		// Exit Early
		if (!getVisible() || !m_children.size() || !m_shader->existsYet()) return;

		// Render
		m_shader->bind();
		glBindVertexArray(m_vaoID);
		if (m_hoverIndex > -1) {
			const glm::vec2 newPosition = position + m_position + m_children[m_hoverIndex]->getPosition();
			const glm::vec2 newScale = glm::min(m_children[m_hoverIndex]->getScale(), scale);
			m_shader->setUniform(0, newPosition);
			m_shader->setUniform(1, newScale);
			m_shader->setUniform(2, glm::vec4(1));
			glDrawArrays(GL_TRIANGLES, 0, 24);
		}
		if (m_selectionIndex > -1) {
			const glm::vec2 newPosition = position + m_position + m_children[m_selectionIndex]->getPosition();
			const glm::vec2 newScale = glm::min(m_children[m_selectionIndex]->getScale(), scale);
			m_shader->setUniform(0, newPosition);
			m_shader->setUniform(1, newScale);
			m_shader->setUniform(2, glm::vec4(0.8, 0.6, 0.1, 1));
			glDrawArrays(GL_TRIANGLES, 24, 24);
		}
		Shader::Release();

		// Render Children
		UI_Element::renderElement(deltaTime, position, scale);
	}
	inline virtual void mouseAction(const MouseEvent& mouseEvent) noexcept override {
		UI_Element::mouseAction(mouseEvent);
		if (getVisible() && getEnabled() && mouseWithin(mouseEvent)) {
			// Move hover selection to whatever is beneath mouse
			MouseEvent subEvent = mouseEvent;
			subEvent.m_xPos = mouseEvent.m_xPos - m_position.x;
			subEvent.m_yPos = mouseEvent.m_yPos - m_position.y;
			int index(0);
			for each (auto & child in m_children) {
				if (child->mouseWithin(subEvent)) {
					setHoverIndex(index); // Set hovered item to whatever is beneath mouse
					if (mouseEvent.m_action == MouseEvent::Action::RELEASE)
						setSelectionIndex(index); // Set selected item to whatever is beneath mouse
					break;
				}
				else
					index++;
			}

			// Force current selection to stay highlighted
			if (m_children.size() && m_hoverIndex > -1)
				m_children[m_hoverIndex]->setHovered();
		}
	}
	inline virtual void userAction(ActionState& actionState) noexcept override {
		// User can go up or down the list_Horizontal with an input device
		// User input wraps around, and if an item is selected, moving will deselect it
		if (m_children.size()) {
			// Allow selected child to receive input first
			if (m_selectionIndex >= 0 && m_selectionIndex < m_children.size())
				m_focusMap.applyActionState(actionState);

			// After, process remaining input for the list_Horizontal
			if (actionState.isAction(ActionState::Action::UI_UP) == ActionState::State::PRESS) {
				setHoverIndex(m_hoverIndex - 1 < 0 ? (int)m_children.size() - 1ull : m_hoverIndex - 1);

				if (m_selectionIndex != -1)
					setSelectionIndex(-1);
			}
			else if (actionState.isAction(ActionState::Action::UI_DOWN) == ActionState::State::PRESS) {
				setHoverIndex(m_hoverIndex + 1 > m_children.size() - 1ull ? 0 : m_hoverIndex + 1);

				if (m_selectionIndex != -1)
					setSelectionIndex(-1);
			}
			else if (actionState.isAction(ActionState::Action::UI_ENTER) == ActionState::State::PRESS) {
				if (m_hoverIndex > -1 && m_hoverIndex < m_children.size())
					setSelectionIndex(m_hoverIndex);
			}
		}
	}

	// Public Methods
	/** Change the item this list_Horizontal is hovered over.
	@param	newIndex		the new hover index to use. */
	inline void setHoverIndex(const int& newIndex) noexcept {
		m_hoverIndex = newIndex;
		if (m_children.size()) {
			for each (auto & child in m_children)
				child->clearFocus();
			if (m_hoverIndex > -1 && m_hoverIndex < m_children.size())
				m_children[m_hoverIndex]->setHovered();
		}
		updateSelectionGeometry();
	}
	/** Retrieve this list_Horizontal's hovered item index.
	@return					this list_Horizontal's hovered index. */
	inline int getHoverIndex() const noexcept {
		return m_hoverIndex;
	}
	/** Change this list_Horizontals selected item.
	@param	newIndex		the new selected index. */
	inline void setSelectionIndex(const int& newIndex) noexcept {
		m_selectionIndex = newIndex;
		m_focusMap.focusIndex(m_selectionIndex);
		updateSelectionGeometry();
		enactCallback((int)List_Horizontal::Interact::on_selection);
	}
	/** Retrieve this list_Horizontal's selected item index.
	@return					this list_Horizontal's selected index. */
	inline int getSelectionIndex() const noexcept {
		return m_selectionIndex;
	}
	/** Retrieve this list_Horizontal's focus map.
	List_Horizontals use a separate focus map, because the top-level element in each list_Horizontal slot may be a container or cosmetic only.
	@return					the focus map for this list_Horizontal. */
	FocusMap& getFocusMap() noexcept {
		return m_focusMap;
	}
	/** Set the margin distance between elements and the edge of this layout.
	@param	margin		the margin for this layout. */
	inline void setMargin(const float& margin) noexcept {
		m_margin = margin;
		alignChildren();
	}
	/** Get the margin distance between elements and the edge of this layout.
	@return				the margin for this layout. */
	inline float getMargin() const noexcept {
		return m_margin;
	}
	/** Set the spacing distance between elements in this layout.
	@param	spacing		the spacing distance between elements. */
	inline void setSpacing(const float& spacing) noexcept {
		m_spacing = spacing;
		alignChildren();
		updateSelectionGeometry();
	}
	/** Get the spacing distance between elements in this layout.
	@return				the spacing distance between elements. */
	inline float getSpacing() const noexcept {
		return m_spacing;
	}
	/** Set the border size.
	@param		size		the new border size to use. */
	inline void setBorderSize(const float& size) noexcept {
		m_borderSize = size;
		updateSelectionGeometry();
	}
	/** Get the border size.
	@return				border size. */
	inline float getBorderSize() const noexcept {
		return m_borderSize;
	}


protected:
	// Protected Methods
	/** Update position of each child element. */
	inline void alignChildren() noexcept {
		float positionFromLeft = m_margin;
		for (size_t x = 0; x < m_children.size(); ++x) {
			const float size = m_children[x]->getScale().x;
			m_children[x]->setScale(glm::vec2(size, m_scale.y - m_margin));
			if (m_children.size() == 1) {
				m_children[x]->setPosition(glm::vec2(0.0f));
				continue;
			}
			positionFromLeft += size;
			m_children[x]->setPosition(glm::vec2(positionFromLeft, 0));
			positionFromLeft += size + (m_spacing * 2.0f);
		}
	}
	/** Update the geometry of the selection box. */
	inline void updateSelectionGeometry() noexcept {
		if (m_children.size() < 1) return;
		constexpr auto num_data = 8 * 6;
		std::vector<glm::vec3> m_data(num_data);

		if (m_hoverIndex > -1) {
			auto scale = glm::min(m_children[m_hoverIndex]->getScale() + m_spacing, m_scale - m_borderSize);
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
		}

		if (m_selectionIndex > -1) {
			auto scale = glm::min(m_children[m_selectionIndex]->getScale() + m_spacing, m_scale - m_borderSize);
			// Bottom Bar
			m_data[24] = { -scale.x - m_borderSize, -scale.y, 0 };
			m_data[25] = { scale.x + m_borderSize, -scale.y, 0 };
			m_data[26] = { scale.x + m_borderSize, -scale.y + m_borderSize, 0 };
			m_data[27] = { scale.x + m_borderSize, -scale.y + m_borderSize, 0 };
			m_data[28] = { -scale.x - m_borderSize, -scale.y + m_borderSize, 0 };
			m_data[29] = { -scale.x - m_borderSize, -scale.y, 0 };

			// Left Bar
			m_data[30] = { -scale.x, -scale.y - m_borderSize, 0 };
			m_data[31] = { -scale.x + m_borderSize, -scale.y - m_borderSize, 0 };
			m_data[32] = { -scale.x + m_borderSize, scale.y + m_borderSize, 0 };
			m_data[33] = { -scale.x + m_borderSize, scale.y + m_borderSize, 0 };
			m_data[34] = { -scale.x, scale.y + m_borderSize, 0 };
			m_data[35] = { -scale.x, -scale.y - m_borderSize, 0 };

			// Top Bar
			m_data[36] = { -scale.x - m_borderSize, scale.y - m_borderSize, 0 };
			m_data[37] = { scale.x + m_borderSize, scale.y - m_borderSize, 0 };
			m_data[38] = { scale.x + m_borderSize, scale.y, 0 };
			m_data[39] = { scale.x + m_borderSize, scale.y, 0 };
			m_data[40] = { -scale.x - m_borderSize, scale.y, 0 };
			m_data[41] = { -scale.x - m_borderSize, scale.y - m_borderSize, 0 };

			// Right Bar
			m_data[42] = { scale.x - m_borderSize, -scale.y - m_borderSize, 0 };
			m_data[43] = { scale.x, -scale.y - m_borderSize, 0 };
			m_data[44] = { scale.x, scale.y + m_borderSize, 0 };
			m_data[45] = { scale.x, scale.y + m_borderSize, 0 };
			m_data[46] = { scale.x - m_borderSize, scale.y + m_borderSize, 0 };
			m_data[47] = { scale.x - m_borderSize, -scale.y - m_borderSize, 0 };
		}

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
	FocusMap m_focusMap;
};

#endif // UI_LIST_HORIZONTAL_H