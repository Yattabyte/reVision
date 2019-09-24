#pragma once
#ifndef PREFABS_H
#define PREFABS_H

#include "Modules/UI/UI_M.h"
#include "Modules/ECS/ecsEntity.h"
#include "Assets/Texture.h"


// Forward declarations
class Engine;
class LevelEditor_Module;

/** A level editor UI element allowing the user to spawn previously made prefab object sets. */
class Prefabs final : public ImGUI_Element {
public:
	// Public (de)Constructors
	/** Destroy this prefab UI element. */
	inline ~Prefabs() = default;
	/** Construct a prefab UI element.
	@param	engine			the currently active engine.
	@param	editor			the level editor. */
	Prefabs(Engine* engine, LevelEditor_Module* editor);


	// Public Interface Implementation
	virtual void tick(const float& deltaTime) override final;


	// Public Methods
	/** Construct a prefab from a specified entity set.
	@param	entityHandles	the entities constituting a new prefab. */
	void makePrefab(const std::vector<ecsHandle>& entityHandles);


private:
	// Private Methods
	/** Populate a list of prefabs given an optional subdirectory.
	@param	directory		if non-blank, a subdirectory within the prefabs folder. */
	void populatePrefabs(const std::string& directory = "");
	/** Open the selected prefab entry, spawning if its an object, if a folder populates with the folder contents.*/
	void openPrefabEntry();


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
			def,
			folder,
			back
		} type = none;
		std::vector<char> serialData;
	};
	std::vector<Prefab> m_prefabs;
};

#endif // PREFABS_H