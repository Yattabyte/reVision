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
	~Prefabs();
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
	glm::ivec2 m_renderSize = glm::ivec2(1);
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // PREFABS_H