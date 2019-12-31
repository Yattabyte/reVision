#pragma once
#ifndef ECS_COMPONENT_H
#define ECS_COMPONENT_H

#include "Modules/ECS/ecsHandle.h"
#include "Utilities/MappedChar.h"
#include <functional>
#include <map>
#include <string>
#include <tuple>
#include <vector>


// Definitions to make life easier
struct ecsBaseComponent;
class ecsWorld;
using ComponentID = int;
using ComponentDataSpace = std::vector<uint8_t>;
using ComponentMap = std::map<ComponentID, ComponentDataSpace>;
using ComponentCreateFunction = std::function<ComponentID(ComponentDataSpace & memory, const ComponentHandle & componentHandle, const EntityHandle & entityHandle, const ecsBaseComponent * comp)>;
using ComponentNewFunction = std::function<std::shared_ptr<ecsBaseComponent>()>;
using ComponentFreeFunction = std::function<void(ecsBaseComponent * comp)>;

/** A base class representing components in an ECS architecture. */
struct ecsBaseComponent {
	// Public (De)Constructors
	/** Destroy this base ecsComponent. */
	inline virtual ~ecsBaseComponent() = default;
	/** Construct a base ecsComponent.
	@param	ID			the runtime ID for this component.
	@param	size		the byte-size of this component.
	@param	name		the char array name of this component. */
	ecsBaseComponent(const ComponentID& ID, const size_t& size, const char* name) noexcept;
	/** Move a base ecsComponent. */
	inline ecsBaseComponent(ecsBaseComponent&&) noexcept = default;
	/** Copy a base ecsComponent. */
	inline ecsBaseComponent(const ecsBaseComponent&) noexcept = default;
	/** Disallow base ecsComponent move assignment. */
	inline ecsBaseComponent& operator =(ecsBaseComponent&&) noexcept = default;
	/** Disallow base ecsComponent copy assignment. */
	inline ecsBaseComponent& operator =(const ecsBaseComponent&) noexcept = default;


	// Public Methods
	/** Save this component to a char buffer. Fulfilled by sub-class using CRTP.
	@return				serialized version of self. */
	virtual std::vector<char> to_buffer() = 0;
	/** Recover and generate a component from a char buffer.
	@param	data		serialized version of component.
	@param	dataRead	reference updated with the number of bytes read.
	@return				if successful a shared pointer to a new component, nullptr otherwise. */
	static std::shared_ptr<ecsBaseComponent> from_buffer(const std::vector<char>& data, size_t& dataRead);


	// Public Attributes
	/** Runtime generated ID per class. */
	ComponentID m_runtimeID;
	/** Total component byte-size. */
	size_t m_size;
	/** Specific class name. */
	const char* m_name;
	/** This component's UUID. */
	ComponentHandle m_handle;
	/** This component's parent-entity's UUID. */
	EntityHandle m_entity;


protected:
	// Protected Methods
	/** Register a specific sub-class component into the component registry for creating and freeing them at runtime.
	@param	createFn	function for creating a specific component type within an input memory block.
	@param	freeFn		function for freeing a specific component type from its memory block.
	@param	newFn		function for creating a new component anywhere. 
	@param	size		the total size of a single component.
	@param	string		type-name of the component, for name-lookups between since component ID's can change.
	@return				runtime component ID. */
	static ComponentID registerType(const ComponentCreateFunction& createFn, const ComponentFreeFunction& freeFn, const ComponentNewFunction& newFn, const size_t& size, const char* string);
	/** Recover and load component data into this component from a char buffer.
	@param	data		serialized component data. */
	virtual void recover_data(const std::vector<char>& data) = 0;


	// Protected Attributes
	/** Runtime container mapping indices to creation/destruction functions for components. */
	inline static std::vector<std::tuple<ComponentCreateFunction, ComponentFreeFunction, ComponentNewFunction, size_t>> m_componentRegistry = {};
	/** A map between component class name's and it's runtime variables like ID and size. */
	inline static MappedChar<ComponentID> m_nameRegistry = MappedChar<ComponentID>();
	/** Allow the ecsWorld to interact with these members. */
	friend class ecsWorld;
};


