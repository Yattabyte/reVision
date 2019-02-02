#pragma once
#ifndef UI_ELEMENT_H
#define UI_ELEMENT_H

#include "Modules/UI/MouseEvent.h"
#include "GL/glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <vector>


#define UIColor_Static glm::vec3(50, 200, 100)
#define UIColor_Static2 glm::vec3(50, 75, 100)
#define UIColor_Hovered glm::vec3(100, 225, 150)
#define UIColor_Hovered2 glm::vec3(50, 100, 125)
#define UIColor_Pressed glm::vec3(25, 175, 75)
#define UIColor_Pressed2 glm::vec3(25, 50, 75)
#define UIColor_Disabled glm::vec3(100, 125, 100)
#define UIColor_Disabled2 glm::vec3(100, 100, 125)
#define UIColor_Background glm::vec3(0.2)
#define UIColor_Background2 glm::vec3(0.3)
#define UIColor_Border glm::vec3(0.3)
#define UIColor_Border2 glm::vec3(0.4)


/** Interface for UI elements, like buttons, labels, panels, etc. */
class UI_Element
{
public:
	// Public (de)Constructors
	~UI_Element() = default;
	UI_Element() = default;


	// Basic UI interaction enums
	enum interact {
		on_resize,
		on_mouse_enter,
		on_mouse_exit,
		on_mouse_press,
		on_mouse_release,
		last_interact_index = on_mouse_release + 1
	};


