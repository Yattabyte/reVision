#pragma once
#ifndef UI_ELEMENT_H
#define UI_ELEMENT_H

#include "Modules/UI/MouseEvent.h"
#include "Modules/UI/KeyboardEvent.h"
#include "Utilities/GL/glad/glad.h"
#include "Utilities/ActionState.h"
#include "glm/glm.hpp"
#include <algorithm>
#include <functional>
#include <map>
#include <vector>


/** Interface for UI elements, like buttons, labels, panels, etc. */
class UI_Element {
public:
	// Public Interaction Enums
	const enum interact {
		on_resize,
		on_hover_start,
		on_hover_stop,
		on_press,
		on_release,
		last_interact_index = on_release + 1
	};


	// Public (de)Constructors
	/** Destroy this ui element. */
	inline ~UI_Element() = default;
	/** Construct a ui element. */
	inline UI_Element() = default;


	// Public Interface Declaration
	/** Sets this elements' scale.
	@param	scale					the new scale to use. */
	inline virtual void setScale(const glm::vec2 & scale) {
		m_scale = scale;
		update();
		enactCallback(on_resize);
	}
	/** Sets this elements' position.
	@param	position				the new position to use. */
	inline virtual void setPosition(const glm::vec2 & position) {
		m_position = position;
		update();
	}
	/** Requests that this element update itself. */
	inline virtual void update() {
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
	/** Render this element (and all subelements).
	@param	transform				transform to use*/
	inline virtual void renderElement(const float & deltaTime, const glm::vec2 & position = glm::vec2(0.0f), const glm::vec2 & scale = glm::vec2(1.0f)) {
		if (!getVisible()) return;
		const glm::vec2 newPosition = position + m_position;
		const glm::vec2 newScale = glm::min(m_scale, scale);
		for each (auto & child in m_children) {
			if (!child->getVisible()) continue;
			child->renderElement(deltaTime, newPosition, newScale);
		}
	}
	/** Applies a mouse action across this ui element.
	@param		mouseEvent			the mouse event occuring. */
	inline virtual void mouseAction(const MouseEvent & mouseEvent) {
		if (getVisible() && getEnabled() && mouseWithin(mouseEvent)) {		
			MouseEvent subEvent = mouseEvent;
			subEvent.m_xPos = mouseEvent.m_xPos - m_position.x;
			subEvent.m_yPos = mouseEvent.m_yPos - m_position.y;
			for each (auto & child in m_children)
				child->mouseAction(subEvent);
			if (!m_hovered) {
				m_hovered = true;
				enactCallback(on_hover_start);
			}
			if (!m_pressed && mouseEvent.m_action == MouseEvent::PRESS) {
				m_pressed = true;
				enactCallback(on_press);
			}
			else if (m_pressed && mouseEvent.m_action == MouseEvent::RELEASE) {
				m_pressed = false;
				enactCallback(on_release);
			}
		}
		else {
			clearFocus();
			for each (auto & child in m_children)
				child->clearFocus();
		}
	}	
	/** Propogates a keyboard action event from this UI element to its children.
	@param keyboardEvent			the event to propagate. */
	inline virtual void keyboardAction(const KeyboardEvent & keyboardEvent) {
		for each (auto & child in m_children)
			child->keyboardAction(keyboardEvent);
	}
	/***/
	inline virtual bool userAction(ActionState & actionState) {
		for each (auto & child in m_children)
			if (child->userAction(actionState))
				return true;
		return false;
	}


	// Public Methods
	/** Add a child UI element to this one.
	@param		child				the element to be chained to this one. */
	inline void addElement(const std::shared_ptr<UI_Element> & child) {
		m_children.push_back(child);
		update();
	}
	/***/
	inline std::shared_ptr<UI_Element> getElement(const size_t & index) {
		return m_children[index];
	}
	/** Remove all child UI elements. */
	inline void clearElements() {
		m_children.clear();
		update();
	}
	/** Add a callback function, to be called when the given event occurs.
	@param		interactionEventID		the ID corresponding to an event type
	@param		func					the callback function to be called. */
	inline void addCallback(const int & interactionEventID, const std::function<void()> && func) {
		m_callbacks[interactionEventID].push_back(func);
		update();
	}
	/** Gets this elements' position.
	@return	this elements' position. */
	inline glm::vec2 getPosition() const {
		return m_position;
	}
	/** Gets this elements' scale.
	@return	this elements' scale. */
	inline glm::vec2 getScale() const {
		return m_scale;
	}
	/** Sets this elements' maximum scale.
	@param	scale					the new maximum scale to use. */
	inline void setMaxScale(const glm::vec2 & scale) {
		m_maxScale = scale;
		update();
		enactCallback(on_resize);
	}
	/** Gets this elements' maximum scale.
	@return	this elements' maximum scale. */
	inline glm::vec2 getMaxScale() const {
		return m_maxScale;
	}
	/** Sets this elements' minimum scale.
	@param	scale					the new minimum scale to use. */
	inline void setMinScale(const glm::vec2 & scale) {
		m_minScale = scale;
		update();
		enactCallback(on_resize);
	}
	/** Gets this elements' minimum scale.
	@return	this elements' minimum scale. */
	inline glm::vec2 getMinScale() const {
		return m_minScale;
	}
	/** Set this element as visible or not.
	@param	visible					whether or not this element should be visible. */
	inline void setVisible(const bool & visible) {
		m_visible = visible;
	}
	/** Gets this elements' visibility.
	@return	if this element is visible. */
	inline bool getVisible() const {
		return m_visible;
	}
	/** Set this element as enabled or not.
	@param	visible					whether or not this element should be enabled. */
	inline void setEnabled(const bool & enabled) {
		m_enabled = enabled;
		for each (auto & child in m_children)
			child->setEnabled(enabled);
	}
	/** Get the enabled state of this element.
	@return	if this element is enabled. */
	inline bool getEnabled() const {
		return m_enabled;
	}
	/***/
	inline void clearFocus() {
		m_pressed = false;
		if (m_hovered) {
			m_hovered = false;
			enactCallback(on_hover_stop);
		}
	}
	/***/
	inline void fullPress() {
		m_hovered = true;
		enactCallback(on_release);
	}
	/** Get whether or not the mouse is within this element. 
	@return			true if the mouse is within this element. */
	inline bool mouseWithin(const MouseEvent & mouseEvent) const {
		return withinBBox(m_position - m_scale, m_position + m_scale, glm::vec2(mouseEvent.m_xPos, mouseEvent.m_yPos));
	}
	/** Returns whether or not a point is within the bbox specified. */
	inline static bool withinBBox(const glm::vec2 & box_p1, const glm::vec2 & box_p2, const glm::vec2 & point) {
		return (point.x >= box_p1.x && point.x <= box_p2.x && point.y >= box_p1.y && point.y <= box_p2.y);
	}


protected:
	// Protected Methods
	/** Triggers all callback functions identified by the event ID specified.
	@param		interactionEventID	the event type. */
	inline void enactCallback(const int & interactionEventID) const {
		if (m_callbacks.find(interactionEventID) != m_callbacks.end())
			for each (const auto & func in m_callbacks.at(interactionEventID))
				func();
	}


	// Protected Attributes
	glm::vec2 
		m_position = glm::vec2(0.0f),
		m_scale = glm::vec2(1.0f), 
		m_maxScale = glm::vec2(std::nanf(0)), 
		m_minScale = glm::vec2(std::nanf(0));
	bool
		m_visible = true,
		m_enabled = true,
		m_hovered = false,
		m_pressed = false;
	std::vector<std::shared_ptr<UI_Element>> m_children;
	std::map<int, std::vector<std::function<void()>>> m_callbacks;
};

#endif // UI_ELEMENT_H