#include "Modules/UI/Basic Elements/Separator.h"


Separator::~Separator() noexcept 
{
	// Delete geometry
	glDeleteBuffers(1, &m_vboID);
	glDeleteVertexArrays(1, &m_vaoID);
}

Separator::Separator(Engine& engine) noexcept :
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

void Separator::renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) noexcept 
{
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

void Separator::setColor(const glm::vec4& color) noexcept 
{
	m_color = color;
}

glm::vec4 Separator::getColor() const noexcept 
{
	return m_color;
}