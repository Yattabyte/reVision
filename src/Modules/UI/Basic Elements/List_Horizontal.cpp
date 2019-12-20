#include "Modules/UI/Basic Elements/List_Horizontal.h"


List_Horizontal::~List_Horizontal() noexcept 
{
	// Delete geometry
	glDeleteBuffers(1, &m_vboID);
	glDeleteVertexArrays(1, &m_vaoID);
}

List_Horizontal::List_Horizontal(Engine& engine) :
	UI_Element(engine),
	m_shader(Shared_Shader(engine, "UI\\List_Horizontal"))
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
	addCallback((int)UI_Element::Interact::on_resize, [&]() {
		alignChildren();
		updateSelectionGeometry();
		});
	addCallback((int)UI_Element::Interact::on_childrenChange, [&]() {
		alignChildren();
		});
}

void List_Horizontal::renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) 
{
	// Exit Early
	if (!getVisible() || !m_children.size() || !m_shader->ready())
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

void List_Horizontal::mouseAction(const MouseEvent& mouseEvent) 
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
			else
				index++;
		}

		// Force current selection to stay highlighted
		if (m_children.size() && m_hoverIndex > -1)
			m_children[m_hoverIndex]->setHovered();
	}
}

void List_Horizontal::userAction(ActionState& actionState) 
{
	// User can go up or down the list_Horizontal with an input device
	// User input wraps around, and if an item is selected, moving will deselect it
	if (m_children.size()) {
		// Allow selected child to receive input first
		if (m_selectionIndex >= 0 && m_selectionIndex < m_children.size())
			m_focusMap.applyActionState(actionState);

		// After, process remaining input for the list_Horizontal
		if (actionState.isAction(ActionState::Action::UI_UP) == ActionState::State::PRESS) {
			setHoverIndex(int(size_t(m_hoverIndex) - 1) < 0 ? (int)(m_children.size() - 1ull) : int(size_t(m_hoverIndex) - 1));

			if (m_selectionIndex != -1)
				setSelectionIndex(-1);
		}
		else if (actionState.isAction(ActionState::Action::UI_DOWN) == ActionState::State::PRESS) {
			setHoverIndex(int(size_t(m_hoverIndex) + 1) > m_children.size() - 1ull ? 0 : int(size_t(m_hoverIndex) + 1));

			if (m_selectionIndex != -1)
				setSelectionIndex(-1);
		}
		else if (actionState.isAction(ActionState::Action::UI_ENTER) == ActionState::State::PRESS) {
			if (m_hoverIndex > -1 && m_hoverIndex < m_children.size())
				setSelectionIndex(m_hoverIndex);
		}
	}
}

void List_Horizontal::setHoverIndex(const int& newIndex) 
{
	m_hoverIndex = newIndex;
	const auto childrenCount = m_children.size();
	if (childrenCount) {
		for (auto& child : m_children)
			child->clearFocus();
		if (m_hoverIndex > -1 && m_hoverIndex < childrenCount)
			m_children[m_hoverIndex]->setHovered();
	}
	updateSelectionGeometry();
}

int List_Horizontal::getHoverIndex() const noexcept 
{
	return m_hoverIndex;
}

void List_Horizontal::setSelectionIndex(const int& newIndex) 
{
	m_selectionIndex = newIndex;
	m_focusMap.focusIndex(m_selectionIndex);
	updateSelectionGeometry();
	enactCallback((int)List_Horizontal::Interact::on_selection);
}

int List_Horizontal::getSelectionIndex() const noexcept 
{
	return m_selectionIndex;
}

FocusMap& List_Horizontal::getFocusMap() noexcept 
{
	return m_focusMap;
}

void List_Horizontal::setMargin(const float& margin) noexcept 
{
	m_margin = margin;
	alignChildren();
}

float List_Horizontal::getMargin() const noexcept 
{
	return m_margin;
}

void List_Horizontal::setSpacing(const float& spacing) 
{
	m_spacing = spacing;
	alignChildren();
	updateSelectionGeometry();
}

float List_Horizontal::getSpacing() const noexcept 
{
	return m_spacing;
}

void List_Horizontal::setBorderSize(const float& size) 
{
	m_borderSize = size;
	updateSelectionGeometry();
}

float List_Horizontal::getBorderSize() const noexcept 
{
	return m_borderSize;
}

void List_Horizontal::alignChildren() 
{
	float positionFromLeft = m_margin;
	for (size_t x = 0; x < m_children.size(); ++x) {
		const float size = m_children[x]->getScale().x;
		m_children[x]->setScale(glm::vec2(size, m_scale.y - m_margin));
		if (m_children.size() == 1) {
			m_children[x]->setPosition(glm::vec2(0.0f));
			continue;
		}
		positionFromLeft += size;
		m_children[x]->setPosition(glm::vec2(positionFromLeft, 0));
		positionFromLeft += size + (m_spacing * 2.0f);
	}
}

void List_Horizontal::updateSelectionGeometry() 
{
	if (m_children.size() < 1) return;
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