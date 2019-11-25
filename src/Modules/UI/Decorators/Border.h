#pragma once
#ifndef UI_BORDER_H
#define UI_BORDER_H

#include "Modules/UI/Decorators/UI_Decorator.h"
#include "Utilities/GL/IndirectDraw.h"


/** Border decorator object. */
class Border : public UI_Decorator {
public:
	// Public (De)Constructors
	/** Destroy the border. */
	inline ~Border() noexcept {
		// Delete geometry
		glDeleteBuffers(1, &m_vboID);
		glDeleteVertexArrays(1, &m_vaoID);
	}
	/** Construct a border, decorating the supplied component.
	@param	engine		the engine to use.
	@param	component	the component to decorate. */
	inline Border(Engine& engine, const std::shared_ptr<UI_Element>& component) noexcept :
		UI_Decorator(engine, component),
		m_shader(Shared_Shader(engine, "UI\\Border")) {
		// Generate vertex array
		glCreateVertexArrays(1, &m_vaoID);
		glEnableVertexArrayAttrib(m_vaoID, 0);
		glVertexArrayAttribBinding(m_vaoID, 0, 0);
		glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glCreateBuffers(1, &m_vboID);
		glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(glm::vec3));
		constexpr auto num_data = 8 * 3;
		glNamedBufferStorage(m_vboID, num_data * sizeof(glm::vec3), 0, GL_DYNAMIC_STORAGE_BIT);
		m_indirect = IndirectDraw<1>((GLuint)num_data, 1, 0, GL_CLIENT_STORAGE_BIT);

		// Add Callbacks
		addCallback((int)UI_Element::Interact::on_resize, [&]() { updateGeometry(); });
	}


	// Public Interface Implementations
	inline virtual void renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) noexcept override {
		// Exit Early
		if (!getVisible() || !m_shader->existsYet()) return;
		const glm::vec2 newPosition = position + m_position;
		const glm::vec2 newScale = glm::min(m_scale, scale);

		// Render
		m_shader->bind();
		m_shader->setUniform(0, newPosition);
		m_shader->setUniform(1, m_borderColor);
		glBindVertexArray(m_vaoID);
		m_indirect.drawCall();
		Shader::Release();

		// Render Children
		UI_Decorator::renderElement(deltaTime, position, newScale);
	}


	// Public Methods
	/** Set the border size for this decorator.
	@param		size		the new border size to use. */
	inline void setBorderSize(const float& size) noexcept {
		m_borderSize = size;
		updateGeometry();
	}
	/** Retrieve the border size of this decorator.
	@return					the size of the border this decorator uses. */
	inline float getBorderSize() const noexcept {
		return m_borderSize;
	}
	/** Set the border color.
	@param		size		the new border color to use. */
	inline void setBorderColor(const glm::vec3& color) noexcept {
		m_borderColor = color;
	}
	/** Retrieve the border color of this decorator.
	@return					the color of the border this decorator uses. */
	inline glm::vec3 getBorderColor() const noexcept {
		return m_borderColor;
	}


protected:
	// Protected Methods
	inline void updateGeometry() noexcept {
		constexpr auto num_data = 8 * 3;
		std::vector<glm::vec3> data(num_data);

		// Bottom Bar
		data[0] = { -m_scale.x - m_borderSize, -m_scale.y, 0 };
		data[1] = { m_scale.x + m_borderSize, -m_scale.y, 0 };
		data[2] = { m_scale.x + m_borderSize, -m_scale.y + m_borderSize, 0 };
		data[3] = { m_scale.x + m_borderSize, -m_scale.y + m_borderSize, 0 };
		data[4] = { -m_scale.x - m_borderSize, -m_scale.y + m_borderSize, 0 };
		data[5] = { -m_scale.x - m_borderSize, -m_scale.y, 0 };

		// Left Bar
		data[6] = { -m_scale.x, -m_scale.y - m_borderSize, 0 };
		data[7] = { -m_scale.x + m_borderSize, -m_scale.y - m_borderSize, 0 };
		data[8] = { -m_scale.x + m_borderSize, m_scale.y + m_borderSize, 0 };
		data[9] = { -m_scale.x + m_borderSize, m_scale.y + m_borderSize, 0 };
		data[10] = { -m_scale.x, m_scale.y + m_borderSize, 0 };
		data[11] = { -m_scale.x, -m_scale.y - m_borderSize, 0 };

		// Top Bar
		data[12] = { -m_scale.x - m_borderSize, m_scale.y - m_borderSize, 0 };
		data[13] = { m_scale.x + m_borderSize, m_scale.y - m_borderSize, 0 };
		data[14] = { m_scale.x + m_borderSize, m_scale.y, 0 };
		data[15] = { m_scale.x + m_borderSize, m_scale.y, 0 };
		data[16] = { -m_scale.x - m_borderSize, m_scale.y, 0 };
		data[17] = { -m_scale.x - m_borderSize, m_scale.y - m_borderSize, 0 };

		// Right Bar
		data[18] = { m_scale.x - m_borderSize, -m_scale.y - m_borderSize, 0 };
		data[19] = { m_scale.x, -m_scale.y - m_borderSize, 0 };
		data[20] = { m_scale.x, m_scale.y + m_borderSize, 0 };
		data[21] = { m_scale.x, m_scale.y + m_borderSize, 0 };
		data[22] = { m_scale.x - m_borderSize, m_scale.y + m_borderSize, 0 };
		data[23] = { m_scale.x - m_borderSize, -m_scale.y - m_borderSize, 0 };

		glNamedBufferSubData(m_vboID, 0, num_data * sizeof(glm::vec3), &data[0]);

		m_component->setScale(getScale() - m_borderSize);
	}


	// Protected Attributes
	float m_borderSize = 2.0f;
	glm::vec3 m_borderColor = glm::vec3(1.0f);
	GLuint m_vaoID = 0, m_vboID = 0;
	Shared_Shader m_shader;
	IndirectDraw<1> m_indirect;
};

#endif // UI_BORDER_H
