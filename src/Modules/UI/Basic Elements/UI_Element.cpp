#include "Modules\UI\Basic Elements\UI_Element.h"
#include <glad/glad.h>
#include "Engine.h"
#include <algorithm>


// Public (De)Constructors

UI_Element::UI_Element(Engine* engine) noexcept
	: m_engine(engine)
{
}


// Public Interface Declaration

void UI_Element::renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) noexcept
{
	// Exit early if this element is invisible (all child elements will be invisible)
	if (!getVisible()) return;

	// Derive new position and scale, relative to this element for all child elements
	const glm::vec2 newPosition = position + m_position;
	const glm::vec2 newScale = glm::min(m_scale, scale);

	// Render all visible children
	for (auto & child : m_children)
		if (child->getVisible())
			child->renderElement(deltaTime, newPosition, newScale);
}

void UI_Element::mouseAction(const MouseEvent& mouseEvent) noexcept
{
	// If the element is visible, enabled, and the mouse is within, then process the mouse event
	if (getVisible() && getEnabled() && mouseWithin(mouseEvent)) {
		// First, allow the furthest child element to attempt process the event first
		// Furthest child is the most nested, 'closest' to user
		// Offset the mouse event by the position of this element, children have relative coordinates
		MouseEvent subEvent = mouseEvent;
		subEvent.m_xPos = mouseEvent.m_xPos - m_position.x;
		subEvent.m_yPos = mouseEvent.m_yPos - m_position.y;
		for (auto & child : m_children)
			child->mouseAction(subEvent);

		// Since mouse is within bounds, flag this element as hovered, pressed, or released (clicked if pressed->released)
		if (mouseEvent.m_action == MouseEvent::Action::MOVE)
			setHovered();
		else if (mouseEvent.m_action == MouseEvent::Action::PRESS)
			setPressed();
		else if (mouseEvent.m_action == MouseEvent::Action::RELEASE)
			setReleased();
	}
	else {
		// Element either invisible, disabled, or the mouse left bounds
		// So 'revert' the UI state, clear its focus, un-press, un-hover, etc.
		// This is true for all children too.
		clearFocus();
		for (auto & child : m_children)
			child->clearFocus();
	}
}

void UI_Element::keyboardAction(const KeyboardEvent& keyboardEvent) noexcept
{
	// Base UI element has no specific keyboard actions
	// Keyboard actions are specific like typing in a text-box, NOT navigating a main menu
	// Propagate action onto children
	for (auto & child : m_children)
		child->keyboardAction(keyboardEvent);
}

void UI_Element::userAction(ActionState& /*unused*/) noexcept
{
	// Base UI element has no specific user action support
	// User actions are specific actions like navigating a menu, toggling a switch
	// Input standardized as action state, but may come from things like keyboard or controller, but NOT a mouse.
}


// Public Methods

void UI_Element::addElement(const std::shared_ptr<UI_Element>& child) noexcept
{
	m_children.push_back(child);
	enactCallback((int)UI_Element::Interact::on_childrenChange);
}

std::shared_ptr<UI_Element> UI_Element::getElement(const size_t& index) const noexcept
{
	return m_children[index];
}

void UI_Element::clearElements() noexcept
{
	m_children.clear();
	enactCallback((int)UI_Element::Interact::on_childrenChange);
}

void UI_Element::addCallback(const int& interactionEventID, const std::function<void()>& func) noexcept
{
	m_callbacks[interactionEventID].push_back(func);
}

void UI_Element::setPosition(const glm::vec2& position) noexcept
{
	m_position = position;
	enactCallback((int)UI_Element::Interact::on_reposition);
}

glm::vec2 UI_Element::getPosition() const noexcept
{
	return m_position;
}

void UI_Element::setScale(const glm::vec2& scale) noexcept
{
	m_scale = scale;

	// Clamp the scale between our upper and lower ranges
	if (!std::isnan(m_minScale.x))
		m_scale.x = std::max<float>(m_scale.x, m_minScale.x);
	if (!std::isnan(m_minScale.y))
		m_scale.y = std::max<float>(m_scale.y, m_minScale.y);
	if (!std::isnan(m_maxScale.x))
		m_scale.x = std::min<float>(m_scale.x, m_maxScale.x);
	if (!std::isnan(m_maxScale.y))
		m_scale.y = std::min<float>(m_scale.y, m_maxScale.y);

	enactCallback((int)UI_Element::Interact::on_resize);
}

glm::vec2 UI_Element::getScale() const noexcept
{
	return m_scale;
}

