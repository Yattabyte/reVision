#include "Modules/UI/Basic Elements/List.h"


List::~List()
{
	// Delete geometry
	glDeleteBuffers(1, &m_vboID);
	glDeleteVertexArrays(1, &m_vaoID);
}

List::List(Engine& engine) :
	UI_Element(engine),
	m_shader(Shared_Shader(engine, "UI\\List"))
{
	// Generate vertex array
	glCreateVertexArrays(1, &m_vaoID);
	glEnableVertexArrayAttrib(m_vaoID, 0);
	glVertexArrayAttribBinding(m_vaoID, 0, 0);
	glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glCreateBuffers(1, &m_vboID);
	glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(glm::vec3));
	constexpr auto num_data = 8 * 6;
	glNamedBufferStorage(m_vboID, num_data * sizeof(glm::vec3), nullptr, GL_DYNAMIC_STORAGE_BIT);

	// Add Callbacks
	addCallback(static_cast<int>(UI_Element::Interact::on_resize), [&] {
		alignChildren();
		updateSelectionGeometry();
		});
	addCallback(static_cast<int>(UI_Element::Interact::on_childrenChange), [&] {
		alignChildren();
		});
}

void List::renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale)
{
	// Exit Early
	if (!getVisible() || m_children.empty() || !m_shader->ready())
		return;

	// Render
	m_shader->bind();
	glBindVertexArray(m_vaoID);
	if (m_hoverIndex > -1) {
		const glm::vec2 newPosition = position + m_position + m_children[m_hoverIndex]->getPosition();
		const glm::vec2 newScale = glm::min(m_children[m_hoverIndex]->getScale(), scale);
		m_shader->setUniform(0, newPosition);
		m_shader->setUniform(1, newScale);
		m_shader->setUniform(2, glm::vec4(1));
		glDrawArrays(GL_TRIANGLES, 0, 24);
	}
	if (m_selectionIndex > -1) {
		const glm::vec2 newPosition = position + m_position + m_children[m_selectionIndex]->getPosition();
		const glm::vec2 newScale = glm::min(m_children[m_selectionIndex]->getScale(), scale);
		m_shader->setUniform(0, newPosition);
		m_shader->setUniform(1, newScale);
		m_shader->setUniform(2, glm::vec4(0.8, 0.6, 0.1, 1));
		glDrawArrays(GL_TRIANGLES, 24, 24);
	}
	Shader::Release();

	// Render Children
	UI_Element::renderElement(deltaTime, position, scale);
}

void List::mouseAction(const MouseEvent& mouseEvent)
{
	UI_Element::mouseAction(mouseEvent);
	if (getVisible() && getEnabled() && mouseWithin(mouseEvent)) {
		// Move hover selection to whatever is beneath mouse
		MouseEvent subEvent = mouseEvent;
		subEvent.m_xPos = mouseEvent.m_xPos - m_position.x;
		subEvent.m_yPos = mouseEvent.m_yPos - m_position.y;
		int index(0);
		for (auto& child : m_children) {
			if (child->mouseWithin(subEvent)) {
				setHoverIndex(index); // Set hovered item to whatever is beneath mouse
				if (mouseEvent.m_action == MouseEvent::Action::RELEASE)
					setSelectionIndex(index); // Set selected item to whatever is beneath mouse
				break;
			}

				index++;
		}

		// Force current selection to stay highlighted
		if (!m_children.empty() && m_hoverIndex > -1)
			m_children[m_hoverIndex]->setHovered();
	}
}

