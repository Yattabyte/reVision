#include "Modules/UI/Macro Elements/Options_Pane.h"


Options_Pane::Options_Pane(Engine& engine) noexcept :
	UI_Element(engine),
	m_title(std::make_shared<Label>(engine)),
	m_description(std::make_shared<Label>(engine)),
	m_layout(std::make_shared<List>(engine)),
	m_separatorTop(std::make_shared<Separator>(engine)),
	m_separatorBot(std::make_shared<Separator>(engine)),
	m_backPanel(std::make_shared<Panel>(engine))
{
	// Make a background panel for cosmetic purposes
	m_backPanel->setColor(glm::vec4(0.1, 0.1, 0.1, 0.5));
	addElement(m_backPanel);

	// Make a vertical layout to house list items
	m_layout->setSpacing(1.0f);
	m_layout->setMargin(50.0f);
	m_layout->addCallback((int)List::Interact::on_selection, [&]() {
		const auto index = m_layout->getSelectionIndex();
		if (index > -1 && size_t(index) < m_descriptions.size())
			std::dynamic_pointer_cast<Label>(m_description)->setText(m_descriptions[index]);
		else
			std::dynamic_pointer_cast<Label>(m_description)->setText("");
		});
	m_backPanel->addElement(m_layout);

	// Title
	m_title->setTextScale(20.0f);
	m_title->setAlignment(Label::Alignment::align_left);
	m_backPanel->addElement(m_title);

	// Top Separator
	m_backPanel->addElement(m_separatorTop);

	// Bottom Separator
	m_backPanel->addElement(m_separatorBot);

	// Bottom Description Label
	m_description->setAlignment(Label::Alignment::align_left);
	m_description->setTextScale(10.0f);
	m_description->setColor(glm::vec3(0.8, 0.6, 0.1));
	m_description->setText("");
	m_backPanel->addElement(m_description);

	// Callbacks
	addCallback((int)UI_Element::Interact::on_resize, [&]() {
		const auto scale = getScale();
		m_backPanel->setScale(scale);
		m_layout->setScale(scale - glm::vec2(0, 50));
		m_title->setScale(scale);
		m_title->setPosition({ 50, scale.y - 50 });
		m_separatorTop->setScale(scale);
		m_separatorTop->setPosition({ 0, scale.y - 100 });
		m_separatorBot->setScale(scale);
		m_separatorBot->setPosition({ 0, -scale.y + 100 });
		m_description->setScale(scale);
		m_description->setPosition({ 50, -scale.y + 50 });
		});
}

void Options_Pane::userAction(ActionState& actionState) noexcept 
{
	// Options menu doesn't implement any custom controls, focus is on the list
	m_layout->userAction(actionState);
	if (actionState.isAction(ActionState::Action::UI_ESCAPE) == ActionState::State::PRESS)
		m_engine.getModule_UI().getFocusMap()->back();
}

void Options_Pane::addOption(Engine& engine, std::shared_ptr<UI_Element> element, const float& ratio, const std::string& text, const std::string& description, const int& eventType, const std::function<void()>& callback) noexcept 
{
	auto horizontalLayout = std::make_shared<Layout_Horizontal>(engine);
	auto label = std::make_shared<Label>(engine, text);
	label->setColor(glm::vec3(0.75f));
	element->addCallback(eventType, callback);
	element->setMaxHeight(14.0f);
	horizontalLayout->addElement(label, (1.0f - ratio) + 1.0f);
	horizontalLayout->addElement(element, ratio);
	horizontalLayout->setScale({ 0, 30 });
	m_layout->addElement(horizontalLayout);
	m_layout->getFocusMap().addElement(element);
	m_elements.push_back(element);
	m_descriptions.push_back(description);
}