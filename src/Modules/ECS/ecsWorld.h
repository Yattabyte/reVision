#pragma once
#ifndef ECS_WORLD_H
#define ECS_WORLD_H

#include "Modules/ECS/ecsComponent.h"
#include "Modules/ECS/ecsEntity.h"
#include "Modules/ECS/ecsSystem.h"


/** A set of ECS entities and components forming a single level. */
class ecsWorld {
public:
	// Public (De)Constructors
	/** Destroy this ECS World. */
	~ecsWorld() noexcept;
	/** Construct an empty ECS World. */
	ecsWorld() noexcept;
	/** Construct an ECS world from a serial data buffer. 
	@param	data				serialized world data. */
	explicit ecsWorld(const std::vector<char>& data) noexcept;
	/** Move an ECS world. 
	@param	other				another ecsWorld to move to here. */
	ecsWorld(ecsWorld&& other) noexcept;


	/////////////////////////////
	/// PUBLIC MAKE FUNCTIONS ///
	/////////////////////////////
	/** Create an entity from a list of input components.
	@param	components			array of component pointers, whom will be hard copied.
	@param	numComponents		the number of components in the array.
	@param	name				optional entity name, more for use in the level editor.
	@param	UUID				optional entity UUID, if empty will auto-generate.
	@param	parentUUID			optional parent entity UUID, if not at the level root.
	@return						handle to the entity on success, empty on failure. */
	[[maybe_unused]] EntityHandle makeEntity(const ecsBaseComponent* const *const components, const size_t& numComponents, const std::string& name = "Entity", const EntityHandle& UUID = EntityHandle(), const EntityHandle& parentUUID = EntityHandle()) noexcept;
	/** Adds a component to an entity.
	@param	entityHandle		handle to the entity to add the component to.
	@param	component			the component being added.
	@param	UUID				optional reference to the component UUID, if empty will auto-generate. */
	void makeComponent(const EntityHandle& entityHandle, const ecsBaseComponent* const component, ComponentHandle& UUID = ComponentHandle()) noexcept;
	/** Adds a component to an entity.
	@param	entityHandle		handle to the entity to add the component to.
	@param	componentID			the runtime component class.
	@param	component			the component being added.
	@param	UUID				optional reference to the component UUID, if empty will auto-generate. */
	void makeComponent(const EntityHandle& entityHandle, const ComponentID& componentID, const ecsBaseComponent* const component = nullptr, ComponentHandle& UUID = ComponentHandle()) noexcept;


	///////////////////////////////
	/// PUBLIC REMOVE FUNCTIONS ///
	///////////////////////////////
	/** Search for and remove an entity matching the specific handle provided.
	@param	entityHandle		handle to the entity to be removed.
	@return						true on successful removal, false otherwise. */
	bool removeEntity(const EntityHandle& entityHandle) noexcept;
	/** Search for and remove a component matching the specific handle provided.
	@param	componentHandle		handle to the component to be removed.
	@return						true on successful removal, false otherwise. */
	bool removeComponent(const ComponentHandle& componentHandle) noexcept;
	/** Remove a specific component class from within a specific entity.
	@param	entityHandle		handle to the entity to remove the component from.
	@param	componentID			the runtime ID identifying the component class.
	@return						true on successful removal, false otherwise. */
	bool removeEntityComponent(const EntityHandle& entityHandle, const ComponentID& componentID) noexcept;


