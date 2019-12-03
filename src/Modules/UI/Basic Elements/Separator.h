#pragma once
#ifndef UI_SEPARATOR_H
#define UI_SEPARATOR_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Assets/Shader.h"
#include "Utilities/GL/IndirectDraw.h"


/** UI separator class. Renders a faded out line across its width. */
class Separator : public UI_Element {
public:
	// Public (De)Constructors
	/** Destroy the separator. */
	inline ~Separator() noexcept {
		// Delete geometry
		glDeleteBuffers(1, &m_vboID);
		glDeleteVertexArrays(1, &m_vaoID);
	}
	/** Construct a separator.
	@param	engine		reference to the engine to use. */
	inline explicit Separator(Engine& engine) noexcept :
		UI_Element(engine),
		m_shader(Shared_Shader(engine, "UI\\Separator"))
	{
		// Generate vertex array
		glCreateVertexArrays(1, &m_vaoID);
		glEnableVertexArrayAttrib(m_vaoID, 0);
		glVertexArrayAttribBinding(m_vaoID, 0, 0);
		glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glCreateBuffers(1, &m_vboID);
		glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(glm::vec3));
		constexpr auto num_data = 2 * 3;
		std::vector<glm::vec3> m_data(num_data);
		m_data[0] = { -1, -1, 0 };
		m_data[1] = { 1, -1, 0 };
		m_data[2] = { 1,  1, 0 };
		m_data[3] = { 1,  1, 0 };
		m_data[4] = { -1,  1, 0 };
		m_data[5] = { -1, -1, 0 };
		glNamedBufferStorage(m_vboID, num_data * sizeof(glm::vec3), &m_data[0], GL_CLIENT_STORAGE_BIT);
		m_indirect = IndirectDraw<1>((GLuint)num_data, 1, 0, GL_CLIENT_STORAGE_BIT);
		setMaxHeight(2.0f);
		setMinHeight(2.0f);
	}


	// Public Interface Implementation
	inline virtual void renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) noexcept override {
		// Exit Early
		if (!getVisible() || !m_shader->existsYet()) return;

		// Render
		const glm::vec2 newPosition = position + m_position;
		const glm::vec2 newScale = glm::min(m_scale, scale);
		m_shader->bind();
		m_shader->setUniform(0, newPosition);
		m_shader->setUniform(1, newScale);
		m_shader->setUniform(2, m_color);
		glBindVertexArray(m_vaoID);
		m_indirect.drawCall();
		Shader::Release();

		// Render Children
		UI_Element::renderElement(deltaTime, position, scale);
	}


	// Public Methods
	/** Set this element's color.
	@param	text	the new color to render with. */
	inline void setColor(const glm::vec4& color) noexcept {
		m_color = color;
	}
	/** Retrieve this element's color.
	@return			the color used by this element. */
	inline glm::vec4 getColor() const noexcept {
		return m_color;
	}


protected:
	// Protected Attributes
	glm::vec4 m_color = glm::vec4(1.0f);
	GLuint m_vaoID = 0, m_vboID = 0;
	Shared_Shader m_shader;
	IndirectDraw<1> m_indirect;
};

#endif // UI_SEPARATOR_H
