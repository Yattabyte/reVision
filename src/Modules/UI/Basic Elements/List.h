#pragma once
#ifndef UI_LIST_H
#define UI_LIST_H

#include "Modules/UI/Basic Elements/UI_Element.h"


/** Represents a vertical list of UI elements. */
class List : public UI_Element
{
public:
	// (de)Constructors
	inline ~List() = default;
	inline List() = default;


	// Interface Implementation
	inline virtual void update() override {
		alignChildren();

		UI_Element::update();
	}


	// Public Methods
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
	@return the spacing distance between elements. */
	inline float getSpacing() const {
		return m_spacing;
	}


protected:
	// Protected Methods
	inline void alignChildren() {		
		// Update position of each child element
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


	// Protected Attributes
	float
		m_margin = 10.0f,
		m_spacing = 10.0f;
};

#endif // UI_LIST_H