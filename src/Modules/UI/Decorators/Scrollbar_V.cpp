#include "Modules/UI/Decorators/Scrollbar_V.h"
#include <algorithm>


Scrollbar_V::~Scrollbar_V()
{
	// Delete geometry
	glDeleteBuffers(1, &m_vboID);
	glDeleteVertexArrays(1, &m_vaoID);
}

Scrollbar_V::Scrollbar_V(Engine& engine, const std::shared_ptr<UI_Element>& component) :
	UI_Decorator(engine, component),
	m_shader(Shared_Shader(engine, "UI\\ScrollBar"))
{
	auto topButton = std::make_shared<Button>(engine);
	auto bottomButton = std::make_shared<Button>(engine);
	auto panel = std::make_shared<Button>(engine);
	addElement(topButton);
	addElement(bottomButton);
	addElement(panel);
	topButton->setBevelRadius(0.0F);
	topButton->setMinScale(glm::vec2(12.5F));
	topButton->setMaxScale(glm::vec2(12.5F));
	bottomButton->setBevelRadius(0.0F);
	bottomButton->setMinScale(glm::vec2(12.5F));
	bottomButton->setMaxScale(glm::vec2(12.5F));
	panel->setMinScale(glm::vec2(12.5F, 12.5F));
	panel->setMaxScale(glm::vec2(12.5F, m_scale.y - 25.0F - 25.0F));
	setMinWidth(12.5F);

	// Generate vertex array
	glCreateVertexArrays(1, &m_vaoID);
	glEnableVertexArrayAttrib(m_vaoID, 0);
	glVertexArrayAttribBinding(m_vaoID, 0, 0);
	glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glCreateBuffers(1, &m_vboID);
	glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(glm::vec3));
	constexpr auto num_data = 2 * 3;
	glNamedBufferStorage(m_vboID, num_data * sizeof(glm::vec3), nullptr, GL_DYNAMIC_STORAGE_BIT);
	m_indirect = IndirectDraw<1>(static_cast<GLuint>(num_data), 1, 0, GL_CLIENT_STORAGE_BIT);

	// Add Callbacks
	addCallback(static_cast<int>(UI_Element::Interact::on_resize), [&] { updateGeometry(); });
}

void Scrollbar_V::mouseAction(const MouseEvent& mouseEvent)
{
	UI_Decorator::mouseAction(mouseEvent);
	if (getVisible() && getEnabled() && mouseWithin(mouseEvent)) {
		MouseEvent subEvent = mouseEvent;
		//subEvent.m_xPos = mouseEvent.m_xPos - m_position.x;
		subEvent.m_yPos = mouseEvent.m_yPos - m_position.y;
		if (m_children.size() == 3) {
			if (std::dynamic_pointer_cast<Button>(m_children[2])->getPressed() && mouseEvent.m_action == MouseEvent::Action::MOVE)
				setLinear(float(subEvent.m_yPos) / (m_scale.y - 25.0F - 12.5F));
		}
		enactCallback(static_cast<int>(UI_Element::Interact::on_hover_start));
		if (mouseEvent.m_action == MouseEvent::Action::PRESS)
			enactCallback(static_cast<int>(UI_Element::Interact::on_press));
		else
			enactCallback(static_cast<int>(UI_Element::Interact::on_release));
	}
}

void Scrollbar_V::renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale)
{
	// Quit Early
	if (!getVisible() || !m_shader->ready())
		return;

	const auto newPosition = position + m_position;
	const auto newScale = glm::min(m_scale, scale);

	// Render
	m_shader->bind();
	m_shader->setUniform(0, newPosition);
	glBindVertexArray(m_vaoID);
	m_indirect.drawCall();
	Shader::Release();

	// Render Children
	UI_Decorator::renderElement(deltaTime, position, newScale);
}

void Scrollbar_V::setLinear(const float& linear)
{
	m_linear = std::clamp<float>(linear, -1.0F, 1.0F);
	updateElementPosition();
	enactCallback(static_cast<int>(Scrollbar_V::Interact::on_scroll_change));
}

float Scrollbar_V::getLinear() const noexcept
{
	return m_linear;
}

void Scrollbar_V::updateGeometry()
{
	constexpr auto num_data = 2 * 3;
	std::vector<glm::vec3> data(num_data);

	// Background
	data[0] = { -1, -1, 0 };
	data[1] = { 1, -1, 0 };
	data[2] = { 1,  1, 0 };
	data[3] = { 1,  1, 0 };
	data[4] = { -1,  1, 0 };
	data[5] = { -1, -1, 0 };
	for (int x = 0; x < 6; ++x)
		data[x] = glm::vec3(m_scale.x - 12.5F, 0, 0) + (data[x] * glm::vec3(12.5F, m_scale.y, 0.0F));

	glNamedBufferSubData(m_vboID, 0, num_data * sizeof(glm::vec3), &data[0]);

	updateElementPosition();
	m_component->setPosition(glm::vec2(-12.5F, 0));
	m_component->setScale(glm::vec2(m_scale.x - 12.5F, m_scale.y));
}

void Scrollbar_V::updateElementPosition()
{
	if (m_children.size() == 3) {
		// Buttons
		m_children[0]->setPosition(glm::vec2(getScale() - 12.5F));
		m_children[1]->setPosition(glm::vec2(getScale().x - 12.5F, -getScale().y + 12.5F));

		// Panel
		m_children[2]->setMaxScale(glm::vec2(12.5F, m_scale.y - 25.0F - 12.5F));
		m_children[2]->setPosition(glm::vec2(getScale().x - 12.5F, m_linear * (m_scale.y - 25.0F - 12.5F)));
	}
}