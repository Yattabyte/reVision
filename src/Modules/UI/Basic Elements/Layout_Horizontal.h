#pragma once
#ifndef UI_LAYOUT_HORIZONTAL_H
#define UI_LAYOUT_HORIZONTAL_H

#include "Modules/UI/Basic Elements/UI_Element.h"


/** A layout class which controls the position and sizes of its children, laying them out evenly in a row. */
class Layout_Horizontal : public UI_Element
{
public:
	// (de)Constructors
	~Layout_Horizontal() = default;
	Layout_Horizontal(Engine * engine) {
		alignChildren();
	}


	// Interface Implementation
	virtual void update() override {
		alignChildren();

		UI_Element::update();
	}


	// Public Methods
	/** Set the margin distance between elements and the edge of this layout.
	@param	margin		the margin for this layout. */
	void setMargin(const float & margin) {
		m_margin = margin;
	}
	/** Get the margin distance between elements and the edge of this layout.
	@return the the margin for this layout. */
	float getMargin() const {
		return m_margin;
	}
	/** Set the spacing distance between elements in this layout.
	@param	spacing		the spacing distance between elements. */
	void setSpacing(const float & spacing) {
		m_spacing = spacing;
	}
	/** Get the spacing distance between elements in this layout.
	@return the spacing distance between elements. */
	float getSpacing() const {
		return m_spacing;
	}


protected:
	// Protected Methods
	void alignChildren() {
		const float innerRectSize = m_scale.x;

		// Available space -= margin
		float sizeUsed = m_margin;

		// Available space -= the dimensions of fixed-sized elements
		int fixedElementCount = 0;
		for (size_t x = 0; x < m_children.size(); ++x)
			if (!std::isnan(m_children[x]->getMinScale().x) || !std::isnan(m_children[x]->getMaxScale().x)) {
				sizeUsed += m_children[x]->getScale().x;
				fixedElementCount++;
			}

		// Available space -= spacing factor between elements
		if (m_children.size() > 1)
			sizeUsed += float(m_children.size() - size_t(1)) * m_spacing;

		// Remaining space divvied up between remaining elements
		const float remainder = innerRectSize - sizeUsed;
		const float elementSize = remainder / (float(m_children.size()) - float(fixedElementCount));

		float positionFromLeft = -m_scale.x + m_margin;
		const float left = positionFromLeft;
		for (size_t x = 0; x < m_children.size(); ++x) {
			m_children[x]->setScale(glm::vec2(elementSize, m_scale.y - m_margin));
			if (m_children.size() == 1) {
				m_children[x]->setPosition(glm::vec2(0.0f));
				continue;
			}
			const float size = m_children[x]->getScale().x;
			positionFromLeft += size;
			m_children[x]->setPosition(glm::vec2(positionFromLeft, 0));
			positionFromLeft += size + (m_spacing * 2.0f);
		}
		const float right = positionFromLeft - (m_spacing * 2.0f);

		// Edge Case: all elements are fixed size, gap may be present
		// Solution: change spacing to fit all elements within bounds
		if (left + right < 0.0f && m_children.size() > 1) {
			const float delta = (left - right) / float(m_children.size() + size_t(1));

			for (size_t x = 1; x < m_children.size(); ++x) {
				m_children[x]->setPosition(m_children[x]->getPosition() + glm::vec2(delta * x, 0));
			}
		}
	}


	// Protected Attributes
	float m_margin = 10.0f;
	float m_spacing = 10.0f;
};

#endif // UI_LAYOUT_HORIZONTAL_H