/** A specialized, specific type of component.
@param	C	the type of this component */
template <typename C, const char* chars>
struct ecsComponent : public ecsBaseComponent {
	// (De)Constructors
	/** Construct this specific component. */
	inline ecsComponent() noexcept : ecsBaseComponent(ecsComponent::Runtime_ID, sizeof(C), chars) {}


	// Public Methods
	/** Default serialization method, doing nothing. */
	inline static std::vector<char> serialize() noexcept {
		return {};
	}
	/** Default de-serialization method, doing nothing. */
	inline static void deserialize(const std::vector<char>&) noexcept {}
	/** Save this component to a char buffer.
	@return				serialized version of self. */
	inline std::vector<char> to_buffer() final {
		// Get portable name data
		const auto charCount = (int)std::string(chars).size();
		std::vector<char> nameData(sizeof(unsigned int) + (charCount * sizeof(char)));
		std::memcpy(&nameData[0], &charCount, sizeof(int));
		std::memcpy(&nameData[sizeof(int)], chars, charCount);
		// Get serial data
		auto classData = static_cast<C&>(*this).serialize();
		const auto& classDataSize = classData.size();
		std::vector<char> classDataSizeData(sizeof(size_t));
		std::memcpy(&classDataSizeData[0], &classDataSize, sizeof(size_t));
		classData.insert(classData.begin(), classDataSizeData.cbegin(), classDataSizeData.cend());

		std::vector<char> output;
		output.reserve(nameData.size() + classData.size());
		output.insert(output.end(), nameData.cbegin(), nameData.cend()); // name first (char count + name)
		output.insert(output.end(), classData.cbegin(), classData.cend()); // then data (data count + data)
		return output;
	}


	// Static Type-Specific Attributes
	/** Runtime class name of this component, also stored in base-component. */
	constexpr static const char* Name = chars;
	/** Runtime generated ID per class, also stored in base-component. */
	static const ComponentID Runtime_ID;


protected:
	// Protected Interface Implementation
	/** Recover previously serialized data.
	@param	data		serialized version of component. */
	inline void recover_data(const std::vector<char>& data) final {
		// Previously recovered type name, created this class
		// Next recover data
		static_cast<C&>(*this).deserialize(data);

		// Enforce runtime variables
		ecsBaseComponent::m_runtimeID = ecsComponent::Runtime_ID;
		ecsBaseComponent::m_size = sizeof(C);
		ecsBaseComponent::m_name = chars;

		// Always clear the handles
		ecsBaseComponent::m_handle = ComponentHandle();
		ecsBaseComponent::m_entity = EntityHandle();
	}
};


/** Constructs a new component of type <C> into the memory space provided.
@param	memory			raw data vector representing all components of type <C>.
@param	componentHandle	handle to the component.
@param	entityHandle	handle to the component's parent entity, the one who'll own this component.
@param	component		temporary pre-constructed component to copy data from, or nullptr.
@return					the index into the memory array where this component was created at. */
template <typename C>
inline constexpr static int createFn(ComponentDataSpace& memory, const ComponentHandle& componentHandle, const EntityHandle& entityHandle, const ecsBaseComponent* component = nullptr) noexcept {
	const size_t index = memory.size();
	memory.resize(index + sizeof(C));
	C* clone = component != nullptr 
		? new(&memory[index])C(*(C*)component) 
		: new(&memory[index])C();
	clone->m_handle = componentHandle;
	clone->m_entity = entityHandle;
	return (int)index;
}

/** Construct a new component of type <C> on the heap.
@return					shared pointer to the new component. */
template <typename C>
inline constexpr static auto newFn() noexcept {
	return std::make_shared<C>();
}

/** Destructs the supplied component, invalidating the memory range it occupied.
@param	component		the component to destruct. */
template <typename C>
inline constexpr static void freeFn(ecsBaseComponent* component) noexcept {
	if (auto * c = dynamic_cast<C*>(component))
		c->~C();
}

/** Generate a static ID at run time for each type of component class used. */
template <typename C, const char* chars>
const ComponentID ecsComponent<C, chars>::Runtime_ID(registerType(createFn<C>, freeFn<C>, newFn<C>, sizeof(C), chars));

#endif // ECS_COMPONENT_H