void List::userAction(ActionState& actionState)
{
	// User can go up or down the list with an input device
	// User input wraps around, and if an item is selected, moving will deselect it
	if (!m_children.empty()) {
		// Allow selected child to receive input first
		if (m_selectionIndex >= 0 && m_selectionIndex < m_children.size())
			m_focusMap.applyActionState(actionState);

		// After, process remaining input for the list
		if (actionState.isAction(ActionState::Action::UI_UP) == ActionState::State::PRESS) {
			setHoverIndex(m_hoverIndex - 1 < 0 ? int(m_children.size() - 1ULL) : m_hoverIndex - 1);

			if (m_selectionIndex != -1)
				setSelectionIndex(-1);
		}
		else if (actionState.isAction(ActionState::Action::UI_DOWN) == ActionState::State::PRESS) {
			setHoverIndex(m_hoverIndex + 1 > int(m_children.size() - 1ULL) ? 0 : m_hoverIndex + 1);

			if (m_selectionIndex != -1)
				setSelectionIndex(-1);
		}
		else if (actionState.isAction(ActionState::Action::UI_ENTER) == ActionState::State::PRESS) {
			if (m_hoverIndex > -1 && m_hoverIndex < m_children.size())
				setSelectionIndex(m_hoverIndex);
		}
	}
}

void List::setHoverIndex(const int& newIndex)
{
	m_hoverIndex = newIndex;
	if (!m_children.empty()) {
		for (auto& child : m_children)
			child->clearFocus();
		if (m_hoverIndex > -1 && m_hoverIndex < m_children.size())
			m_children[m_hoverIndex]->setHovered();
	}
	updateSelectionGeometry();
}

int List::getHoverIndex() const noexcept
{
	return m_hoverIndex;
}

void List::setSelectionIndex(const int& newIndex)
{
	m_selectionIndex = newIndex;
	m_focusMap.focusIndex(m_selectionIndex);
	updateSelectionGeometry();
	enactCallback(static_cast<int>(List::Interact::on_selection));
}

int List::getSelectionIndex() const noexcept
{
	return m_selectionIndex;
}

FocusMap& List::getFocusMap() noexcept
{
	return m_focusMap;
}

void List::setMargin(const float& margin)
{
	m_margin = margin;
	alignChildren();
}

float List::getMargin() const noexcept
{
	return m_margin;
}

void List::setSpacing(const float& spacing)
{
	m_spacing = spacing;
	alignChildren();
	updateSelectionGeometry();
}

float List::getSpacing() const noexcept
{
	return m_spacing;
}

void List::setBorderSize(const float& size)
{
	m_borderSize = size;
	updateSelectionGeometry();
}

float List::getBorderSize() const noexcept
{
	return m_borderSize;
}

void List::alignChildren()
{
	float positionFromTop = m_scale.y - m_margin;
	const auto childrenCount = m_children.size();
	for (size_t x = 0; x < childrenCount; ++x) {
		const float size = m_children[x]->getScale().y;
		m_children[x]->setScale(glm::vec2(m_scale.x - m_margin, size));
		if (childrenCount == 1) {
			m_children[x]->setPosition(glm::vec2(0.0F));
			continue;
		}
		positionFromTop -= size;
		m_children[x]->setPosition(glm::vec2(0, positionFromTop));
		positionFromTop -= size + (m_spacing * 2.0F);
	}
}

