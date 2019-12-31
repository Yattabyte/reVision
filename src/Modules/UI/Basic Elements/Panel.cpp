#include "Modules/UI/Basic Elements/Panel.h"


Panel::~Panel()
{
	// Delete geometry
	glDeleteBuffers(1, &m_vboID);
	glDeleteVertexArrays(1, &m_vaoID);
}

Panel::Panel(Engine& engine) :
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
	glNamedBufferStorage(m_vboID, num_data * sizeof(glm::vec3), nullptr, GL_DYNAMIC_STORAGE_BIT);
	m_indirect = IndirectDraw<1>(static_cast<GLuint>(num_data), 1, 0, GL_CLIENT_STORAGE_BIT);

	// Add Callbacks
	addCallback(static_cast<int>(UI_Element::Interact::on_resize), [&] { updateGeometry(); });
}

void Panel::renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale)
{
	// Exit Early
	if (!getVisible() || !m_shader->ready())
		return;

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

void Panel::setColor(const glm::vec4& color) noexcept
{
	m_color = color;
}

glm::vec4 Panel::getColor() const noexcept
{
	return m_color;
}

void Panel::updateGeometry()
{
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
		data[x] *= glm::vec3(m_scale, 0.0F);
	glNamedBufferSubData(m_vboID, 0, num_data * sizeof(glm::vec3), &data[0]);
}