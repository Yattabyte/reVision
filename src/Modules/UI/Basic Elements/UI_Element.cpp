#include "Modules\UI\Basic Elements\UI_Element.h"
#include "Engine.h"


UI_Element::UI_Element(Engine * engine) : m_engine(engine) {}

void UI_Element::setScale(const glm::vec2 & scale) 
{
	m_scale = scale;
	update();
	enactCallback(on_resize);
}

void UI_Element::setPosition(const glm::vec2 & position) 
{
	m_position = position;
	update();
}

void UI_Element::update() 
{
	if (!std::isnan(m_minScale.x))
		m_scale.x = std::max<float>(m_scale.x, m_minScale.x);
	if (!std::isnan(m_minScale.y))
		m_scale.y = std::max<float>(m_scale.y, m_minScale.y);
	if (!std::isnan(m_maxScale.x))
		m_scale.x = std::min<float>(m_scale.x, m_maxScale.x);
	if (!std::isnan(m_maxScale.y))
		m_scale.y = std::min<float>(m_scale.y, m_maxScale.y);

	for each (auto & child in m_children)
		child->update();
}

void UI_Element::renderElement(const float & deltaTime, const glm::vec2 & position, const glm::vec2 & scale) 
{
	if (!getVisible()) return;
	const glm::vec2 newPosition = position + m_position;
	const glm::vec2 newScale = glm::min(m_scale, scale);
	for each (auto & child in m_children) {
		if (!child->getVisible()) continue;
		child->renderElement(deltaTime, newPosition, newScale);
	}
}

void UI_Element::mouseAction(const MouseEvent & mouseEvent) 
{
	if (getVisible() && getEnabled() && mouseWithin(mouseEvent)) {
		MouseEvent subEvent = mouseEvent;
		subEvent.m_xPos = mouseEvent.m_xPos - m_position.x;
		subEvent.m_yPos = mouseEvent.m_yPos - m_position.y;
		for each (auto & child in m_children)
			child->mouseAction(subEvent);
		setHovered();
		if (mouseEvent.m_action == MouseEvent::PRESS)
			setPressed();
		else if (mouseEvent.m_action == MouseEvent::RELEASE)
			setReleased();
	}
	else {
		clearFocus();
		for each (auto & child in m_children)
			child->clearFocus();
	}
}

void UI_Element::keyboardAction(const KeyboardEvent & keyboardEvent)
{
	for each (auto & child in m_children)
		child->keyboardAction(keyboardEvent);
}

void UI_Element::userAction(ActionState & actionState) 
{
}

void UI_Element::addElement(const std::shared_ptr<UI_Element>& child) 
{
	m_children.push_back(child);
	update();
}

std::shared_ptr<UI_Element> UI_Element::getElement(const size_t & index) 
{
	return m_children[index];
}

void UI_Element::clearElements() 
{
	m_children.clear();
	update();
}

void UI_Element::addCallback(const int & interactionEventID, const std::function<void()>& func) 
{
	m_callbacks[interactionEventID].push_back(func);
	update();
}

glm::vec2 UI_Element::getPosition() const 
{
	return m_position;
}

glm::vec2 UI_Element::getScale() const {
	return m_scale;
}

void UI_Element::setMaxScale(const glm::vec2 & scale) 
{
	m_maxScale = scale;
	update();
	enactCallback(on_resize);
}

glm::vec2 UI_Element::getMaxScale() const 
{
	return m_maxScale;
}

void UI_Element::setMinScale(const glm::vec2 & scale)
{
	m_minScale = scale;
	update();
	enactCallback(on_resize);
}

glm::vec2 UI_Element::getMinScale() const
{
	return m_minScale;
}

void UI_Element::setVisible(const bool & visible) 
{
	m_visible = visible;
}

bool UI_Element::getVisible() const 
{
	return m_visible;
}

void UI_Element::setEnabled(const bool & enabled) 
{
	m_enabled = enabled;
	for each (auto & child in m_children)
		child->setEnabled(enabled);
}

bool UI_Element::getEnabled() const 
{
	return m_enabled;
}

void UI_Element::setHovered() 
{
	if (!m_hovered) {
		m_hovered = true;
		enactCallback(on_hover_start);
	}
}

bool UI_Element::getHovered() const 
{
	return m_hovered;
}

void UI_Element::setPressed() 
{
	if (!m_pressed) {
		m_pressed = true;
		enactCallback(on_press);
	}
}

bool UI_Element::getPressed() const 
{
	return m_pressed;
}

void UI_Element::setReleased() 
{
	if (m_pressed)
		setClicked();
	m_pressed = false;
	enactCallback(on_release);
}

bool UI_Element::getReleased() const 
{
	return !m_pressed;
}

void UI_Element::setClicked() 
{
	m_hovered = true;
	m_clicked = true;
	enactCallback(on_clicked);
}

void UI_Element::clearFocus() 
{
	//m_pressed = false;
	m_clicked = false;
	if (m_hovered) {
		m_hovered = false;
		enactCallback(on_hover_stop);
	}
}

bool UI_Element::mouseWithin(const MouseEvent & mouseEvent) const 
{
	return withinBBox(m_position - m_scale, m_position + m_scale, glm::vec2(mouseEvent.m_xPos, mouseEvent.m_yPos));
}

bool UI_Element::withinBBox(const glm::vec2 & box_p1, const glm::vec2 & box_p2, const glm::vec2 & point)
{
	return (point.x >= box_p1.x && point.x <= box_p2.x && point.y >= box_p1.y && point.y <= box_p2.y);
}

void UI_Element::enactCallback(const int & interactionEventID) const 
{
	if (m_callbacks.find(interactionEventID) != m_callbacks.end())
		for each (const auto & func in m_callbacks.at(interactionEventID))
			m_engine->getModule_UI().pushCallback(func);
}
