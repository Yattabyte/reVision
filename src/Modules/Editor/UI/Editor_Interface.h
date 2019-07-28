#pragma once
#ifndef EDITOR_INTERFACE_H
#define EDITOR_INTERFACE_H

#include "Modules/UI/UI_M.h"
#include "Assets/Auto_Model.h"
#include "Assets/Shader.h"


// Forward Declarations
class Engine;
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
	virtual void tick(const float & deltaTime) override;


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	LevelEditor_Module * m_editor = nullptr;
	std::vector<ImGUI_Element*> m_elements;
	Shared_Auto_Model m_shapeQuad;
	Shared_Shader m_shader;
	StaticBuffer m_indirectBuffer;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // EDITOR_INTERFACE_H