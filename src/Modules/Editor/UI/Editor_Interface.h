#pragma once
#ifndef EDITOR_INTERFACE_H
#define EDITOR_INTERFACE_H

#include "Modules/UI/UI_M.h"


// Forward Declarations
class LevelEditor_Module;

/***/
class Editor_Interface : public ImGUI_Element {
public:
	// Public (de)Constructors
	/** Destroy the editor. */
	inline ~Editor_Interface() = default;
	/** Creates an editor. */
	Editor_Interface(Engine * engine, LevelEditor_Module * editor);


	// Public Interface Implementation
	virtual void render(const float & deltaTime) override;;


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	std::vector<ImGUI_Element*> m_elements;
};

#endif // EDITOR_INTERFACE_H