#pragma once
#ifndef TOOLBAR_H
#define TOOLBAR_H

#include "Modules/UI/Basic Elements/Panel.h"
#include "Modules/UI/Basic Elements/Button.h"
#include "Modules/UI/Basic Elements/List_Horizontal.h"


/***/
class ToolBar : public Panel {
public:
	// Public (de)Constructors
	/** Destroy the tool bar. */
	inline ~ToolBar() = default;
	/** Creates a tool bar. */
	inline ToolBar(Engine * engine)
		: Panel(engine) {
		setColor(glm::vec4(0,0,0,0.5));

		m_layout = std::make_shared<List_Horizontal>(engine);
		m_layout->setSpacing(5.0);
		addElement(m_layout);
		addButton(engine, "New");
		addButton(engine, "Open");
		addButton(engine, "Save");
		addButton(engine, "Save As");
		addButton(engine, "Separator");
		addButton(engine, "Cut");
		addButton(engine, "Copy");
		addButton(engine, "Paste");
		addButton(engine, "Delete");
		addButton(engine, "Separator");
		addButton(engine, "Undo");
		addButton(engine, "Redo");

		// Callbacks
		addCallback(UI_Element::on_resize, [&]() {
			const auto scale = getScale();
			m_layout->setScale(scale);
		});
	}


private:
	// 
	inline void addButton(Engine * engine, const char * buttonText) {
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

#endif // TOOLBAR_H