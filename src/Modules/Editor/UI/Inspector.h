#pragma once
#ifndef INSPECTOR_H
#define INSPECTOR_H

#include "Modules/UI/UI_M.h"
#include "Modules/World/ECS/ecsSystem.h"
#include <map>


// Forward declarations
class Engine;
class LevelEditor_Module;

/** A level editor UI element responsible for allowing the user to inspect selected entity components. */
class Inspector : public ImGUI_Element {
public:
	// Public (de)Constructors
	/** Destroy this inspector. */
	inline ~Inspector() = default;
	/** Construct a component inspector.
	@param	engine		the currently active engine.
	@param	editor		the level editor. */
	Inspector(Engine * engine, LevelEditor_Module * editor);


	// Public Interface Implementation
	virtual void tick(const float & deltaTime) override;


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	LevelEditor_Module * m_editor = nullptr;
	ECSSystemList m_inspectorSystems;
	std::map<int, size_t> m_selectionCountMap;
};

#endif // INSPECTOR_H