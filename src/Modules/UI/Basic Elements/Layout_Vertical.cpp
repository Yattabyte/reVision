#include "Modules/UI/Basic Elements/Layout_Vertical.h"


Layout_Vertical::Layout_Vertical(Engine& engine) noexcept :
	UI_Element(engine)
{
	// Add Callbacks
	addCallback((int)UI_Element::Interact::on_resize, [&]() { alignChildren(); });
	addCallback((int)UI_Element::Interact::on_childrenChange, [&]() { alignChildren(); });
}

void Layout_Vertical::addElement(const std::shared_ptr<UI_Element>& child, const float& sizeRatio) noexcept 
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

void Layout_Vertical::alignChildren() noexcept 
{
	const float innerRectSize = m_scale.y;

	// Available space -= margin
	float sizeUsed = m_margin;

	// Available space -= the dimensions of fixed-sized elements
	int fixedElementCount = 0;
	for (const auto& pair : m_sizedChildren) {
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
	for (const auto& pair : m_sizedChildren) {
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