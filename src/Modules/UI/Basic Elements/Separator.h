#pragma once
#ifndef UI_SEPARATOR_H
#define UI_SEPARATOR_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Assets/Shader.h"
#include "Utilities/GL/StaticBuffer.h"


/** UI separator class. Renders a faded out line across its width. */
class Separator : public UI_Element {
public:
	// Public (de)Constructors
	/** Destroy the separator. */
	inline ~Separator() {
		// Delete geometry
		glDeleteBuffers(1, &m_vboID);
		glDeleteVertexArrays(1, &m_vaoID);
	}
	/** Construct a separator. 
	@param	engine		the engine to use. */
	inline Separator(Engine * engine) : UI_Element(engine) {
		// Asset Loading
		m_shader = Shared_Shader(engine, "UI\\Separator");

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
		const GLuint quad[4] = { (GLuint)num_data, 1, 0, 0 };
		m_indirect = StaticBuffer(sizeof(GLuint) * 4, quad, GL_CLIENT_STORAGE_BIT);
		setMaxScale(glm::vec2(getMaxScale().x, 2.0f));
		setMinScale(glm::vec2(getMinScale().x, 2.0f));
	}


	// Public Interface Implementation
	inline virtual void renderElement(const float & deltaTime, const glm::vec2 & position, const glm::vec2 & scale) override {
		// Exit Early
		if (!getVisible() || !m_shader->existsYet()) return;
		
		// Render
		const glm::vec2 newPosition = position + m_position;
		const glm::vec2 newScale = glm::min(m_scale, scale);
		m_shader->bind();
		m_shader->setUniform(0, newPosition);
		m_shader->setUniform(1, newScale);
		glBindVertexArray(m_vaoID);
		m_indirect.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		
		// Render Children
		UI_Element::renderElement(deltaTime, position, newScale);
	}


protected:
	// Protected Attributes
	GLuint m_vaoID = 0, m_vboID = 0;
	Shared_Shader m_shader;
	StaticBuffer m_indirect;
};

#endif // UI_SEPARATOR_H