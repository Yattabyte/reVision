#pragma once
#ifndef UI_SIDELIST_H
#define UI_SIDELIST_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Label.h"
#include "Modules/UI/Decorators/Border.h"
#include "Utilities/GL/IndirectDraw.h"
#include "Assets/Shader.h"
#include <string>
#include <vector>


/** UI list class, scrolls horizontally, displays one element at a time.
Controllable by directional arrows. */
class SideList final : public UI_Element {
public:
	// Public Interaction Enums
	/** Enumerations for interacting with this element. */
	enum class Interact : int {
		on_index_changed = (int)UI_Element::Interact::last_interact_index
	};


	// Public (De)Constructors
	/** Destroy this side list. */
	~SideList();
	/** Construct the side list.
	@param	engine		reference to the engine to use. */
	explicit SideList(Engine& engine);


	// Public Interface Implementation
	void mouseAction(const MouseEvent& mouseEvent) final;
	void userAction(ActionState& actionState) final;
	void renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) final;


	// Public Methods
	/** Set the index to display as selected in the list.
	@param	index		the new integer index to use. */
	void setIndex(const int& index);
	/** Retrieve the index currently used in this list.
	@return	currently	active index. */
	int getIndex() const noexcept;
	/** Set the strings to display in this list.
	@param	strings		the new strings to use in this list. */
	void setStrings(const std::vector<std::string>& strings);
	/** Retrieve the strings this list uses for each item in this list.
	@return				the list of strings describing each item. */
	std::vector<std::string> getStrings() const;


private:
	// Private but deleted
	/** Disallow default constructor. */
	inline SideList() noexcept = delete;
	/** Disallow move constructor. */
	inline SideList(SideList&&) noexcept = delete;
	/** Disallow copy constructor. */
	inline SideList(const SideList&) noexcept = delete;
	/** Disallow move assignment. */
	inline SideList& operator =(SideList&&) noexcept = delete;
	/** Disallow copy assignment. */
	inline SideList& operator =(const SideList&) noexcept = delete;


protected:
	// Protected Methods
	/** Update the data dependant on the scale of this element. */
	void updateGeometry();


	// Protected Attributes
	std::vector<std::string> m_strings;
	int m_index = 0;
	bool
		m_lEnabled = true,
		m_rEnabled = true,
		m_lhighlighted = false,
		m_rhighlighted = false,
		m_lpressed = false,
		m_rpressed = false;
	GLuint
		m_vaoID = 0,
		m_vboID[2] = { 0, 0 };
	Shared_Shader m_shader;
	IndirectDraw<1> m_indirect;
	std::shared_ptr<Label> m_label;
	std::shared_ptr<Border> m_backPanel;
};

#endif // UI_SIDELIST_H