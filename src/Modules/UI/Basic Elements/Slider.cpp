#include "Modules/UI/Basic Elements/Slider.h"


Slider::Slider(Engine& engine, const float& value, const glm::vec2& range) noexcept :
	UI_Element(engine), m_value(value), m_lowerRange(range.x), m_upperRange(range.y)
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
	m_label = std::make_shared<Label>(engine, std::to_string(value));
	m_label->setAlignment(Label::Alignment::align_right);
	m_label->setTextScale(12.0f);
	m_label->setColor(glm::vec3(0.75f));
	addElement(m_label);

	// Callbacks
	addCallback((int)UI_Element::Interact::on_resize, [&]() { updateGeometry(); });

	// Configure THIS element
	setValue(value);
}

void Slider::renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) noexcept 
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

void Slider::mouseAction(const MouseEvent& mouseEvent) noexcept 
{
	UI_Element::mouseAction(mouseEvent);
	if (getVisible() && getEnabled() && mouseWithin(mouseEvent)) {
		if (m_pressed && mouseEvent.m_action == MouseEvent::Action::MOVE) {
			const float mx = float(mouseEvent.m_xPos) - m_position.x - m_backPanel->getPosition().x + m_backPanel->getScale().x;
			setValue(((mx / (m_backPanel->getScale().x * 2.0f)) * (m_upperRange - m_lowerRange)) + m_lowerRange);
		}
	}
}

void Slider::userAction(ActionState& actionState) noexcept 
{
	const float offsetAmount = std::min<float>((m_upperRange - m_lowerRange) / 100.0f, 1.0f);
	if (actionState.isAction(ActionState::Action::UI_LEFT) == ActionState::State::PRESS)
		setValue(getValue() - offsetAmount);
	else if (actionState.isAction(ActionState::Action::UI_RIGHT) == ActionState::State::PRESS)
		setValue(getValue() + offsetAmount);
}

void Slider::setValue(const float& amount) noexcept 
{
	m_value = std::clamp<float>(amount, m_lowerRange, m_upperRange);
	setText(std::to_string((int)std::round(m_value)));
	updatePaddle();
	enactCallback((int)Slider::Interact::on_value_change);
}

float Slider::getValue() const noexcept 
{
	return m_value;
}

void Slider::setRanges(const float& lowerRange, const float& upperRange) noexcept 
{
	m_lowerRange = lowerRange;
	m_upperRange = upperRange;

	// Set the value again, in case it falls outside of the new ranges
	setValue(m_value);
	updatePaddle();
}

void Slider::setText(const std::string& text) noexcept 
{
	m_label->setText(text);
}

std::string Slider::getText() const noexcept 
{
	return m_label->getText();
}

void Slider::updateGeometry()
{
	// Shorten the back panel by 50 units, and it is offset to the right by 50 units
	m_backPanel->setPosition({ 50, 0 });
	m_backPanel->setScale(glm::vec2(getScale().x - m_backPanel->getPosition().x, getScale().y));
	// Move the label to the left side of the back panel
	m_label->setPosition(-glm::vec2(getScale().x - 25, 0));
	m_label->setScale({ 50, 28 });
	// Update the paddle
	updatePaddle();
}

void Slider::updatePaddle()
{
	if (m_paddle) {
		// The paddle fills a 6th of the back panel, or 10 pixels, whichever is bigger
		m_paddle->setScale(glm::vec2(std::max<float>((getScale().x - 50) / 6.0f, 10.0), getScale().y - 2.0f) - 2.0f);
		m_paddle->setPosition({ (2.0f * ((m_value - m_lowerRange) / (m_upperRange - m_lowerRange)) - 1.0f) * (getScale().x - 54.0f - m_paddle->getScale().x), 0 });
	}
}