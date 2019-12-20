#include "Modules/UI/FocusMap.h"


void FocusMap::addElement(const std::shared_ptr<UI_Element>& element) 
{
	m_elements.push_back(element);
}

bool FocusMap::removeElement(const std::shared_ptr<UI_Element>& element) 
{
	size_t index(0ull);
	bool found = false;
	for (const auto& e : m_elements) {
		if (e.get() == element.get()) {
			found = true;
			break;
		}
		index++;
	}

	if (found)
		m_elements.erase(m_elements.begin() + index);
	return found;
}

void FocusMap::clear() noexcept
{
	m_elements.clear();
	m_index = -1;
}

void FocusMap::focusIndex(const int& newIndex) noexcept 
{
	if (newIndex >= 0 && newIndex < m_elements.size() && elementFocusable(m_elements[newIndex]))
		m_index = newIndex;
}

bool FocusMap::focusElement(const std::shared_ptr<UI_Element>& element) noexcept
{
	int index(0);
	bool found = false;
	for (const auto& e : m_elements) {
		if (e.get() == element.get()) {
			found = true;
			break;
		}
		index++;
	}

	if (found)
		focusIndex(index);
	return found;
}

void FocusMap::applyActionState(ActionState& actionState) 
{
	if (m_elements.size() && m_index >= 0) {
		if (elementFocusable(m_elements[m_index]))
			m_elements[m_index]->userAction(actionState);

		// Switch focus last, let element try to consume input first
		if (actionState.isAction(ActionState::Action::UI_LEFT) == ActionState::State::PRESS)
			back();
		else if (actionState.isAction(ActionState::Action::UI_RIGHT) == ActionState::State::PRESS)
			forward();
	}
}

void FocusMap::back() noexcept 
{
	focusIndex(m_index - 1);
}

void FocusMap::forward() noexcept 
{
	focusIndex(m_index + 1);
}

bool FocusMap::elementFocusable(const std::shared_ptr<UI_Element>& element) noexcept 
{
	return element->getVisible() && element->getEnabled();
}