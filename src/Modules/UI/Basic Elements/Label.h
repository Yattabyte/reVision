#pragma once
#ifndef UI_LABEL_H
#define UI_LABEL_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Assets/Shader.h"
#include "Assets/Texture.h"
#include "Utilities/GL/IndirectDraw.h"
#include "Utilities/GL/DynamicBuffer.h"
#include <string>


/** UI text label class, affords displaying text on the screen. */
class Label final : public UI_Element {
public:
	// Public Interaction Enums
	enum class Interact : int {
		on_textChanged = (int)UI_Element::Interact::last_interact_index
	};
	// Public Alignment Enums
	enum class Alignment {
		align_left = -1,
		align_center = 0,
		align_right = 1
	};


	// Public (De)Constructors
	/** Destroy this label. */
	~Label() noexcept;
	/** Construct a label, giving it the desired text.
	@param	engine		reference to the engine to use. 
	@param	text		the label text. */
	explicit Label(Engine& engine, const std::string& text = "Label");


	// Public Interface Implementation
	void renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) noexcept final;


	// Public Methods
	/** Set this label element's text.
	@param	text	the text to use. */
	void setText(const std::string& text);
	/** Retrieve this label's text.
	@return	the text this label uses. */
	std::string getText() const noexcept;
	/** Set this label element's text scaling factor.
	@param	text	the new scaling factor to use. */
	void setTextScale(const float& textScale) noexcept;
	/** Retrieve this label's text scaling factor.
	@return	the text scaling factor. */
	float getTextScale() const noexcept;
	/** Set this label's color.
	@param	text	the new color to render with. */
	void setColor(const glm::vec3& color) noexcept;
	/** Retrieve this label's color.
	@return	the color used by this element. */
	glm::vec3 getColor() const noexcept;
	/** Set this label element's alignment.
	@param	text	the alignment (left, center, right). */
	void setAlignment(const Alignment& alignment) noexcept;
	/** Retrieve this label's alignment.
	@return	the alignment. */
	Alignment getAlignment() const noexcept;


protected:
	// Protected Attributes
	std::string m_text = "";
	float m_textScale = 10.0f;
	glm::vec3 m_color = glm::vec3(1.0f);
	Alignment m_textAlignment = Alignment::align_left;
	GLuint m_vaoID = 0, m_vboID = 0, m_charCount = 0;
	Shared_Shader m_shader;
	Shared_Texture m_textureFont;
	IndirectDraw<> m_indirect;
	DynamicBuffer<1> m_bufferString;
};

#endif // UI_LABEL_H