#include "Modules/UI/Macro Elements/Options_Levels.h"
#include "Modules/UI/Basic Elements/SideList.h"
#include "Modules/UI/Basic Elements/Button.h"
#include "Engine.h"
#include <filesystem>


Options_Levels::Options_Levels(Engine& engine) :
	Options_Pane(engine)
{
	// Title
	m_title->setText("Choose a level");

	const auto rootPath = std::filesystem::path(Engine::Get_Current_Dir() + "\\Maps\\");
	std::vector<std::string> levelNames;
	for (auto& entry : std::filesystem::recursive_directory_iterator(rootPath)) {
		const auto& path = entry.path();
		if (path.has_extension() && path.extension() == ".bmap") {
			levelNames.push_back(std::filesystem::relative(entry.path(), rootPath).string());
		}
	}
	m_level = levelNames[0];

	// Levels Option
	auto element_list = std::make_shared<SideList>(engine);
	element_list->setStrings(levelNames);
	element_list->setIndex(0);
	addOption(engine, element_list, 1.0F, "Level Name:", "Choose a level to play.", static_cast<int>(SideList::Interact::on_index_changed), [&, levelNames, element_list]() {
		m_level = levelNames[element_list->getIndex()];
		});

	// Play Button
	auto element_button = std::make_shared<Button>(engine, "Launch");
	addOption(engine, element_button, 0.5F, "Play Level", "Play the selected level.", static_cast<int>(Button::Interact::on_clicked), [&, element_button]() {
		enactCallback(static_cast<int>(Options_Levels::Interact::on_levelSelect));
	});
}