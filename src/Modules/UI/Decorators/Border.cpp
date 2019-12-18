#include "Modules/UI/Decorators/Border.h"


Border::~Border() noexcept 
{
	// Delete geometry
	glDeleteBuffers(1, &m_vboID);
	glDeleteVertexArrays(1, &m_vaoID);
}

Border::Border(Engine& engine, const std::shared_ptr<UI_Element>& component) :
	UI_Decorator(engine, component),
	m_shader(Shared_Shader(engine, "UI\\Border"))
{
	// Generate vertex array
	glCreateVertexArrays(1, &m_vaoID);
	glEnableVertexArrayAttrib(m_vaoID, 0);
	glVertexArrayAttribBinding(m_vaoID, 0, 0);
	glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glCreateBuffers(1, &m_vboID);
	glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(glm::vec3));
	constexpr auto num_data = 8 * 3;
	glNamedBufferStorage(m_vboID, num_data * sizeof(glm::vec3), nullptr, GL_DYNAMIC_STORAGE_BIT);
	m_indirect = IndirectDraw<1>((GLuint)num_data, 1, 0, GL_CLIENT_STORAGE_BIT);

	// Add Callbacks
	addCallback((int)UI_Element::Interact::on_resize, [&]() noexcept { updateGeometry(); });
}

void Border::renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) noexcept 
{
	// Exit Early
	if (!getVisible() || !m_shader->ready())
		return;

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

void Border::setBorderSize(const float& size) noexcept 
{
	m_borderSize = size;
	updateGeometry();
}

float Border::getBorderSize() const noexcept 
{
	return m_borderSize;
}

void Border::setBorderColor(const glm::vec3& color) noexcept 
{
	m_borderColor = color;
}

glm::vec3 Border::getBorderColor() const noexcept 
{
	return m_borderColor;
}

void Border::updateGeometry() 
{
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