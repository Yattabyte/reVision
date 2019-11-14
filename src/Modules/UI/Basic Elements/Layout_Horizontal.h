#pragma once
#ifndef UI_LAYOUT_HORIZONTAL_H
#define UI_LAYOUT_HORIZONTAL_H

#include "Modules/UI/Basic Elements/UI_Element.h"


/** A layout class which controls the position and sizes of its children, laying them out evenly in a row. */
class Layout_Horizontal : public UI_Element {
public:
	// Public (De)Constructors
	/** Destroy the layout. */
	inline ~Layout_Horizontal() = default;
	/** Construct the layout.
	@param	engine		the engine to use. */
	inline explicit Layout_Horizontal(Engine* engine) noexcept :
		UI_Element(engine)
	{
		// Add Callbacks
		addCallback((int)UI_Element::Interact::on_resize, [&]() { alignChildren(); });
		addCallback((int)UI_Element::Interact::on_childrenChange, [&]() { alignChildren(); });
	}


	// Public Methods
	/** Add a child to this layout, optionally using a specific fraction of size alloted to it.
	@param	child		the child to add to this layout.
	@param	sizeRatio	the fractional amount of size this element should retain when resizing. */
	inline void addElement(const std::shared_ptr<UI_Element>& child, const float& sizeRatio = 1.0f) noexcept {
		UI_Element::addElement(child);
		m_sizedChildren.push_back(std::make_pair(child, sizeRatio));
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
	}
	/** Get the spacing distance between elements in this layout.
	@return				the spacing distance between elements. */
	inline float getSpacing() const noexcept {
		return m_spacing;
	}


protected:
	// Protected Methods
	/** Rearrange the position and scale of children, to uniformly fit in this layout. */
	inline void alignChildren() noexcept {
		const float innerRectSize = m_scale.x;

		// Available space -= margin
		float sizeUsed = m_margin;

		// Available space -= the dimensions of fixed-sized elements
		int fixedElementCount = 0;
		for each (const auto & pair in m_sizedChildren) {
			auto [child, ratio] = pair;
			if (!std::isnan(child->getMinScale().x) || !std::isnan(child->getMaxScale().x)) {
				sizeUsed += child->getScale().x * ratio;
				fixedElementCount++;
			}
		}

		// Available space -= spacing factor between elements
		if (m_sizedChildren.size() > 1)
			sizeUsed += float(m_sizedChildren.size() - size_t(1)) * m_spacing;

		// Remaining space divvied up between remaining elements
		const float remainder = innerRectSize - sizeUsed;
		const float elementSize = remainder / (float(m_sizedChildren.size()) - float(fixedElementCount));

		float positionFromLeft = -m_scale.x + m_margin;
		const float left = positionFromLeft;
		for each (const auto & pair in m_sizedChildren) {
			auto [child, ratio] = pair;
			child->setScale(glm::vec2(elementSize * ratio, m_scale.y - m_margin));
			if (m_sizedChildren.size() == 1) {
				child->setPosition(glm::vec2(0.0f));
				continue;
			}
			const float size = child->getScale().x;
			positionFromLeft += size;
			child->setPosition(glm::vec2(positionFromLeft, 0));
			positionFromLeft += size + (m_spacing * 2.0f);
		}
		const float right = positionFromLeft - (m_spacing * 2.0f);

		// Edge Case: all elements are fixed size, gap may be present
		// Solution: change spacing to fit all elements within bounds
		if (m_sizedChildren.size() > 1 && fixedElementCount == m_sizedChildren.size()) {
			const float delta = (left - right) / float(m_sizedChildren.size() + size_t(1));

			for (size_t x = 1; x < m_sizedChildren.size(); ++x)
				m_sizedChildren[x].first->setPosition(m_sizedChildren[x].first->getPosition() + glm::vec2(delta * x, 0));
		}
	}


	// Protected Attributes
	float m_margin = 10.0f, m_spacing = 10.0f;
	std::vector<std::pair<std::shared_ptr<UI_Element>, float>> m_sizedChildren;
};

#endif // UI_LAYOUT_HORIZONTAL_H