void List::updateSelectionGeometry()
{
	if (m_children.empty()) return;
	constexpr auto num_data = 8 * 6;
	std::vector<glm::vec3> m_data(num_data);

	if (m_hoverIndex > -1) {
		const auto scale = glm::min(m_children[m_hoverIndex]->getScale() + m_spacing, m_scale - m_borderSize);
		// Bottom Bar
		m_data[0] = { -scale.x - m_borderSize, -scale.y, 0 };
		m_data[1] = { scale.x + m_borderSize, -scale.y, 0 };
		m_data[2] = { scale.x + m_borderSize, -scale.y + m_borderSize, 0 };
		m_data[3] = { scale.x + m_borderSize, -scale.y + m_borderSize, 0 };
		m_data[4] = { -scale.x - m_borderSize, -scale.y + m_borderSize, 0 };
		m_data[5] = { -scale.x - m_borderSize, -scale.y, 0 };

		// Left Bar
		m_data[6] = { -scale.x, -scale.y - m_borderSize, 0 };
		m_data[7] = { -scale.x + m_borderSize, -scale.y - m_borderSize, 0 };
		m_data[8] = { -scale.x + m_borderSize, scale.y + m_borderSize, 0 };
		m_data[9] = { -scale.x + m_borderSize, scale.y + m_borderSize, 0 };
		m_data[10] = { -scale.x, scale.y + m_borderSize, 0 };
		m_data[11] = { -scale.x, -scale.y - m_borderSize, 0 };

		// Top Bar
		m_data[12] = { -scale.x - m_borderSize, scale.y - m_borderSize, 0 };
		m_data[13] = { scale.x + m_borderSize, scale.y - m_borderSize, 0 };
		m_data[14] = { scale.x + m_borderSize, scale.y, 0 };
		m_data[15] = { scale.x + m_borderSize, scale.y, 0 };
		m_data[16] = { -scale.x - m_borderSize, scale.y, 0 };
		m_data[17] = { -scale.x - m_borderSize, scale.y - m_borderSize, 0 };

		// Right Bar
		m_data[18] = { scale.x - m_borderSize, -scale.y - m_borderSize, 0 };
		m_data[19] = { scale.x, -scale.y - m_borderSize, 0 };
		m_data[20] = { scale.x, scale.y + m_borderSize, 0 };
		m_data[21] = { scale.x, scale.y + m_borderSize, 0 };
		m_data[22] = { scale.x - m_borderSize, scale.y + m_borderSize, 0 };
		m_data[23] = { scale.x - m_borderSize, -scale.y - m_borderSize, 0 };
	}

	if (m_selectionIndex > -1) {
		const auto scale = glm::min(m_children[m_selectionIndex]->getScale() + m_spacing, m_scale - m_borderSize);
		// Bottom Bar
		m_data[24] = { -scale.x - m_borderSize, -scale.y, 0 };
		m_data[25] = { scale.x + m_borderSize, -scale.y, 0 };
		m_data[26] = { scale.x + m_borderSize, -scale.y + m_borderSize, 0 };
		m_data[27] = { scale.x + m_borderSize, -scale.y + m_borderSize, 0 };
		m_data[28] = { -scale.x - m_borderSize, -scale.y + m_borderSize, 0 };
		m_data[29] = { -scale.x - m_borderSize, -scale.y, 0 };

		// Left Bar
		m_data[30] = { -scale.x, -scale.y - m_borderSize, 0 };
		m_data[31] = { -scale.x + m_borderSize, -scale.y - m_borderSize, 0 };
		m_data[32] = { -scale.x + m_borderSize, scale.y + m_borderSize, 0 };
		m_data[33] = { -scale.x + m_borderSize, scale.y + m_borderSize, 0 };
		m_data[34] = { -scale.x, scale.y + m_borderSize, 0 };
		m_data[35] = { -scale.x, -scale.y - m_borderSize, 0 };

		// Top Bar
		m_data[36] = { -scale.x - m_borderSize, scale.y - m_borderSize, 0 };
		m_data[37] = { scale.x + m_borderSize, scale.y - m_borderSize, 0 };
		m_data[38] = { scale.x + m_borderSize, scale.y, 0 };
		m_data[39] = { scale.x + m_borderSize, scale.y, 0 };
		m_data[40] = { -scale.x - m_borderSize, scale.y, 0 };
		m_data[41] = { -scale.x - m_borderSize, scale.y - m_borderSize, 0 };

		// Right Bar
		m_data[42] = { scale.x - m_borderSize, -scale.y - m_borderSize, 0 };
		m_data[43] = { scale.x, -scale.y - m_borderSize, 0 };
		m_data[44] = { scale.x, scale.y + m_borderSize, 0 };
		m_data[45] = { scale.x, scale.y + m_borderSize, 0 };
		m_data[46] = { scale.x - m_borderSize, scale.y + m_borderSize, 0 };
		m_data[47] = { scale.x - m_borderSize, -scale.y - m_borderSize, 0 };
	}

	glNamedBufferSubData(m_vboID, 0, num_data * sizeof(glm::vec3), &m_data[0]);
}