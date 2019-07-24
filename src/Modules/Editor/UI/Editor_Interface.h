#pragma once
#ifndef EDITOR_INTERFACE_H
#define EDITOR_INTERFACE_H

#include "Modules/Editor/UI/TitleBar.h"
#include "Modules/Editor/UI/ToolBar.h"
#include "Modules/Editor/UI/Inspector.h"


/***/
class Editor_Interface : public UI_Element {
public:
	// Public (de)Constructors
	/** Destroy the editor. */
	inline ~Editor_Interface() = default;
	/** Creates an editor. */
	inline Editor_Interface(Engine * engine)
		: UI_Element(engine) {
		auto titlebar = std::make_shared<TitleBar>(engine);
		addElement(titlebar);
		auto toolbar = std::make_shared<ToolBar>(engine);
		addElement(toolbar);
		auto inspector = std::make_shared<Inspector>(engine);
		addElement(inspector);


		// Callbacks
		addCallback(UI_Element::on_resize, [&, titlebar, toolbar, inspector]() {
			const auto scale = getScale();
			titlebar->setScale({ scale.x, 12.5f });
			titlebar->setPosition(glm::vec2(0, scale.y-12.5f));
			toolbar->setScale({ scale.x, 15.0f });
			toolbar->setPosition(glm::vec2(0, scale.y - 25.0f - 15.0f));
			inspector->setScale({ 175.0f, scale.y - 55 });
			inspector->setPosition(glm::vec2(scale.x-175.0f, 0));
		});
	}
};

#endif // EDITOR_INTERFACE_H