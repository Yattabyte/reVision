#pragma once
#ifndef UI_LAYOUT_VERTICAL_H
#define UI_LAYOUT_VERTICAL_H

#include "Modules/UI/Basic Elements/UI_Element.h"


/** A layout class which controls the position and sizes of its children, laying them out evenly in a column. */
class Layout_Vertical : public UI_Element {
public:
	// Public (De)Constructors
	/** Destroy this layout. */
	inline ~Layout_Vertical() = default;
	/** Construct the layout.
	@param	engine		reference to the engine to use. */
	explicit Layout_Vertical(Engine& engine) noexcept;


	// Public Methods
	/** Add a child to this layout, optionally using a specific fraction of size alloted to it.
	@param	child		the child to add to this layout.
	@param	sizeRatio	the fractional amount of size this element should retain when resizing. */
	void addElement(const std::shared_ptr<UI_Element>& child, const float& sizeRatio = 1.0f) noexcept;
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


protected:
	// Protected Methods
	/** Rearrange the position and scale of children, to uniformly fit in this layout. */
	void alignChildren() noexcept;


	// Protected Attributes
	float m_margin = 10.0f, m_spacing = 10.0f;
	std::vector<std::pair<std::shared_ptr<UI_Element>, float>> m_sizedChildren;
};

#endif // UI_LAYOUT_VERTICAL_H