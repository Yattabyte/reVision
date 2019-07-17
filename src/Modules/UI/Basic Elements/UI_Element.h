#pragma once
#ifndef UI_ELEMENT_H
#define UI_ELEMENT_H

#include "Modules/UI/MouseEvent.h"
#include "Modules/UI/KeyboardEvent.h"
#include "Utilities/ActionState.h"
#include "glm/glm.hpp"
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
		on_reposition,
		on_childrenChange,
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
	/** Construct a ui element. 
	@param	engine		the engine to use. */
	UI_Element(Engine * engine);


	// Public Interface Declaration
	/** Render this element (and all subelements).
	@param	transform				transform to use. */
	virtual void renderElement(const float & deltaTime, const glm::vec2 & position = glm::vec2(0.0f), const glm::vec2 & scale = glm::vec2(1.0f));
	/** Applies a mouse action across this ui element.
	@param	mouseEvent				the mouse event occuring. */
	virtual void mouseAction(const MouseEvent & mouseEvent);
	/** Propogates a keyboard action event from this UI element to its children.
	@param keyboardEvent			the event to propagate. */
	virtual void keyboardAction(const KeyboardEvent & keyboardEvent);
	/** Process user input from peripheral devices. 
	@param	actionState				engine action state. */
	virtual void userAction(ActionState & actionState);


	// Public Methods
	/** Add a child UI element to this one.
	@param	child				the element to be chained to this one. */
	void addElement(const std::shared_ptr<UI_Element> & child);
	/** Retrieve the child element found at the index specified.
	@param	index				the index of the element to retrieve.
	@return						the element found at the index specified. */
	std::shared_ptr<UI_Element> getElement(const size_t & index) const;
	/** Remove all child UI elements. */
	void clearElements();
	/** Add a callback function, to be called when the given event occurs.
	@param	interactionEventID	the ID corresponding to an event type
	@param	func				the callback function to be called. */
	void addCallback(const int & interactionEventID, const std::function<void()> & func);
	/** Sets this elements' position.
	@param	position			the new position to use. */
	void setPosition(const glm::vec2 & position);
	/** Gets this elements' position.
	@return	this elements' position. */
	glm::vec2 getPosition() const;
	/** Sets this elements' scale.
	@param	scale				the new scale to use. */
	void setScale(const glm::vec2 & scale);
	/** Gets this elements' scale.
	@return	this elements' scale. */
	glm::vec2 getScale() const;
	/** Sets this elements' maximum scale.
	@param	scale				the new maximum scale to use. */
	void setMaxScale(const glm::vec2 & scale);
	/** Gets this elements' maximum scale.
	@return	this elements' maximum scale. */
	glm::vec2 getMaxScale() const;
	/** Set the max width of this element.
	@param	width				the maximum width to use. */
	void setMaxWidth(const float & width);
	/** Set the max height of this element. 
	@param	height				the maximum height to use. */
	void setMaxHeight(const float & height);
	/** Sets this elements' minimum scale.
	@param	scale				the new minimum scale to use. */
	void setMinScale(const glm::vec2 & scale);
	/** Gets this elements' minimum scale.
	@return	this elements' minimum scale. */
	glm::vec2 getMinScale() const;
	/** Set the minimum width of this element.
	@param	width				the minimum width to use. */
	void setMinWidth(const float & width);
	/** Set the minimum height of this element.
	@param	width				the minimum height to use. */
	void setMinHeight(const float & height);
	/** Set this element as visible or not.
	@param	visible				whether or not this element should be visible. */
	void setVisible(const bool & visible);
	/** Gets this elements' visibility.
	@return	if this element is visible. */
	bool getVisible() const;
	/** Set this element as enabled or not.
	@param	visible				whether or not this element should be enabled. */
	void setEnabled(const bool & enabled);
	/** Get the enabled state of this element.
	@return	if this element is enabled. */
	bool getEnabled() const;
	/** Set this element as hovered, enacting its callback. */
	void setHovered();
	/** Retrieve this element's hovered state.
	@return						true if this element is hovered, false otherwise. */
	bool getHovered() const;
	/** Set this element as pressed, enacting its callback. */
	void setPressed();
	/** Retrieve this element's pressed state.
	@return						true if this element is pressed, false otherwise. */
	bool getPressed() const;
	/** Set this element as released, enacting its callback. */
	void setReleased();
	/** Retrieve this element's released state.
	@return						true if this element is released, false otherwise. */
	bool getReleased() const;
	/** Set this element as clicked, enacting its callback. */
	void setClicked();
	/** Retrieve this element's clicked state.
	@return						true if this element is clicked, false otherwise. */
	bool getClicked() const;
	/** Reset this element, releasing, unpressing, unhovering this element and all its children. */
	void clearFocus();
	/** Get whether or not the mouse is within this element. 
	@return						true if the mouse is within this element. */
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