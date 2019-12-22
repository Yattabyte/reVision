#include "Modules/UI/Basic Elements/SideList.h"
#include "Modules/UI/Basic Elements/Panel.h"
#include <algorithm>


SideList::~SideList() 
{
	// Delete geometry
	glDeleteBuffers(2, m_vboID);
	glDeleteVertexArrays(1, &m_vaoID);
}

SideList::SideList(Engine& engine) :
	UI_Element(engine),
	m_shader(Shared_Shader(engine, "UI\\SideList")),
	m_label(std::make_shared<Label>(engine, ""))
{
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
	constexpr auto num_data = 2 * 3;
	glNamedBufferStorage(m_vboID[0], num_data * sizeof(glm::vec3), nullptr, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferStorage(m_vboID[1], num_data * sizeof(int), nullptr, GL_DYNAMIC_STORAGE_BIT);
	m_indirect = IndirectDraw<1>((GLuint)num_data, 1, 0, GL_CLIENT_STORAGE_BIT);

	// Make a background panel for cosmetic purposes
	auto panel = std::make_shared<Panel>(engine);
	panel->setColor(glm::vec4(0.3f));
	m_backPanel = std::make_shared<Border>(engine, panel);
	addElement(m_backPanel);

	// Other UI elements
	m_label->setAlignment(Label::Alignment::align_center);
	m_label->setColor(glm::vec3(1.0f));
	addElement(m_label);

	// Add Callbacks
	addCallback((int)UI_Element::Interact::on_resize, [&] { updateGeometry(); });

	// Configure THIS element
	setIndex(0);
}

void SideList::mouseAction(const MouseEvent& mouseEvent) 
{
	UI_Element::mouseAction(mouseEvent);
	if (getVisible() && getEnabled() && mouseWithin(mouseEvent)) {
		const float mx = float(mouseEvent.m_xPos) - m_position.x;
		// Left button
		if (mx >= -m_scale.x && mx <= (-m_scale.x + 14) && m_lEnabled) {
			m_lhighlighted = true;
			if (!m_lpressed && mouseEvent.m_action == MouseEvent::Action::PRESS)
				m_lpressed = true;
			else if (m_lpressed && mouseEvent.m_action == MouseEvent::Action::RELEASE) {
				m_lpressed = false;
				setIndex(getIndex() - 1);
			}
		}
		// Right button
		if (mx >= (m_scale.x - 14) && mx <= m_scale.x && m_rEnabled) {
			m_rhighlighted = true;
			if (!m_rpressed && mouseEvent.m_action == MouseEvent::Action::PRESS)
				m_rpressed = true;
			else if (m_rpressed && mouseEvent.m_action == MouseEvent::Action::RELEASE) {
				m_rpressed = false;
				setIndex(getIndex() + 1);
			}
		}
	}
	else {
		m_lhighlighted = false;
		m_rhighlighted = false;
		m_lpressed = false;
		m_rpressed = false;
	}
}

void SideList::userAction(ActionState& actionState) 
{
	// User can only change selection by using the left/right directional key actions
	if (actionState.isAction(ActionState::Action::UI_LEFT) == ActionState::State::PRESS)
		setIndex(m_index - 1);
	else if (actionState.isAction(ActionState::Action::UI_RIGHT) == ActionState::State::PRESS)
		setIndex(m_index + 1);
}

void SideList::renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) 
{
	// Exit Early
	if (!getVisible() || !m_shader->ready())
		return;

	const glm::vec2 newPosition = position + m_position;

	// Render (background)
	m_shader->bind();
	m_shader->setUniform(0, newPosition);
	m_shader->setUniform(1, m_enabled);
	m_shader->setUniform(2, m_lEnabled);
	m_shader->setUniform(3, m_rEnabled);
	m_shader->setUniform(4, m_lhighlighted);
	m_shader->setUniform(5, m_rhighlighted);
	m_shader->setUniform(6, m_lpressed);
	m_shader->setUniform(7, m_rpressed);
	glBindVertexArray(m_vaoID);
	m_indirect.drawCall();
	Shader::Release();

	// Render children (text)
	UI_Element::renderElement(deltaTime, position, scale);
}

void SideList::setIndex(const int& index) 
{
	if (m_index != index) {
		m_index = std::clamp<int>(index, 0, int(m_strings.size()) - 1);
		m_label->setText(m_strings[m_index]);

		m_lEnabled = (index > 0);
		m_rEnabled = (index < (int)(m_strings.size() - 1ull));
		enactCallback((int)SideList::Interact::on_index_changed);
	}
}

int SideList::getIndex() const noexcept 
{
	return m_index;
}

void SideList::setStrings(const std::vector<std::string>& strings) 
{
	m_strings = strings;
	setIndex(getIndex());
}

std::vector<std::string> SideList::getStrings() const
{
	return m_strings;
}

void SideList::updateGeometry() 
{
	// Shorten the back panel by the width of the arrows
	const float arrowHeight = m_scale.y;
	m_backPanel->setScale(glm::vec2(getScale().x - (arrowHeight * 2.0f), getScale().y));

	// Adjust the size of the text font, ensuring it at least fits within bounds (automatic when setting scale)
	m_label->setScale(m_backPanel->getScale());

	constexpr auto num_data = 2 * 3;
	std::vector<glm::vec3> data(num_data);
	std::vector<int> objIndices(num_data);

	// Arrows
	data[0] = { -1,  0, 0 };
	data[1] = { 0, -1, 0 };
	data[2] = { 0, 1, 0 };
	for (int x = 0; x < 3; ++x) {
		data[x] = (data[x] * glm::vec3(arrowHeight)) - glm::vec3(getScale().x - arrowHeight, 0, 0);
		objIndices[x] = 0;
	}
	data[3] = { 1,  0, 0 };
	data[4] = { 0, 1, 0 };
	data[5] = { 0, -1, 0 };
	for (int x = 3; x < 6; ++x) {
		data[x] = (data[x] * glm::vec3(arrowHeight)) + glm::vec3(getScale().x - arrowHeight, 0, 0);
		objIndices[x] = 1;
	}

	glNamedBufferSubData(m_vboID[0], 0, num_data * sizeof(glm::vec3), &data[0]);
	glNamedBufferSubData(m_vboID[1], 0, num_data * sizeof(int), &objIndices[0]);
}