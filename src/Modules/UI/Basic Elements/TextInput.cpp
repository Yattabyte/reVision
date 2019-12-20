#include "Modules/UI/Basic Elements/TextInput.h"
#include <algorithm>


TextInput::~TextInput() noexcept
{
	// Delete geometry
	glDeleteBuffers(2, m_vboID);
	glDeleteVertexArrays(1, &m_vaoID);
}

TextInput::TextInput(Engine& engine) :
	UI_Element(engine),
	m_shader(Shared_Shader(engine, "UI\\TextInput")),
	m_label(std::make_shared<Label>(engine))
{
	// Label
	m_label->setAlignment(Label::Alignment::align_left);
	m_label->setColor(glm::vec3(0.0f));
	addElement(m_label);

	// Callbacks
	addCallback((int)UI_Element::Interact::on_resize, [&]() {
		m_label->setScale(getScale());
		updateGeometry();
		});

	// Generate vertex array
	glCreateVertexArrays(1, &m_vaoID);
	glEnableVertexArrayAttrib(m_vaoID, 0);
	glEnableVertexArrayAttrib(m_vaoID, 1);
	glVertexArrayAttribBinding(m_vaoID, 0, 0);
	glVertexArrayAttribBinding(m_vaoID, 1, 1);
	glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribIFormat(m_vaoID, 1, 1, GL_INT, 0);
	glCreateBuffers(2, m_vboID);
	glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID[0], 0, sizeof(glm::vec3));
	glVertexArrayVertexBuffer(m_vaoID, 1, m_vboID[1], 0, sizeof(int));
	constexpr auto num_data = 4 * 3;
	glNamedBufferStorage(m_vboID[0], num_data * sizeof(glm::vec3), nullptr, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferStorage(m_vboID[1], num_data * sizeof(int), nullptr, GL_DYNAMIC_STORAGE_BIT);
	m_indirect = IndirectDraw<1>((GLuint)num_data, 1, 0, GL_CLIENT_STORAGE_BIT);
}

void TextInput::mouseAction(const MouseEvent& mouseEvent) 
{
	UI_Element::mouseAction(mouseEvent);
	if (getVisible() && getEnabled() && mouseWithin(mouseEvent)) {
		if (m_clicked) {
			// If already editing, move caret to mouse position
			if (m_edit) {
				const int mx = int(float(mouseEvent.m_xPos) - m_position.x + m_scale.x);
				setCaret((size_t)std::roundf(float(mx) / 10.0f));
			}
			m_edit = true;
			m_clicked = false;
			return;
		}
	}
	else
		m_edit = false;
}

void TextInput::keyboardAction(const KeyboardEvent& keyboardEvent) 
{
	if (m_edit) {
		// Check for a text stream
		if (const auto character = keyboardEvent.getChar()) {
			setText(m_text.substr(0, m_caretIndex) + char(character) + m_text.substr(m_caretIndex, m_text.size()));
			setCaret(size_t(m_caretIndex) + 1);
			enactCallback((int)TextInput::Interact::on_text_change);
		}
		// Otherwise, check keyboard states
		else {
			if ((int)keyboardEvent.getState(KeyboardEvent::Key::ENTER) || (int)keyboardEvent.getState(KeyboardEvent::Key::ESCAPE))
				m_edit = false;
			else if ((int)keyboardEvent.getState(KeyboardEvent::Key::BACKSPACE)) {
				if (m_caretIndex > 0) {
					setText(m_text.substr(0, size_t(m_caretIndex) - 1) + m_text.substr(m_caretIndex, m_text.size()));
					setCaret(size_t(m_caretIndex) - 1);
					enactCallback((int)TextInput::Interact::on_text_change);
				}
			}
			else if ((int)keyboardEvent.getState(KeyboardEvent::Key::DEL)) {
				if (size_t(m_caretIndex) + 1 <= m_text.size()) {
					setText(m_text.substr(0, m_caretIndex) + m_text.substr(size_t(m_caretIndex) + 1, m_text.size()));
					enactCallback((int)TextInput::Interact::on_text_change);
				}
			}
			else if ((int)keyboardEvent.getState(KeyboardEvent::Key::LEFT))
				setCaret(size_t(m_caretIndex) - 1);
			else if ((int)keyboardEvent.getState(KeyboardEvent::Key::RIGHT))
				setCaret(size_t(m_caretIndex) + 1);
		}
	}
}

void TextInput::renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale)
{
	// Exit Early
	if (!getVisible() || !m_shader->ready())
		return;

	const glm::vec2 newPosition = position + m_position;
	//const glm::vec2 newScale = glm::min(m_scale, scale);

	// Render (background)
	m_shader->bind();
	m_shader->setUniform(0, newPosition);
	m_shader->setUniform(1, m_enabled);
	m_shader->setUniform(2, m_edit);
	m_shader->setUniform(3, m_blinkTime += deltaTime);
	glBindVertexArray(m_vaoID);
	m_indirect.drawCall();
	Shader::Release();

	// Render Children (text)
	UI_Element::renderElement(deltaTime, position, scale);
}

void TextInput::setText(const std::string& text) 
{
	m_text = text;
	m_label->setText(text);
}

std::string TextInput::getText() const 
{
	return m_text;
}

void TextInput::setCaret(const size_t& index) 
{
	m_caretIndex = (int)std::clamp<size_t>(index, 0, m_text.size());
	updateGeometry();
}

void TextInput::updateGeometry() 
{
	constexpr auto num_data = 4 * 3;
	std::vector<glm::vec3> data(num_data);
	std::vector<int> objIndices(num_data);

	for (size_t x = 0; x < 12; x += 6) {
		data[x + 0] = { -1, -1, 0 };
		data[x + 1] = { 1, -1, 0 };
		data[x + 2] = { 1,  1, 0 };
		data[x + 3] = { 1,  1, 0 };
		data[x + 4] = { -1,  1, 0 };
		data[x + 5] = { -1, -1, 0 };
	}
	for (auto x = 0; x < 6; ++x) {
		data[x] *= glm::vec3(m_scale, 0.0f);
		objIndices[x] = 0;
	}
	for (auto x = 6; x < 12; ++x) {
		data[x] *= glm::vec3(1.0, 10, 1);
		data[x].x = (data[x].x - m_scale.x) + (10.0f * m_caretIndex);
		objIndices[x] = 1;
	}

	glNamedBufferSubData(m_vboID[0], 0, num_data * sizeof(glm::vec3), &data[0]);
	glNamedBufferSubData(m_vboID[1], 0, num_data * sizeof(int), &objIndices[0]);
}