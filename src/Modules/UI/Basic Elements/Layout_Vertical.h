#pragma once
#ifndef UI_LAYOUT_VERTICAL_H
#define UI_LAYOUT_VERTICAL_H

#include "Modules/UI/Basic Elements/UI_Element.h"


/** A layout class which controls the position and sizes of its children, laying them out evenly in a column. */
class Layout_Vertical : public UI_Element
{
public:
	// (de)Constructors
	inline ~Layout_Vertical() = default;
	inline Layout_Vertical() = default;


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
		const float innerRectSize = m_scale.y;

		// Available space -= margin
		float sizeUsed = m_margin;

		// Available space -= the dimensions of fixed-sized elements
		int fixedElementCount = 0;
		for (size_t x = 0; x < m_children.size(); ++x) 
			if (!std::isnan(m_children[x]->getMinScale().y) || !std::isnan(m_children[x]->getMaxScale().y)) {
				sizeUsed += m_children[x]->getScale().y;
				fixedElementCount++;
			}

		// Available space -= spacing factor between elements
		if (m_children.size() > 1)
			sizeUsed += float(m_children.size() - 1ull) * m_spacing;
		
		// Remaining space divvied up between remaining elements
		const float remainder = innerRectSize - sizeUsed;
		const float elementSize = remainder / (float(m_children.size()) - float(fixedElementCount));
		
		float positionFromTop = m_scale.y - m_margin;
		const float top = positionFromTop;
		for (size_t x = 0; x < m_children.size(); ++x) {
			m_children[x]->setScale(glm::vec2(m_scale.x - m_margin, elementSize));
			if (m_children.size() == 1) {
				m_children[x]->setPosition(glm::vec2(0.0f));
				continue;
			}
			const float size = m_children[x]->getScale().y;
			positionFromTop -= size;
			m_children[x]->setPosition(glm::vec2(0, positionFromTop));
			positionFromTop -= size + (m_spacing * 2.0f);
		}
		const float bottom = positionFromTop + (m_spacing * 2.0f);

		// Edge Case: all elements are fixed size, gap may be present
		// Solution: change spacing to fit all elements within bounds
		if (m_children.size() > 1 && fixedElementCount == m_children.size()) {
			const float delta = (bottom - top) / float(m_children.size() + size_t(1));

			for (size_t x = 1; x < m_children.size(); ++x) {
				m_children[x]->setPosition(m_children[x]->getPosition() + glm::vec2(0, delta * x));
			}
		}
	}


	// Protected Attributes
	float
		m_margin = 10.0f,
		m_spacing = 10.0f;
};

#endif // UI_LAYOUT_VERTICAL_H