#include "Modules/UI/Basic Elements/Toggle.h"


Toggle::Toggle(Engine& engine, const bool& state) :
	UI_Element(engine)
{
	// Make a background panel for cosmetic purposes
	auto panel = std::make_shared<Panel>(engine);
	panel->setColor(glm::vec4(0.3f));
	m_backPanel = std::make_shared<Border>(engine, panel);
	addElement(m_backPanel);

	// Create the sliding paddle
	m_paddle = std::make_shared<Panel>(engine);
	m_paddle->setColor(glm::vec4(0.75f));
	panel->addElement(m_paddle);

	// Add a label indicating the toggle state
	m_label = std::make_shared<Label>(engine);
	m_label->setAlignment(Label::Alignment::align_right);
	m_label->setTextScale(12.0f);
	m_label->setColor(glm::vec3(0.75f));
	addElement(m_label);

	// Callbacks
	addCallback((int)UI_Element::Interact::on_clicked, [&] { setToggled(!m_toggledOn); });
	addCallback((int)UI_Element::Interact::on_resize, [&] { updateGeometry(); });

	// Configure THIS element
	setToggled(state);
}

void Toggle::renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) 
{
	// Update Colors
	glm::vec4 color(0.75);
	if (m_pressed)
		color *= 0.5f;
	if (m_hovered)
		color *= 1.5f;
	m_paddle->setColor(color);

	// Render Children
	UI_Element::renderElement(deltaTime, position, scale);
}

void Toggle::userAction(ActionState& actionState)
{
	if (actionState.isAction(ActionState::Action::UI_LEFT) == ActionState::State::PRESS)
		setToggled(false);
	else if (actionState.isAction(ActionState::Action::UI_RIGHT) == ActionState::State::PRESS)
		setToggled(true);
	else if (actionState.isAction(ActionState::Action::UI_ENTER) == ActionState::State::PRESS)
		setToggled(!m_toggledOn);
}

void Toggle::setText(const std::string& text) 
{
	m_label->setText(text);
}

std::string Toggle::getText() const 
{
	return m_label->getText();
}

void Toggle::setToggled(const bool& state) 
{
	m_toggledOn = state;
	setText(m_toggledOn ? "ON" : "OFF");
	updateGeometry();
	enactCallback((int)Toggle::Interact::on_toggle);
}

bool Toggle::isToggled() const noexcept 
{
	return m_toggledOn;
}

void Toggle::updateGeometry()
{
	// Shorten the back panel by 50 units, and it is offset to the right by 50 units
	m_backPanel->setPosition({ 50, 0 });
	m_backPanel->setScale(glm::vec2(getScale().x - m_backPanel->getPosition().x, getScale().y));
	// Move the label to the left side of the back panel
	m_label->setPosition(-glm::vec2(getScale().x - 25, 0));
	m_label->setScale({ 50, 28 });
	// The paddle fills HALF of the back panel, and is positioned on one of the 2 sides of it.
	m_paddle->setScale(glm::vec2((getScale().x - 50) / 2.0f, getScale().y - 2.0f) - 2.0f);
	m_paddle->setPosition(glm::vec2(m_paddle->getScale().x, 0) * (m_toggledOn ? 1.0f : -1.0f));
}