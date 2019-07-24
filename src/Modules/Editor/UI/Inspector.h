#pragma once
#ifndef INSPECTOR_H
#define INSPECTOR_H

#include "Modules/UI/Basic Elements/Panel.h"
#include "Modules/UI/Basic Elements/Button.h"
#include "Modules/UI/Basic Elements/List.h"
#include "Modules/UI/Basic Elements/Layout_Vertical.h"
#include "Modules/UI/Basic Elements/Layout_Horizontal.h"


/***/
class Inspector : public UI_Element {
public:
	// Public (de)Constructors
	/** Destroy the component inspector. */
	inline ~Inspector() = default;
	/** Creates a component inspector. */
	inline Inspector(Engine * engine)
		: UI_Element(engine) {

		/*m_layout = std::make_shared<List>(engine);
		m_layout->setSpacing(5.0);
		addElement(m_layout);*/

		// Top title panel
		auto panel = std::make_shared<Panel>(engine);
		panel->setColor(glm::vec4(0.25, 0.35, 0.30, 1.0f));
		addElement(panel);
		auto label = std::make_shared<Label>(engine, "Inspector");
		label->setAlignment(Label::align_left);
		addElement(label);

		// Bottom list portion
		auto panel2 = std::make_shared<Panel>(engine);
		panel2->setColor(glm::vec4(0,0,0, 0.5f));
		addElement(panel2);

		// Callbacks
		addCallback(UI_Element::on_resize, [&, panel, label, panel2]() {
			const auto scale = getScale();

			panel->setScale({ scale.x, 12.5f });
			panel->setPosition({ 0, scale.y - 12.5 });
			label->setScale({ scale.x - 12.5f, 12.5f });
			label->setPosition({ 25.0f, scale.y - 12.5 });

			panel2->setScale({ scale.x, scale.y - 25.0f });
		});
	}
};

#endif // INSPECTOR_H