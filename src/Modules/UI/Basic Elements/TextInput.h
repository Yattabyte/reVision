#pragma once
#ifndef UI_TEXTINPUT_H
#define UI_TEXTINPUT_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Label.h"
#include "Assets/Shader.h"
#include "Utilities/GL/IndirectDraw.h"
#include <string>


/** UI element which displays an editable text box. */
class TextInput final : public UI_Element {
public:
	// Public Interaction Enums
	enum class Interact : int {
		on_text_change = (int)UI_Element::Interact::last_interact_index
	};


	// Public (De)Constructors
	/** Destroy this text input. */
	~TextInput();
	/** Construct a text input.
	@param	engine		reference to the engine to use. */
	explicit TextInput(Engine& engine);


	// Public Interface Implementation
	void mouseAction(const MouseEvent& mouseEvent) final;
	void keyboardAction(const KeyboardEvent& keyboardEvent) final;
	void renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) final;


	// Public Methods
	/** Set the text to display in this field.
	@param		string		the new text to display. */
	void setText(const std::string& text);
	/** Retrieve the text displayed in this field.
	@return					the text displayed in this field. */
	std::string getText() const;


private:
	// Private but deleted
	/** Disallow default constructor. */
	inline TextInput() noexcept = delete;
	/** Disallow move constructor. */
	inline TextInput(TextInput&&) noexcept = delete;
	/** Disallow copy constructor. */
	inline TextInput(const TextInput&) noexcept = delete;
	/** Disallow move assignment. */
	inline TextInput& operator =(TextInput&&) noexcept = delete;
	/** Disallow copy assignment. */
	inline TextInput& operator =(const TextInput&) noexcept = delete;


protected:
	// Protected Methods
	/** Set the caret position in this text box. */
	void setCaret(const size_t& index);
	/** Update the data dependant on the scale of this element. */
	void updateGeometry();


	// Protected Attributes
	std::shared_ptr<Label> m_label;
	std::string m_text;
	bool m_edit = false;
	int m_caretIndex = 0;
	GLuint
		m_vaoID = 0,
		m_vboID[2] = { 0, 0 };
	float m_blinkTime = 0.0f;
	Shared_Shader m_shader;
	IndirectDraw<1> m_indirect;
};

#endif // UI_TEXTINPUT_H