	////////////////////////////
	/// PUBLIC GET FUNCTIONS ///
	////////////////////////////
	/** Try to find an entity matching the UUID provided.
	@param	UUID				the target entity's UUID.
	@return						pointer to the found entity on success, nullptr on failure. */
	std::shared_ptr<ecsEntity> getEntity(const EntityHandle& UUID) const noexcept;
	/** Try to find a list of entities matching the UUID's provided.
	@param	UUIDs				list of target entity UUID's.
	@return						list of pointers to the found entities. Dimensions may not match input list (nullptr's omitted). */
	std::vector<std::shared_ptr<ecsEntity>> getEntities(const std::vector<EntityHandle>& uuids) const noexcept;
	/** Retrieve a list of entity handles, given a root node.
	@param	rootHandle			an optional root element to start fetching at (empty == map root).
	@return						a vector of all level entities. */
	std::vector<EntityHandle> getEntityHandles(const EntityHandle& rootHandle = EntityHandle()) const noexcept;
	/** Try to retrieve a component of a specific type from an entity matching the handle supplied.
	@param	<T>					the category of component being retrieved.
	@param	entityHandle		handle to the entity to retrieve from.
	@return						the specific component of the type requested on success, nullptr otherwise. */
	template <typename T>
	inline T* getComponent(const EntityHandle& entityHandle) const noexcept {
		if (auto* component = getComponent(entityHandle, T::Runtime_ID))
			return dynamic_cast<T*>(component);
		return nullptr;
	}
	/** Retrieve a component.
	@param	entityHandle		handle to the entity to retrieve from.
	@param	componentID			the runtime ID identifying the component class.
	@return						the specific component on success, nullptr otherwise. */
	ecsBaseComponent* getComponent(const EntityHandle& entityHandle, const ComponentID& componentID) const noexcept;
	/** Try to retrieve a component of a specific type matching the UUID provided.
	@param	<T>					the category of component being retrieved.
	@param	componentHandle		the target component's handle.
	@return						the specific component of the type requested on success, nullptr otherwise. */
	template <typename T>
	inline T* getComponent(const ComponentHandle& componentHandle) const noexcept {
		if (auto* component = getComponent(componentHandle))
			return dynamic_cast<T*>(component);
		return nullptr;
	}
	/** Try to find a component matching the UUID provided.
	@param	UUID				the target component's UUID.
	@return						pointer to the found component on success, nullptr on failure. */
	ecsBaseComponent* getComponent(const ComponentHandle& componentHandle) const noexcept;
	/** Retrieve the component from an entity matching the class specified.
	@param	entityComponents	the array of entity component IDS.
	@param	mem_array			the array of component data.
	@param	componentID			the class ID of the component.
	@return						the component pointer matching the ID specified. */
	static ecsBaseComponent* getComponent(const std::vector<std::tuple<ComponentID, int, ComponentHandle>>& entityComponents, const ComponentDataSpace& mem_array, const ComponentID& componentID) noexcept;


