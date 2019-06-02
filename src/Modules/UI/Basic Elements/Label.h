#pragma once
#ifndef UI_LABEL_H
#define UI_LABEL_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Assets/Shader.h"
#include "Assets/Texture.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/DynamicBuffer.h"
#include <string>


/** UI text label class, affords displaying text on the screen. */
class Label : public UI_Element {
public:
	// Public Interaction Enums
	const enum interact {
		on_textChanged = UI_Element::last_interact_index
	};
	// Public Alignment Enums
	const enum Alignment : int {
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
		std::vector<glm::vec3> m_data(num_data);
		m_data[0] = { -1, -1, 0 };
		m_data[1] = { 1, -1, 0 };
		m_data[2] = { 1,  1, 0 };
		m_data[3] = { 1,  1, 0 };
		m_data[4] = { -1,  1, 0 };
		m_data[5] = { -1, -1, 0 };
		glNamedBufferSubData(m_vboID, 0, num_data * sizeof(glm::vec3), &m_data[0]);
		const GLuint quad[4] = { (GLuint)num_data, 1, 0, 0 };
		m_indirect = StaticBuffer(sizeof(GLuint) * 4, quad, GL_DYNAMIC_STORAGE_BIT);

		setText(text);
		setTextScale(m_textScale);
	}


	// Public Interface Implementation
	inline virtual void update() override {
		// Write letters to a buffer
		const GLuint count = (GLuint)m_text.size();
		std::vector<int> data(count + 1);
		data[0] = count;
		for (int x = 0; x < (int)count; ++x)
			data[x + 1] = (int)(m_text[x]) - 32;
		m_bufferString.write(0, sizeof(int)*(count + 1), data.data());
		m_indirect.write(GLsizeiptr(sizeof(GLuint)), GLsizeiptr(sizeof(GLuint)), &count);

		UI_Element::update();
	}
	inline virtual void renderElement(const float & deltaTime, const glm::vec2 & position, const glm::vec2 & scale) override {
		// Exit Early
		if (!getVisible() || !m_shader->existsYet() || !m_textureFont->existsYet()) return;

		// Render
		const glm::vec2 newPosition = position + m_position;
		const glm::vec2 newScale = glm::min(m_scale, scale);
		m_shader->bind();
		m_shader->setUniform(0, newPosition);
		m_shader->setUniform(1, newScale);
		m_shader->setUniform(2, m_textScale);
		m_shader->setUniform(3, (int)m_textAlignment);
		m_shader->setUniform(4, m_enabled);
		m_shader->setUniform(5, m_color);
		m_textureFont->bind(0);
		m_bufferString.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
		glBindVertexArray(m_vaoID);
		m_indirect.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Render Children
		UI_Element::renderElement(deltaTime, position, newScale);
	}


	// Public Methods
	/** Set this label element's text. 
	@param	text	the text to use. */
	inline void setText(const std::string & text) {
		m_text = text;
		update();
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
		update();
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
	StaticBuffer m_indirect;
	DynamicBuffer m_bufferString;
};

#endif // UI_LABEL_H