void UI_Element::setMaxScale(const glm::vec2& scale) noexcept
{
	m_maxScale = scale;

	// Clamp upper range
	if (!std::isnan(m_maxScale.x))
		m_scale.x = std::min<float>(m_scale.x, m_maxScale.x);
	if (!std::isnan(m_maxScale.y))
		m_scale.y = std::min<float>(m_scale.y, m_maxScale.y);

	enactCallback((int)UI_Element::Interact::on_resize);
}

glm::vec2 UI_Element::getMaxScale() const noexcept
{
	return m_maxScale;
}

void UI_Element::setMaxWidth(const float& width) noexcept
{
	m_maxScale.x = width;
	enactCallback((int)UI_Element::Interact::on_resize);
}

void UI_Element::setMaxHeight(const float& height) noexcept
{
	m_maxScale.y = height;
	enactCallback((int)UI_Element::Interact::on_resize);
}

void UI_Element::setMinScale(const glm::vec2& scale) noexcept
{
	m_minScale = scale;

	// Clamp lower range
	if (!std::isnan(m_minScale.x))
		m_scale.x = std::max<float>(m_scale.x, m_minScale.x);
	if (!std::isnan(m_minScale.y))
		m_scale.y = std::max<float>(m_scale.y, m_minScale.y);

	enactCallback((int)UI_Element::Interact::on_resize);
}

glm::vec2 UI_Element::getMinScale() const noexcept
{
	return m_minScale;
}

void UI_Element::setMinWidth(const float& width) noexcept
{
	m_minScale.x = width;
	enactCallback((int)UI_Element::Interact::on_resize);
}

void UI_Element::setMinHeight(const float& height) noexcept
{
	m_minScale.y = height;
	enactCallback((int)UI_Element::Interact::on_resize);
}

void UI_Element::setVisible(const bool& visible) noexcept
{
	m_visible = visible;
}

bool UI_Element::getVisible() const noexcept
{
	return m_visible;
}

void UI_Element::setEnabled(const bool& enabled) noexcept
{
	m_enabled = enabled;
	for (auto & child : m_children)
		child->setEnabled(enabled);
}

bool UI_Element::getEnabled() const noexcept
{
	return m_enabled;
}

void UI_Element::setHovered() noexcept
{
	if (!m_hovered) {
		m_hovered = true;
		enactCallback((int)UI_Element::Interact::on_hover_start);
	}
}

bool UI_Element::getHovered() const noexcept
{
	return m_hovered;
}

void UI_Element::setPressed() noexcept
{
	if (!m_pressed) {
		m_pressed = true;
		enactCallback((int)UI_Element::Interact::on_press);
	}
}

bool UI_Element::getPressed() const noexcept
{
	return m_pressed;
}

void UI_Element::setReleased() noexcept
{
	if (m_pressed)
		setClicked();
	m_pressed = false;
	enactCallback((int)UI_Element::Interact::on_release);
}

bool UI_Element::getReleased() const noexcept
{
	return !m_pressed;
}

void UI_Element::setClicked() noexcept
{
	m_hovered = true;
	m_clicked = true;
	enactCallback((int)UI_Element::Interact::on_clicked);
}

bool UI_Element::getClicked() noexcept
{
	return false;
}

void UI_Element::clearFocus() noexcept
{
	m_pressed = false;
	m_clicked = false;
	if (m_hovered) {
		m_hovered = false;
		enactCallback((int)UI_Element::Interact::on_hover_stop);
	}
}

bool UI_Element::mouseWithin(const MouseEvent& mouseEvent) const noexcept
{
	return withinBBox(m_position - m_scale, m_position + m_scale, glm::vec2(mouseEvent.m_xPos, mouseEvent.m_yPos));
}

bool UI_Element::withinBBox(const glm::vec2& box_p1, const glm::vec2& box_p2, const glm::vec2& point) noexcept
{
	return (point.x >= box_p1.x && point.x <= box_p2.x && point.y >= box_p1.y && point.y <= box_p2.y);
}

void UI_Element::enactCallback(const int& interactionEventID) noexcept
{
	// Callbacks aren't actually called immediately, but are deferred to the UI module to be performed later
	// This is a safety net in case the callback drastically alters the overall engine state, like deleting the calling UI element
	if (m_callbacks.find(interactionEventID) != m_callbacks.end())
		for (const auto & func : m_callbacks.at(interactionEventID))
			m_engine->getModule_UI().pushCallback(func);
}