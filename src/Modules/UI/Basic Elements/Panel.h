#pragma once
#ifndef UI_PANEL_H
#define UI_PANEL_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Assets/Shader.h"
#include "Utilities/GL/IndirectDraw.h"


/** UI panel class, affords containing other elements, and rendering a fixed color. */
class Panel : public UI_Element {
public:
	// Public (de)Constructors
	/** Destroy the panel. */
	inline ~Panel() {
		// Delete geometry
		glDeleteBuffers(1, &m_vboID);
		glDeleteVertexArrays(1, &m_vaoID);
	}
	/** Construct a panel.
	@param	engine		the engine to use. */
	inline explicit Panel(Engine* engine) :
		UI_Element(engine),
		m_shader(Shared_Shader(engine, "UI\\Panel"))
	{
		// Generate vertex array
		glCreateVertexArrays(1, &m_vaoID);
		glEnableVertexArrayAttrib(m_vaoID, 0);
		glVertexArrayAttribBinding(m_vaoID, 0, 0);
		glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribIFormat(m_vaoID, 1, 1, GL_INT, 0);
		glCreateBuffers(1, &m_vboID);
		glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(glm::vec3));
		constexpr auto num_data = 2 * 3;
		glNamedBufferStorage(m_vboID, num_data * sizeof(glm::vec3), 0, GL_DYNAMIC_STORAGE_BIT);
		m_indirect = IndirectDraw<1>((GLuint)num_data, 1, 0, GL_CLIENT_STORAGE_BIT);

		// Add Callbacks
		addCallback(UI_Element::on_resize, [&]() { updateGeometry(); });
	}


	// Public Interface Implementation
	inline virtual void renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) override {
		// Exit Early
		if (!getVisible() || !m_shader->existsYet()) return;

		// Render
		const glm::vec2 newPosition = position + m_position;
		m_shader->bind();
		m_shader->setUniform(0, newPosition);
		m_shader->setUniform(1, m_color);
		glBindVertexArray(m_vaoID);
		m_indirect.drawCall();
		Shader::Release();

		// Render Children
		UI_Element::renderElement(deltaTime, position, scale);
	}


	// Public Methods
	/** Set this panel's color.
	@param	text	the new color to render with. */
	inline void setColor(const glm::vec4& color) {
		m_color = color;
	}
	/** Retrieve this panel's color.
	@return	the color used by this element. */
	inline glm::vec4 getColor() const {
		return m_color;
	}


protected:
	// Protected Methods
	/** Update the data dependant on the scale of this element. */
	inline void updateGeometry() {
		constexpr auto num_data = 2 * 3;
		std::vector<glm::vec3> data(num_data);

		// Center
		data[0] = { -1, -1, 0 };
		data[1] = { 1, -1, 0 };
		data[2] = { 1,  1, 0 };
		data[3] = { 1,  1, 0 };
		data[4] = { -1,  1, 0 };
		data[5] = { -1, -1, 0 };
		for (int x = 0; x < 6; ++x)
			data[x] *= glm::vec3(m_scale, 0.0f);
		glNamedBufferSubData(m_vboID, 0, num_data * sizeof(glm::vec3), &data[0]);
	}


	// Protected Attributes
	glm::vec4 m_color = glm::vec4(0.2f);
	GLuint m_vaoID = 0, m_vboID = 0;
	Shared_Shader m_shader;
	IndirectDraw<1> m_indirect;
};

#endif // UI_PANEL_H