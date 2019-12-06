#pragma once
#ifndef UI_TEXTINPUT_H
#define UI_TEXTINPUT_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Label.h"
#include "Assets/Shader.h"
#include "Utilities/GL/IndirectDraw.h"
#include <string>


/** UI element which displays an editable text box. */
class TextInput : public UI_Element {
public:
	// Public Interaction Enums
	enum class Interact : int {
		on_text_change = (int)UI_Element::Interact::last_interact_index
	};


	// Public (De)Constructors
	/** Destroy this text input. */
	~TextInput() noexcept;
	/** Construct a text input.
	@param	engine		reference to the engine to use. */
	explicit TextInput(Engine& engine) noexcept;


	// Public Interface Implementation
	virtual void mouseAction(const MouseEvent& mouseEvent) noexcept override;
	virtual void keyboardAction(const KeyboardEvent& keyboardEvent) noexcept override;
	virtual void renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) noexcept override;


	// Public Methods
	/** Set the text to display in this field.
	@param		string		the new text to display. */
	void setText(const std::string& text) noexcept;
	/** Retrieve the text displayed in this field.
	@return					the text displayed in this field. */
	std::string getText() const noexcept;


protected:
	// Protected Methods
	/** Set the caret position in this text box. */
	void setCaret(const int& index) noexcept;
	/** Update the data dependant on the scale of this element. */
	void updateGeometry() noexcept;


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