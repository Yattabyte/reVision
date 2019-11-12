#pragma once
#ifndef UI_LAYOUT_VERTICAL_H
#define UI_LAYOUT_VERTICAL_H

#include "Modules/UI/Basic Elements/UI_Element.h"


/** A layout class which controls the position and sizes of its children, laying them out evenly in a column. */
class Layout_Vertical : public UI_Element {
public:
	// Public (De)Constructors
	/** Destroy the layout. */
	inline ~Layout_Vertical() = default;
	/** Construct the layout.
	@param	engine		the engine to use. */
	inline Layout_Vertical(Engine* engine) : UI_Element(engine) {
		// Add Callbacks
		addCallback(UI_Element::on_resize, [&]() { alignChildren(); });
		addCallback(UI_Element::on_childrenChange, [&]() { alignChildren(); });
	}


	// Public Methods
	/** Add a child to this layout, optionally using a specific fraction of size alloted to it.
	@param	child		the child to add to this layout.
	@param	sizeRatio	the fractional amount of size this element should retain when resizing. */
	inline void addElement(const std::shared_ptr<UI_Element>& child, const float& sizeRatio = 1.0f) {
		UI_Element::addElement(child);
		m_sizedChildren.push_back(std::make_pair(child, sizeRatio));
	}
	/** Set the margin distance between elements and the edge of this layout.
	@param	margin		the margin for this layout. */
	inline void setMargin(const float& margin) {
		m_margin = margin;
		alignChildren();
	}
	/** Get the margin distance between elements and the edge of this layout.
	@return				the margin for this layout. */
	inline float getMargin() const {
		return m_margin;
	}
	/** Set the spacing distance between elements in this layout.
	@param	spacing		the spacing distance between elements. */
	inline void setSpacing(const float& spacing) {
		m_spacing = spacing;
		alignChildren();
	}
	/** Get the spacing distance between elements in this layout.
	@return				the spacing distance between elements. */
	inline float getSpacing() const {
		return m_spacing;
	}


protected:
	// Protected Methods
	/** Rearrange the position and scale of children, to uniformly fit in this layout. */
	inline void alignChildren() {
		const float innerRectSize = m_scale.y;

		// Available space -= margin
		float sizeUsed = m_margin;

		// Available space -= the dimensions of fixed-sized elements
		int fixedElementCount = 0;
		for each (const auto & pair in m_sizedChildren) {
			auto [child, ratio] = pair;
			if (!std::isnan(child->getMinScale().y) || !std::isnan(child->getMaxScale().y)) {
				sizeUsed += child->getScale().y * ratio;
				fixedElementCount++;
			}
		}

		// Available space -= spacing factor between elements
		if (m_sizedChildren.size() > 1)
			sizeUsed += float(m_sizedChildren.size() - 1ull) * m_spacing;

		// Remaining space divvied up between remaining elements
		const float remainder = innerRectSize - sizeUsed;
		const float elementSize = remainder / (float(m_sizedChildren.size()) - float(fixedElementCount));

		float positionFromTop = m_scale.y - m_margin;
		const float top = positionFromTop;
		for each (const auto & pair in m_sizedChildren) {
			auto [child, ratio] = pair;
			child->setScale(glm::vec2(m_scale.x - m_margin, elementSize * ratio));
			if (m_sizedChildren.size() == 1) {
				child->setPosition(glm::vec2(0.0f));
				continue;
			}
			const float size = child->getScale().y;
			positionFromTop -= size;
			child->setPosition(glm::vec2(0, positionFromTop));
			positionFromTop -= size + (m_spacing * 2.0f);
		}
		const float bottom = positionFromTop + (m_spacing * 2.0f);

		// Edge Case: all elements are fixed size, gap may be present
		// Solution: change spacing to fit all elements within bounds
		if (m_sizedChildren.size() > 1 && fixedElementCount == m_sizedChildren.size()) {
			const float delta = (bottom - top) / float(m_sizedChildren.size() + size_t(1));

			for (size_t x = 1; x < m_sizedChildren.size(); ++x)
				m_sizedChildren[x].first->setPosition(m_sizedChildren[x].first->getPosition() + glm::vec2(0, delta * x));
		}
	}


	// Protected Attributes
	float m_margin = 10.0f, m_spacing = 10.0f;

	std::vector<std::pair<std::shared_ptr<UI_Element>, float>> m_sizedChildren;
};

#endif // UI_LAYOUT_VERTICAL_H