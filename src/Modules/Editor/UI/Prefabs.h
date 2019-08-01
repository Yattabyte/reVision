#pragma once
#ifndef PREFABS_H
#define PREFABS_H

#include "Modules/UI/UI_M.h"


// Forward declarations
class Engine;
class LevelEditor_Module;

/***/
class Prefabs : public ImGUI_Element {
public:
	// Public (de)Constructors
	/***/
	inline ~Prefabs() = default;
	/***/
	Prefabs(Engine * engine, LevelEditor_Module * editor);


	// Public Interface Implementation
	virtual void tick(const float & deltaTime) override;


private:
	// Private Methods
	/***/
	void populatePrefabs();
	/***/
	void spawnPrefab(const int & index);


	// Private Attributes
	Engine * m_engine = nullptr;
	LevelEditor_Module * m_editor = nullptr;
};

#endif // PREFABS_H