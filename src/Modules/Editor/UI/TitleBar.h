#pragma once
#ifndef TITLEBAR_H
#define TITLEBAR_H

#include "Modules/UI/Basic Elements/Panel.h"
#include "Modules/UI/Basic Elements/Button.h"
#include "Modules/UI/Basic Elements/List_Horizontal.h"


/***/
class TitleBar : public Panel {
public:
	// Public (de)Constructors
	/** Destroy the title bar. */
	inline ~TitleBar() = default;
	/** Creates a title bar. */
	inline TitleBar(Engine * engine)
		: Panel(engine) {
		setColor(glm::vec4(0.25, 0.35, 0.30, 1.0f));

		m_layout = std::make_shared<List_Horizontal>(engine);
		m_layout->setSpacing(5.0);
		addElement(m_layout);
		addMenu(engine, "File");
		addMenu(engine, "Edit");
		addMenu(engine, "View");
		addMenu(engine, "Help");

		// Callbacks
		addCallback(UI_Element::on_resize, [&]() {
			const auto scale = getScale();
			m_layout->setScale(scale);
		});
	}


private:
	// 
	inline void addMenu(Engine * engine, const char * buttonText) {
		auto button = std::make_shared<Button>(engine, buttonText);
		button->setScale({ 30, 12.5 });
		button->setTextScale(9.5f);
		//button->addCallback(Button::on_clicked, callback);
		//m_selectionCallbacks.push_back(callback);
		m_layout->addElement(button);
		m_layout->getFocusMap().addElement(button);
	};

	//
	std::shared_ptr<List_Horizontal> m_layout;
};

#endif // TITLEBAR_H