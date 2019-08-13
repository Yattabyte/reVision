#pragma once
#ifndef PREFABS_H
#define PREFABS_H

#include "Modules/UI/UI_M.h"
#include "Modules/World/ECS/ecsEntity.h"
#include "Assets/Texture.h"


// Forward declarations
class Engine;
class LevelEditor_Module;
struct Prefab {
	std::string name, path;
	enum type {
		file,
		folder,
		back
	} type = file;
	std::vector<char> serialData;
};

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


	// Public Methods
	/***/
	void makePrefab(const std::vector<ecsEntity*> & entities);


private:
	// Private Methods
	/***/
	void populatePrefabs(const std::string & directory = "");
	/***/
	void selectPrefab(const Prefab& prefab);


	// Private Attributes
	Engine * m_engine = nullptr;
	LevelEditor_Module * m_editor = nullptr;	
	Shared_Texture m_texBack, m_texFolder, m_texMissingThumb;
	int m_selectedPrefab = -1;
	std::string m_prefabSubDirectory = "";
	std::vector<Prefab> m_prefabs;
};

#endif // PREFABS_H