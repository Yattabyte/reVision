#pragma once
#ifndef PREFABS_H
#define PREFABS_H

#include "Modules/UI/UI_M.h"
#include "Modules/World/ECS/ecsEntity.h"
#include "Assets/Texture.h"


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
	Prefabs(Engine* engine, LevelEditor_Module* editor);


	// Public Interface Implementation
	virtual void tick(const float& deltaTime) override;


	// Public Methods
	/***/
	void makePrefab(const std::vector<ecsEntity*>& entities);


private:
	// Private Methods
	/***/
	void populatePrefabs(const std::string& directory = "");
	/***/
	void openPrefab();


	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
	Shared_Texture m_texBack, m_texFolder, m_texMissingThumb, m_texIconRefresh;
	std::string m_prefabSubDirectory = "";
	int m_selectedIndex = -1;
	struct Prefab {
		std::string name = "", path = "";
		enum type {
			none,
			file,
			folder,
			back
		} type = none;
		std::vector<char> serialData;
	};
	std::vector<Prefab> m_prefabs;
};

#endif // PREFABS_H