#include "Modules/UI/Basic Elements/Layout_Horizontal.h"


Layout_Horizontal::Layout_Horizontal(Engine& engine) :
	UI_Element(engine)
{
	// Add Callbacks
	addCallback(static_cast<int>(UI_Element::Interact::on_resize), [&] { alignChildren(); });
	addCallback(static_cast<int>(UI_Element::Interact::on_childrenChange), [&] { alignChildren(); });
}

void Layout_Horizontal::addElement(const std::shared_ptr<UI_Element>& child, const float& sizeRatio)
{
	UI_Element::addElement(child);
	m_sizedChildren.push_back(std::pair(child, sizeRatio));
}

void Layout_Horizontal::setMargin(const float& margin)
{
	m_margin = margin;
	alignChildren();
}

float Layout_Horizontal::getMargin() const noexcept
{
	return m_margin;
}

void Layout_Horizontal::setSpacing(const float& spacing)
{
	m_spacing = spacing;
	alignChildren();
}

float Layout_Horizontal::getSpacing() const noexcept
{
	return m_spacing;
}

void Layout_Horizontal::alignChildren()
{
	const float innerRectSize = m_scale.x;

	// Available space -= margin
	float sizeUsed = m_margin;

	// Available space -= the dimensions of fixed-sized elements
	int fixedElementCount = 0;
	for (const auto& [child, ratio] : m_sizedChildren) {
		if (!std::isnan(child->getMinScale().x) || !std::isnan(child->getMaxScale().x)) {
			sizeUsed += child->getScale().x * ratio;
			fixedElementCount++;
		}
	}

	// Available space -= spacing factor between elements
	if (m_sizedChildren.size() > 1)
		sizeUsed += static_cast<float>(m_sizedChildren.size() - 1ULL) * m_spacing;

	// Remaining space divvied up between remaining elements
	const float remainder = innerRectSize - sizeUsed;
	const float elementSize = remainder / (float(m_sizedChildren.size()) - float(fixedElementCount));

	float positionFromLeft = -m_scale.x + m_margin;
	const float left = positionFromLeft;
	const auto childrenCount = m_sizedChildren.size();
	for (const auto& [child, ratio] : m_sizedChildren) {
		child->setScale(glm::vec2(elementSize * ratio, m_scale.y - m_margin));
		if (childrenCount == 1) {
			child->setPosition(glm::vec2(0.0F));
			continue;
		}
		const float size = child->getScale().x;
		positionFromLeft += size;
		child->setPosition(glm::vec2(positionFromLeft, 0));
		positionFromLeft += size + (m_spacing * 2.0F);
	}
	const float right = positionFromLeft - (m_spacing * 2.0F);

	// Edge Case: all elements are fixed size, gap may be present
	// Solution: change spacing to fit all elements within bounds
	if (childrenCount > 1 && fixedElementCount == childrenCount) {
		const float delta = (left - right) / float(childrenCount + 1ULL);

		for (size_t x = 1; x < childrenCount; ++x)
			m_sizedChildren[x].first->setPosition(m_sizedChildren[x].first->getPosition() + glm::vec2(delta * x, 0));
	}
}