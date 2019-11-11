#pragma once
#ifndef PREFABS_H
#define PREFABS_H

#include "Modules/Editor/UI/Editor_Interface.h"
#include "Modules/ECS/ecsWorld.h"
#include "Assets/Texture.h"


// Forward declarations
class Engine;
class LevelEditor_Module;
struct Viewport;
class Camera;

/** A level editor UI element allowing the user to spawn previously made prefab object sets. */
class Prefabs final : public ImGUI_Element {
public:
	// Public (de)Constructors
	/** Destroy this prefab UI element. */
	~Prefabs();
	/** Construct a prefab UI element.
	@param	engine			the currently active engine.
	@param	editor			the level editor. */
	Prefabs(Engine* engine, LevelEditor_Module* editor);


	// Public Interface Implementation
	virtual void tick(const float& deltaTime) override final;


	// Public Methods
	/** Construct a prefab from serialized entity data.
	@param	entityData		serialized entity data. */
	void addPrefab(const std::vector<char>& entityData);


private:
	// Private Methods
	/** Populate a list of prefabs given an optional subdirectory.
	@param	directory		if non-blank, a subdirectory within the prefabs folder. */
	void populatePrefabs(const std::string& directory = "");
	/** Open the selected prefab entry, spawning if its an object, if a folder populates with the folder contents.*/
	void openPrefabEntry();
	/** Update the thumbnails by a specific delta time.
	@param	deltaTime		the amount of time since last frame. */
	void tickThumbnails(const float& deltaTime);
	/** Update the prefabs window.
	@param	deltaTime		the amount of time since last frame. */
	void tickWindow(const float& deltaTime);
	/** Update any prefab pop-up's.
	@param	deltaTime		the amount of time since last frame. */
	void tickPopupDialogues(const float& deltaTime);


	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
	Shared_Texture m_texBack, m_texFolder, m_texMissingThumb, m_texIconRefresh;
	std::string m_prefabSubDirectory = "";
	glm::ivec2 m_renderSize;
	int m_thumbSize = 256;
	int m_selectedIndex = -1, m_hoverIndex = -1;
	struct Entry {
		std::string name = "", path = "";
		enum type {
			none,
			file,
			folder,
			back
		} type = none;
		std::vector<EntityHandle> entityHandles;
		glm::vec3 spawnPoint;
		GLuint texID = 0;
	};
	std::vector<Entry> m_prefabs;
	std::vector<std::shared_ptr<Camera>> m_prefabCameras;
	ecsWorld m_previewWorld;
	std::shared_ptr<Viewport> m_viewport;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	EntityHandle m_sunHandle;


	// Private Methods
	/** Add a specific prefab to the list.
	@param	prefab			the prefab to add. */
	void addPrefab(Prefabs::Entry& prefab);
};

#endif // PREFABS_H