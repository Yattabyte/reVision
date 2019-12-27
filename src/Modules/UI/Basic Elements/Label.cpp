#include "Modules/UI/Basic Elements/Label.h"
#include <algorithm>


Label::~Label() 
{
	// Delete geometry
	glDeleteBuffers(1, &m_vboID);
	glDeleteVertexArrays(1, &m_vaoID);
}

Label::Label(Engine& engine, const std::string& text) :
	UI_Element(engine),
	m_shader(Shared_Shader(engine, "UI\\Label")),
	m_textureFont(Shared_Texture(engine, "font.tga", GL_TEXTURE_2D, true, true))
{
	// Generate vertex array
	glCreateVertexArrays(1, &m_vaoID);
	glEnableVertexArrayAttrib(m_vaoID, 0);
	glVertexArrayAttribBinding(m_vaoID, 0, 0);
	glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glCreateBuffers(1, &m_vboID);
	glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(glm::vec3));
	constexpr auto num_data = 2 * 3;
	glNamedBufferStorage(m_vboID, num_data * sizeof(glm::vec3), nullptr, GL_DYNAMIC_STORAGE_BIT);
	std::vector<glm::vec3> data(num_data);
	data[0] = { -1, -1, 0 };
	data[1] = { 1, -1, 0 };
	data[2] = { 1,  1, 0 };
	data[3] = { 1,  1, 0 };
	data[4] = { -1,  1, 0 };
	data[5] = { -1, -1, 0 };
	glNamedBufferSubData(m_vboID, 0, num_data * sizeof(glm::vec3), &data[0]);
	m_indirect = IndirectDraw<>(static_cast<GLuint>(num_data), 1, 0, GL_DYNAMIC_STORAGE_BIT);

	// Configure THIS element
	setText(text);
	setTextScale(m_textScale);
}

void Label::renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) 
{
	// Exit Early
	if (!getVisible() || !Asset::All_Ready(m_shader, m_textureFont)) 
		return;

	// Update indirect draw call
	m_indirect.beginWriting();
	m_indirect.setPrimitiveCount(m_charCount);
	m_indirect.endWriting();

	// Render
	const glm::vec2 newPosition = position + m_position;
	const glm::vec2 newScale = glm::min(m_scale, scale);
	m_shader->bind();
	m_shader->setUniform(0, newPosition);
	m_shader->setUniform(1, newScale);
	m_shader->setUniform(2, std::clamp<float>((getScale().x / getText().size()) * 2.0F, 5.0F, m_textScale));
	m_shader->setUniform(3, static_cast<int>(m_textAlignment));
	m_shader->setUniform(4, m_enabled);
	m_shader->setUniform(5, m_color);
	m_textureFont->bind(0);
	m_bufferString.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
	glBindVertexArray(m_vaoID);
	m_indirect.drawCall();
	m_indirect.endReading();
	Shader::Release();

	// Render Children
	UI_Element::renderElement(deltaTime, position, scale);
}

void Label::setText(const std::string& text) 
{
	m_text = text;

	// Write letters to a buffer
	const auto count = m_text.size();
	std::vector<int> data(count + 1ULL);
	data[0] = static_cast<int>(count);
	for (size_t x = 0; x < count; ++x)
		data[x + 1ULL] = static_cast<int>(m_text[x]) - 32;
	m_bufferString.write_immediate(0, sizeof(int) * (count + 1ULL), data.data());
	m_charCount = static_cast<GLuint>(count);

	// Notify text changed
	enactCallback(static_cast<int>(Label::Interact::on_textChanged));
}

std::string Label::getText() const 
{
	return m_text;
}

void Label::setTextScale(const float& textScale) noexcept
{
	m_textScale = textScale;
	m_maxScale.y = textScale;
}

float Label::getTextScale() const noexcept 
{
	return m_textScale;
}

void Label::setColor(const glm::vec3& color) noexcept 
{
	m_color = color;
}

glm::vec3 Label::getColor() const noexcept
{
	return m_color;
}

void Label::setAlignment(const Alignment& alignment) noexcept
{
	m_textAlignment = alignment;
}

Label::Alignment Label::getAlignment() const noexcept
{
	return m_textAlignment;
}