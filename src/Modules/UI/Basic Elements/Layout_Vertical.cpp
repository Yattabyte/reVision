#include "Modules/UI/Basic Elements/Layout_Vertical.h"


Layout_Vertical::Layout_Vertical(Engine& engine) :
	UI_Element(engine)
{
	// Add Callbacks
	addCallback((int)UI_Element::Interact::on_resize, [&]() noexcept { alignChildren(); });
	addCallback((int)UI_Element::Interact::on_childrenChange, [&]() noexcept { alignChildren(); });
}

void Layout_Vertical::addElement(const std::shared_ptr<UI_Element>& child, const float& sizeRatio) 
{
	UI_Element::addElement(child);
	m_sizedChildren.push_back(std::pair(child, sizeRatio));
}

void Layout_Vertical::setMargin(const float& margin) noexcept
{
	m_margin = margin;
	alignChildren();
}

float Layout_Vertical::getMargin() const noexcept 
{
	return m_margin;
}

void Layout_Vertical::setSpacing(const float& spacing) noexcept 
{
	m_spacing = spacing;
	alignChildren();
}

float Layout_Vertical::getSpacing() const noexcept 
{
	return m_spacing;
}

void Layout_Vertical::alignChildren() 
{
	const float innerRectSize = m_scale.y;

	// Available space -= margin
	float sizeUsed = m_margin;

	// Available space -= the dimensions of fixed-sized elements
	int fixedElementCount = 0;
	for (const auto& [child, ratio] : m_sizedChildren) {
		if (!std::isnan(child->getMinScale().y) || !std::isnan(child->getMaxScale().y)) {
			sizeUsed += child->getScale().y * ratio;
			fixedElementCount++;
		}
	}

	// Available space -= spacing factor between elements
	if (m_sizedChildren.size() > 1)
		sizeUsed += (float)(m_sizedChildren.size() - 1ULL) * m_spacing;

	// Remaining space divvied up between remaining elements
	const float remainder = innerRectSize - sizeUsed;
	const float elementSize = remainder / (float(m_sizedChildren.size()) - float(fixedElementCount));

	float positionFromTop = m_scale.y - m_margin;
	const float top = positionFromTop;
	const auto childrenCount = m_sizedChildren.size();
	for (const auto& [child, ratio] : m_sizedChildren) {
		child->setScale(glm::vec2(m_scale.x - m_margin, elementSize * ratio));
		if (childrenCount == 1) {
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
	if (childrenCount > 1 && fixedElementCount == childrenCount) {
		const float delta = (bottom - top) / float(childrenCount + 1ULL);

		for (size_t x = 1; x < childrenCount; ++x)
			m_sizedChildren[x].first->setPosition(m_sizedChildren[x].first->getPosition() + glm::vec2(0, delta * x));
	}
}