	////////////////////////
	/// PUBLIC FUNCTIONS ///
	////////////////////////
	/** Move the data from another ecsWorld into this.
	@param	other		another ecsWorld to move the data from.
	@return				reference to this. */
	ecsWorld& operator=(ecsWorld&& other) noexcept;
	/** Clear the data out of this ecsWorld. */
	void clear() noexcept;
	/** Generate a universally unique identifier for entities or components.
	@return						a new ID. */
	static ecsHandle generateUUID() noexcept;
	/** Parent an entity to another entity.
	@param	parentHandle		the handle to the desired parent entity.
	@param	childHandle			the handle to the desired child entity. */
	void parentEntity(const EntityHandle& parentHandle, const EntityHandle& childHandle) noexcept;
	/** Strip a child entity of its parent.
	@param	entityHandle		handle to the child entity, whom will be stripped of its parent. */
	void unparentEntity(const EntityHandle& entityHandle) noexcept;
	/** Serialize a specific set of entities to a char vector.
	@param	entityHandles		the set of entities identified by their handles to serialize.
	@return						char vector containing serialized entity data. */
	[[nodiscard]] std::vector<char> serializeEntities(const std::vector<EntityHandle>& entityHandles) const noexcept;
	/** Serialize a specific set of entities to a char vector.
	@param	entities			the set of specific entities to serialize.
	@return						char vector containing serialized entity data. */
	[[nodiscard]] std::vector<char> serializeEntities(const std::vector<std::shared_ptr<ecsEntity>>& entities) const noexcept;
	/** Serialize a specific entity to a char vector.
	@param	entityHandle		handle to the entity to serialize.
	@return						char vector containing serialized entity data. */
	[[nodiscard]] std::vector<char> serializeEntity(const EntityHandle& entityHandle) const noexcept;
	/** Serialize a specific entity to a char vector.
	@param	entity				the specific entity to serialize.
	@return						char vector containing serialized entity data. */
	[[nodiscard]] std::vector<char> serializeEntity(const ecsEntity& entity) const noexcept;
	/** De-serialize an entity from a char array.
	@param	data				previously serialized entity data.
	@param	dataSize			the size of the data in bytes (sizeof(char) * elements).
	@param	dataRead			reference to number of elements or bytes read in data so far.
	@param	desiredHandle		specific handle to use. If empty will be updated with serialized value instead.
	@param	parentHandle		optional handle to parent entity, designed to be called recursively if entity has children. */
	void deserializeEntity(const std::vector<char>& data, const size_t& dataSize, size_t& dataRead, EntityHandle& desiredHandle = EntityHandle(),  const EntityHandle& parentHandle = EntityHandle()) noexcept;
	/** Try to find a component ID based on the component ID.
	@param	name				the component class name to search for.
	@return						optional component ID on success, nullptr on failure. */
	static std::optional<ComponentID> nameToComponentID(const char* name) noexcept;
	/** Search for a component template with a matching class name.
	@param	name				the component class name to search for.
	@return						shared pointer to the a new component with a matching class name if successful, nullptr otherwise. */
	[[nodiscard]] std::shared_ptr<ecsBaseComponent> makeComponentType(const char* name) noexcept;
	/** Update the components of all systems provided.
	@param	systems				the systems to update.
	@param	deltaTime			the delta time. */
	void updateSystems(ecsSystemList& systems, const float& deltaTime) noexcept;
	/** Update the components of a single system.
	@param	system				the system to update.
	@param	deltaTime			the delta time. */
	void updateSystem(ecsBaseSystem* system, const float& deltaTime) noexcept;
	/** Update the components of a single system.
	@param	system				the system to update.
	@param	deltaTime			the delta time. */
	void updateSystem(const std::shared_ptr<ecsBaseSystem>& system, const float& deltaTime) noexcept;
	/** Update the components of a single system.
	@param	deltaTime			the delta time.
	@param	componentTypes		list of component types to retrieve.
	@param	func				lambda function serving as a system. */
	void updateSystem(const float& deltaTime, const std::vector<std::pair<ComponentID, ecsBaseSystem::RequirementsFlag>>& componentTypes, const std::function<void(const float&, const std::vector<std::vector<ecsBaseComponent*>>&)>& func) noexcept;


private:
	// Private Methods
	/** Disallow copying an ECS world. */
	inline ecsWorld(const ecsWorld&) noexcept = delete;
	/** Check if a given component ID has been previously registered and deemed valid.
	@param	componentID			the component ID to verify.
	@return						true if valid and registered, false otherwise. */
	static bool isComponentIDValid(const ComponentID& componentID) noexcept;
	/** Delete a component matching the category ID supplied, at the given index.
	@param	componentID			the component class/category ID.
	@param	index				the component index to delete. */
	void deleteComponent(const ComponentID& componentID, const ComponentID& index) noexcept;
	/** Retrieve the components relevant to an ECS system.
	@param	componentTypes		list of component types to retrieve. */
	[[nodiscard]] std::vector<std::vector<ecsBaseComponent*>> getRelevantComponents(const std::vector<std::pair<ComponentID, ecsBaseSystem::RequirementsFlag>>& componentTypes) noexcept;
	/** Find the least common component.
	@param	componentTypes		the component types.
	@return						the byte-size of the least common component. */
	size_t findLeastCommonComponent(const std::vector<std::pair<ComponentID, ecsBaseSystem::RequirementsFlag>>& componentTypes) noexcept;


	// Private Attributes
	ComponentMap m_components;
	EntityMap m_entities;
};

#endif // ECS_WORLD_H