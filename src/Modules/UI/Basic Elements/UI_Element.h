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


class Engine;

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
		on_clicked,
		last_interact_index = on_clicked + 1
	};


	// Public (de)Constructors
	/** Destroy this ui element. */
	inline ~UI_Element() = default;
	/** Construct a ui element. */
	UI_Element(Engine * engine);


	// Public Interface Declaration
	/** Sets this elements' scale.
	@param	scale					the new scale to use. */
	virtual void setScale(const glm::vec2 & scale);
	/** Sets this elements' position.
	@param	position				the new position to use. */
	virtual void setPosition(const glm::vec2 & position);
	/** Requests that this element update itself. */
	virtual void update();
	/** Render this element (and all subelements).
	@param	transform				transform to use*/
	virtual void renderElement(const float & deltaTime, const glm::vec2 & position = glm::vec2(0.0f), const glm::vec2 & scale = glm::vec2(1.0f));
	/** Applies a mouse action across this ui element.
	@param		mouseEvent			the mouse event occuring. */
	virtual void mouseAction(const MouseEvent & mouseEvent);
	/** Propogates a keyboard action event from this UI element to its children.
	@param keyboardEvent			the event to propagate. */
	virtual void keyboardAction(const KeyboardEvent & keyboardEvent);
	/***/
	virtual void userAction(ActionState & actionState);


	// Public Methods
	/** Add a child UI element to this one.
	@param		child				the element to be chained to this one. */
	void addElement(const std::shared_ptr<UI_Element> & child);
	/***/
	std::shared_ptr<UI_Element> getElement(const size_t & index);
	/** Remove all child UI elements. */
	void clearElements();
	/** Add a callback function, to be called when the given event occurs.
	@param		interactionEventID		the ID corresponding to an event type
	@param		func					the callback function to be called. */
	void addCallback(const int & interactionEventID, const std::function<void()> & func);
	/** Gets this elements' position.
	@return	this elements' position. */
	glm::vec2 getPosition() const;
	/** Gets this elements' scale.
	@return	this elements' scale. */
	glm::vec2 getScale() const;
	/** Sets this elements' maximum scale.
	@param	scale					the new maximum scale to use. */
	void setMaxScale(const glm::vec2 & scale);
	/** Gets this elements' maximum scale.
	@return	this elements' maximum scale. */
	glm::vec2 getMaxScale() const;
	/** Sets this elements' minimum scale.
	@param	scale					the new minimum scale to use. */
	void setMinScale(const glm::vec2 & scale);
	/** Gets this elements' minimum scale.
	@return	this elements' minimum scale. */
	glm::vec2 getMinScale() const;
	/** Set this element as visible or not.
	@param	visible					whether or not this element should be visible. */
	void setVisible(const bool & visible);
	/** Gets this elements' visibility.
	@return	if this element is visible. */
	bool getVisible() const;
	/** Set this element as enabled or not.
	@param	visible					whether or not this element should be enabled. */
	void setEnabled(const bool & enabled);
	/** Get the enabled state of this element.
	@return	if this element is enabled. */
	bool getEnabled() const;
	/***/
	void setHovered();
	/***/
	bool getHovered() const;
	/***/
	void setPressed();
	/***/
	bool getPressed() const;
	/***/
	void setReleased();
	/***/
	bool getReleased() const;
	/***/
	void setClicked();
	/***/
	void clearFocus();
	/** Get whether or not the mouse is within this element. 
	@return			true if the mouse is within this element. */
	bool mouseWithin(const MouseEvent & mouseEvent) const;
	/** Returns whether or not a point is within the bbox specified. */
	static bool withinBBox(const glm::vec2 & box_p1, const glm::vec2 & box_p2, const glm::vec2 & point);


protected:
	// Protected Methods
	/** Triggers all callback functions identified by the event ID specified.
	@param		interactionEventID	the event type. */
	void enactCallback(const int & interactionEventID) const;


	// Protected Attributes
	Engine * m_engine = nullptr;
	glm::vec2 
		m_position = glm::vec2(0.0f),
		m_scale = glm::vec2(1.0f), 
		m_maxScale = glm::vec2(std::nanf(0)), 
		m_minScale = glm::vec2(std::nanf(0));
	bool
		m_visible = true,
		m_enabled = true,
		m_hovered = false,
		m_pressed = false,
		m_clicked = false;
	std::vector<std::shared_ptr<UI_Element>> m_children;
	std::map<int, std::vector<std::function<void()>>> m_callbacks;
};

#endif // UI_ELEMENT_H