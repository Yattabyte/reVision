#pragma once
#ifndef UI_BORDER_H
#define UI_BORDER_H

#include "Modules/UI/Decorators/UI_Decorator.h"
#include <memory>


/** Border decorator object*/
class Border : public UI_Decorator
{
public:
	// Public (de)Constructors
	~Border() {
		// Delete geometry
		glDeleteBuffers(1, &m_vboID);
		glDeleteVertexArrays(1, &m_vaoID);
	}
	Border(Engine * engine, const std::shared_ptr<UI_Element> & component) : UI_Decorator(component) {
		// Asset Loading
		m_shader = Shared_Shader(engine, "UI\\Border");

		// Generate vertex array
		glCreateVertexArrays(1, &m_vaoID);
		glEnableVertexArrayAttrib(m_vaoID, 0);
		glVertexArrayAttribBinding(m_vaoID, 0, 0);
		glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glCreateBuffers(1, &m_vboID);
		glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(glm::vec3));
		constexpr auto num_data = 8 * 3;
		glNamedBufferStorage(m_vboID, num_data * sizeof(glm::vec3), 0, GL_DYNAMIC_STORAGE_BIT);
		const GLuint quad[4] = { (GLuint)num_data, 1, 0, 0 };
		m_indirect = StaticBuffer(sizeof(GLuint) * 4, quad, GL_CLIENT_STORAGE_BIT);
	}


	// Public Interface Implementations
	virtual void update() override {
		constexpr auto num_data = 8 * 3;
		std::vector<glm::vec3> m_data(num_data);

		// Bottom Bar
		m_data[0] = { -m_scale.x, -m_scale.y, 0 };
		m_data[1] = { m_scale.x, -m_scale.y, 0 };
		m_data[2] = { m_scale.x, -m_scale.y + m_borderSize, 0 };
		m_data[3] = { m_scale.x, -m_scale.y + m_borderSize, 0 };
		m_data[4] = { -m_scale.x, -m_scale.y + m_borderSize, 0 };
		m_data[5] = { -m_scale.x, -m_scale.y, 0 };

		// Left Bar
		m_data[6] = { -m_scale.x, -m_scale.y + m_borderSize, 0 };
		m_data[7] = { -m_scale.x + m_borderSize, -m_scale.y + m_borderSize, 0 };
		m_data[8] = { -m_scale.x + m_borderSize, m_scale.y - m_borderSize, 0 };
		m_data[9] = { -m_scale.x + m_borderSize, m_scale.y - m_borderSize, 0 };
		m_data[10] = { -m_scale.x, m_scale.y - m_borderSize, 0 };
		m_data[11] = { -m_scale.x, -m_scale.y + m_borderSize, 0 };

		// Top Bar
		m_data[12] = { -m_scale.x, m_scale.y - m_borderSize, 0 };
		m_data[13] = { m_scale.x, m_scale.y - m_borderSize, 0 };
		m_data[14] = { m_scale.x, m_scale.y, 0 };
		m_data[15] = { m_scale.x, m_scale.y, 0 };
		m_data[16] = { -m_scale.x, m_scale.y, 0 };
		m_data[17] = { -m_scale.x, m_scale.y - m_borderSize, 0 };

		// Right Bar
		m_data[18] = { m_scale.x - m_borderSize, -m_scale.y + m_borderSize, 0 };
		m_data[19] = { m_scale.x, -m_scale.y + m_borderSize, 0 };
		m_data[20] = { m_scale.x, m_scale.y - m_borderSize, 0 };
		m_data[21] = { m_scale.x, m_scale.y - m_borderSize, 0 };
		m_data[22] = { m_scale.x - m_borderSize, m_scale.y - m_borderSize, 0 };
		m_data[23] = { m_scale.x - m_borderSize, -m_scale.y + m_borderSize, 0 };

		glNamedBufferSubData(m_vboID, 0, num_data * sizeof(glm::vec3), &m_data[0]);

		UI_Element::update();
		m_component->setScale(getScale() - m_borderSize);
	}
	virtual void renderElement(const float & deltaTime, const glm::vec2 & position, const glm::vec2 & scale) override {
		if (!getVisible()) return;
		const auto newPosition = position + m_position;
		const auto newScale = glm::min(m_scale, scale);
		if (m_shader->existsYet()) {
			m_shader->bind();
			m_shader->setUniform(0, newPosition);
			m_shader->setUniform(1, m_borderColor);
			glBindVertexArray(m_vaoID);
			m_indirect.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
		}
		UI_Decorator::renderElement(deltaTime, position, newScale);
	}


	// Public Methods
	/** Set the border size.
	@param		size		the new border size to use. */
	void setBorderSize(const float & size) {
		m_borderSize = size;
		update();
	}
	/** Set the border color.
		@param		size		the new border color to use. */
	void setBorderColor(const glm::vec3 & color) {
		m_borderColor = color;
	}
	

protected:
	// Protected Attributes
	float m_borderSize = 1.0f;
	glm::vec3 m_borderColor = glm::vec3(1.0f);


private:
	// Private Attributes
	GLuint m_vaoID = 0, m_vboID = 0;
	Shared_Shader m_shader;
	StaticBuffer m_indirect;
};

#endif // UI_BORDER_H