#include "Modules/UI/Basic Elements/Button.h"


Button::Button(Engine& engine, const std::string& text) :
	UI_Element(engine),
	m_label(std::make_shared<Label>(engine, text))
{
	// All buttons have labels
	m_label->setAlignment(Label::Alignment::align_center);
	m_label->setTextScale(12.5F);
	addElement(m_label);

	// Callbacks
	addCallback(static_cast<int>(UI_Element::Interact::on_resize), [&] { m_label->setScale(getScale()); });
}

void Button::userAction(ActionState& actionState)
{
	// Only thing a user can do is press the button
	if (actionState.isAction(ActionState::Action::UI_ENTER) == ActionState::State::PRESS)
		pressButton();
}

void Button::renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale)
{
	// Update Colors
	glm::vec4 color(0.75);
	if (m_pressed)
		color *= 0.5F;
	if (m_hovered)
		color *= 1.5F;
	m_label->setColor(color);

	// Render Children
	UI_Element::renderElement(deltaTime, position, scale);
}

void Button::pressButton()
{
	enactCallback(static_cast<int>(UI_Element::Interact::on_clicked));
}

void Button::setText(const std::string& text)
{
	m_label->setText(text);
}

std::string Button::getText() const
{
	return m_label->getText();
}

void Button::setTextScale(const float& textScale) noexcept
{
	m_label->setTextScale(textScale);
}

float Button::getTextScale() const noexcept {
	return m_label->getTextScale();
}

void Button::setBevelRadius(const float& radius) noexcept
{
	m_bevelRadius = radius;
}

float Button::getBevelRadius() const noexcept
{
	return m_bevelRadius;
}