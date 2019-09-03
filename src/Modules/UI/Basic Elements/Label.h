#pragma once
#ifndef UI_LABEL_H
#define UI_LABEL_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Assets/Shader.h"
#include "Assets/Texture.h"
#include "Utilities/GL/IndirectDraw.h"
#include "Utilities/GL/DynamicBuffer.h"
#include <algorithm>
#include <string>
#include "Engine.h"


/** UI text label class, affords displaying text on the screen. */
class Label : public UI_Element {
public:
	// Public Interaction Enums
	const enum interact {
		on_textChanged = UI_Element::last_interact_index
	};
	// Public Alignment Enums
	const enum Alignment {
		align_left = -1,
		align_center = 0,
		align_right = 1
	};


	// Public (de)Constructors
	/** Destroy the label. */
	inline ~Label() {
		// Delete geometry
		glDeleteBuffers(1, &m_vboID);
		glDeleteVertexArrays(1, &m_vaoID);
	}
	/** Construct a label, giving it the desired text. 
	@param	engine		the engine.
	@param	text		the label text. */
	inline Label(Engine * engine, const std::string & text = "Label")
		: UI_Element(engine) {
		// Asset Loading
		m_shader = Shared_Shader(engine, "UI\\Label");
		m_textureFont = Shared_Texture(engine, "font.tga", GL_TEXTURE_2D, true, true);

		// Generate vertex array
		glCreateVertexArrays(1, &m_vaoID);
		glEnableVertexArrayAttrib(m_vaoID, 0);
		glVertexArrayAttribBinding(m_vaoID, 0, 0);
		glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glCreateBuffers(1, &m_vboID);
		glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(glm::vec3));
		constexpr auto num_data = 2 * 3;
		glNamedBufferStorage(m_vboID, num_data * sizeof(glm::vec3), 0, GL_DYNAMIC_STORAGE_BIT);
		std::vector<glm::vec3> data(num_data);
		data[0] = { -1, -1, 0 };
		data[1] = { 1, -1, 0 };
		data[2] = { 1,  1, 0 };
		data[3] = { 1,  1, 0 };
		data[4] = { -1,  1, 0 };
		data[5] = { -1, -1, 0 };
		glNamedBufferSubData(m_vboID, 0, num_data * sizeof(glm::vec3), &data[0]);
		m_indirect = IndirectDraw((GLuint)num_data, 1, 0, GL_DYNAMIC_STORAGE_BIT);

		// Configure THIS element
		setText(text);
		setTextScale(m_textScale);
	}


	// Public Interface Implementation
	inline virtual void renderElement(const float & deltaTime, const glm::vec2 & position, const glm::vec2 & scale) override {
		// Exit Early
		if (!getVisible() || !m_shader->existsYet() || !m_textureFont->existsYet()) return;

		// Render
		const glm::vec2 newPosition = position + m_position;
		const glm::vec2 newScale = glm::min(m_scale, scale);
		m_shader->bind();
		m_shader->setUniform(0, newPosition);
		m_shader->setUniform(1, newScale);
		m_shader->setUniform(2, std::clamp<float>((getScale().x / getText().size()) * 2.0f, 5.0f, m_textScale));
		m_shader->setUniform(3, (int)m_textAlignment);
		m_shader->setUniform(4, m_enabled);
		m_shader->setUniform(5, m_color);
		m_textureFont->bind(0);
		m_bufferString.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
		glBindVertexArray(m_vaoID);
		m_indirect.drawCall();

		// Render Children
		UI_Element::renderElement(deltaTime, position, scale);
	}


	// Public Methods
	/** Set this label element's text. 
	@param	text	the text to use. */
	inline void setText(const std::string & text) {
		m_text = text;

		// Write letters to a buffer
		const GLuint count = (GLuint)m_text.size();
		std::vector<int> data(size_t(count) + 1ull);
		data[0] = (int)count;
		for (size_t x = 0; x < (size_t)count; ++x)
			data[x + 1ull] = (int)(m_text[x]) - 32;
		m_bufferString.write_immediate(0, sizeof(int)*(int(count + 1u)), data.data());
		m_indirect.setPrimitiveCount(count);

		// Notify text changed
		enactCallback(on_textChanged);
	}
	/** Retrieve this label's text. 
	@return	the text this label uses. */
	inline std::string getText() const {
		return m_text;
	}
	/** Set this label element's text scaling factor.
	@param	text	the new scaling factor to use. */
	inline void setTextScale(const float & textScale) {
		m_textScale = textScale;
		m_maxScale.y = textScale;
	}
	/** Retrieve this label's text scaling factor.
	@return	the text scaling factor. */
	inline float getTextScale() const {
		return m_textScale;
	}
	/** Set this label's color.
	@param	text	the new color to render with. */
	inline void setColor(const glm::vec3 & color) {
		m_color = color;
	}
	/** Retrieve this label's color.
	@return	the color used by this element. */
	inline glm::vec3 getColor() const {
		return m_color;
	}
	/** Set this label element's alignment.
	@param	text	the alignment (left, center, right). */
	inline void setAlignment(const Alignment & alignment) {
		m_textAlignment = alignment;
	}
	/** Retrieve this label's alignment.
	@return	the alignment. */
	inline Alignment getAlignment() const {
		return m_textAlignment;
	}


protected:
	// Protected Attributes
	std::string m_text = "";
	float m_textScale = 10.0f;
	glm::vec3 m_color = glm::vec3(1.0f);
	Alignment m_textAlignment = align_left;
	GLuint m_vaoID = 0, m_vboID = 0;
	Shared_Shader m_shader;
	Shared_Texture m_textureFont;
	IndirectDraw m_indirect;
	DynamicBuffer m_bufferString;
};

#endif // UI_LABEL_H