	// Public Interface
	/** Sets this elements' scale.
	@param	scale					the new scale to use. */
	virtual void setScale(const glm::vec2 & scale) {
		m_scale = scale;
		update();
		enactCallback(on_resize);
	}
	/** Sets this elements' position.
	@param	position				the new position to use. */
	virtual void setPosition(const glm::vec2 & position) {
		m_position = position;
		update();
	}
	/** Requests that this element update itself. */
	virtual void update() {
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
	/***/
	virtual bool doElementsExceedBounds(const glm::vec2 & scale, const glm::vec2 & positionOffset = glm::vec2(0.0f)) const {
		for each (auto & child in m_children) {
			if (child->doElementsExceedBounds(scale, positionOffset + child->getPosition()))
				return true;

			const auto childPos = child->getPosition();
			const auto childScl = child->getScale();
			if (
				((childPos.x + positionOffset.x) - childScl.x) < (-scale.x) ||
				((childPos.x + positionOffset.x) + childScl.x) > (scale.x) || 
				((childPos.y + positionOffset.y) - childScl.y) < (-scale.y) || 
				((childPos.y + positionOffset.y) + childScl.y) > (scale.y))
				return true;			
		}
		return false;
	}
	/** Render this element (and all subelements).
	@param	transform				transform to use*/
	virtual void renderElement(const float & deltaTime, const glm::vec2 & position = glm::vec2(0.0f), const glm::vec2 & scale = glm::vec2(1.0f)) {	
		const auto newPosition = position + m_position;
		const auto newScale = glm::min(m_scale, scale);
		for each (auto & child in m_children) {
			if (!child->getVisible()) continue;
			// Scissor the region, preventing sub elements to render outside of the bounds of this element
			glScissor(
				newPosition.x - (newScale.x),
				newPosition.y - (newScale.y),
				(newScale.x * 2.0f),
				(newScale.y * 2.0f)
			);
			child->renderElement(deltaTime, newPosition, newScale);
		}
	}
	/** Applies and checks if mouse movement interacts with this UI element. 
	@param		mouseEvent			the mouse event occuring.*/
	virtual bool mouseMove(const MouseEvent & mouseEvent) {
		if (!getVisible() || !getEnabled()) return false;
		if (mouseWithin(mouseEvent) || doElementsExceedBounds(m_scale)) {
			MouseEvent subEvent = mouseEvent;
			subEvent.m_xPos = mouseEvent.m_xPos - m_position.x;
			subEvent.m_yPos = mouseEvent.m_yPos - m_position.y;
			for each (auto & child in m_children) {
				if (child->mouseMove(subEvent))
					return true;
			}
			enactCallback(on_mouse_enter);
			return true;
		}
		enactCallback(on_mouse_exit);
		return false;
	}
	/** Applies and checks if mouse button interacts with this UI element.
	@param		mouseEvent			the mouse event occuring.*/
	virtual bool mouseButton(const MouseEvent & mouseEvent) {
		if (!getVisible() || !getEnabled()) return false;
		if (mouseWithin(mouseEvent) || doElementsExceedBounds(m_scale)) {
			MouseEvent subEvent = mouseEvent;
			subEvent.m_xPos = mouseEvent.m_xPos - m_position.x;
			subEvent.m_yPos = mouseEvent.m_yPos - m_position.y;		
			for each (auto & child in m_children) {
				if (child->mouseButton(subEvent))
					return true;
			}			
			if (mouseEvent.m_button == 0) {
				if (mouseEvent.m_action == 1) 
					enactCallback(on_mouse_press);				
				else 
					enactCallback(on_mouse_release);				
			}
			return true;
		}
		return false;
	}


	// Public Methods
	/** Add a child UI element to this one.
	@param		child				the element to be chained to this one. */
	void addElement(const std::shared_ptr<UI_Element> & child) {
		m_children.push_back(child);
		update();
	}
	/** Remove all child UI elements. */
	void clearElements() {
		m_children.clear();
		update();
	}
	/** Add a callback function, to be called when the given event occurs.
	@param		interactionEventID		the ID corresponding to an event type
	@param		func				the callback function to be called. */
	void addCallback(const int & interactionEventID, const std::function<void()> && func) {
		m_callbacks[interactionEventID].push_back(func);
		update();
	}
	/** Gets this elements' position.
	@return	this elements' position. */
	glm::vec2 getPosition() const {
		return m_position;
	}
	/** Gets this elements' scale.
	@return	this elements' scale. */
	glm::vec2 getScale() const {
		return m_scale;
	}
	/** Sets this elements' maximum scale.
	@param	scale					the new maximum scale to use. */
	void setMaxScale(const glm::vec2 & scale) {
		m_maxScale = scale;
		update();
		enactCallback(on_resize);
	}
	/** Gets this elements' maximum scale.
	@return	this elements' maximum scale. */
	glm::vec2 getMaxScale() const {
		return m_maxScale;
	}
	/** Sets this elements' minimum scale.
	@param	scale					the new minimum scale to use. */
	void setMinScale(const glm::vec2 & scale) {
		m_minScale = scale;
		update();
		enactCallback(on_resize);
	}
	/** Gets this elements' minimum scale.
	@return	this elements' minimum scale. */
	glm::vec2 getMinScale() const {
		return m_minScale;
	}
	/** Set this element as visible or not.
	@param	visible					whether or not this element should be visible. */
	void setVisible(const bool & visible) {
		m_visible = visible;
		for each (auto & child in m_children)
			child->setVisible(visible);
	}
	/** Gets this elements' visibility.
	@return	if this element is visible. */
	bool getVisible() const {
		return m_visible;
	}
	/** Set this element as enabled or not.
	@param	visible					whether or not this element should be enabled. */
	void setEnabled(const bool & enabled) {
		m_enabled = enabled;
		for each (auto & child in m_children)
			child->setEnabled(enabled);
	}
	/** Get the enabled state of this element.
	@return	if this element is enabled. */
	bool getEnabled() const {
		return m_enabled;
	}
	/** Get whether or not the mouse is within this element. 
	@return			true if the mouse is within this element. */
	bool mouseWithin(const MouseEvent & mouseEvent) const {
		return withinBBox(m_position - m_scale, m_position + m_scale, glm::vec2(mouseEvent.m_xPos, mouseEvent.m_yPos));
	}
	/** Returns whether or not a point is within the bbox specified. */
	static bool withinBBox(const glm::vec2 & box_p1, const glm::vec2 & box_p2, const glm::vec2 & point) {
		return (point.x >= box_p1.x && point.x <= box_p2.x && point.y >= box_p1.y && point.y <= box_p2.y);
	}


protected:
	// Protected Methods
	/** Triggers all callback functions identified by the event ID specified.
	@param		interactionEventID	the event type. */
	void enactCallback(const int & interactionEventID) {
		for each (const auto & func in m_callbacks[interactionEventID])
			func();
	}


	// Protected Attributes
	glm::vec2 m_position = glm::vec2(0.0f);
	glm::vec2 m_scale = glm::vec2(1.0f), 
		m_maxScale = glm::vec2(std::nanf(0)), 
		m_minScale = glm::vec2(std::nanf(0));
	bool m_visible = true, 
		m_enabled = true;
	std::vector<std::shared_ptr<UI_Element>> m_children;
	std::map<int, std::vector<std::function<void()>>> m_callbacks;
};

#endif // UI_ELEMENT_H