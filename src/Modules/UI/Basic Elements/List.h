#pragma once
#ifndef UI_LIST_H
#define UI_LIST_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/FocusMap.h"
#include "Assets/Shader.h"


/** A UI container class that layout its children vertically, in a list.
Only modifies the position of its children, not their scale.
If children need to expand to fit inside a parent container, consider using a vertical layout. */
class List : public UI_Element {
public:
	// Public Interaction Enums
	enum class Interact : int {
		on_selection = (int)UI_Element::Interact::last_interact_index
	};


	// Public (De)Constructors
	/** Destroy this list. */
	~List() noexcept;
	/** Constructs a list.
	@param	engine		reference to the engine to use. */
	explicit List(Engine& engine) noexcept;


	// Public Interface Implementation
	virtual void renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) noexcept override;
	virtual void mouseAction(const MouseEvent& mouseEvent) noexcept override;
	virtual void userAction(ActionState& actionState) noexcept override;

	// Public Methods
	/** Change the item this list is hovered over.
	@param	newIndex		the new hover index to use. */
	void setHoverIndex(const int& newIndex) noexcept;
	/** Retrieve this list's hovered item index.
	@return					this list's hovered index. */
	int getHoverIndex() const noexcept;
	/** Change this lists selected item.
	@param	newIndex		the new selected index. */
	void setSelectionIndex(const int& newIndex) noexcept;
	/** Retrieve this list's selected item index.
	@return					this list's selected index. */
	int getSelectionIndex() const noexcept;
	/** Retrieve this list's focus map.
	Lists use a separate focus map, because the top-level element in each list slot may be a container or cosmetic only.
	@return					the focus map for this list. */
	FocusMap& getFocusMap() noexcept;
	/** Set the margin distance between elements and the edge of this layout.
	@param	margin		the margin for this layout. */
	void setMargin(const float& margin) noexcept;
	/** Retrieve the margin distance between elements and the edge of this layout.
	@return				the margin for this layout. */
	float getMargin() const noexcept;
	/** Set the spacing distance between elements in this layout.
	@param	spacing		the spacing distance between elements. */
	void setSpacing(const float& spacing) noexcept;
	/** Retrieve the spacing distance between elements in this layout.
	@return				the spacing distance between elements. */
	float getSpacing() const noexcept;
	/** Set the border size.
	@param	size		the new border size to use. */
	void setBorderSize(const float& size) noexcept;
	/** Retrieve the border size.
	@return				border size. */
	float getBorderSize() const noexcept;


protected:
	// Protected Methods
	/** Update position of each child element. */
	void alignChildren() noexcept;
	/** Update the geometry of the selection box. */
	void updateSelectionGeometry() noexcept;


	// Protected Attributes
	float
		m_margin = 10.0f,
		m_spacing = 10.0f,
		m_borderSize = 2.0f;
	int
		m_hoverIndex = -1,
		m_selectionIndex = -1;
	GLuint
		m_vaoID = 0,
		m_vboID = 0;
	Shared_Shader m_shader;
	FocusMap m_focusMap;
};

#endif // UI_LIST_H