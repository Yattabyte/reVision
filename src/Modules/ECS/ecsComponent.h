#pragma once
#ifndef ECS_COMPONENT_H
#define ECS_COMPONENT_H

#include "Modules/ECS/ecsHandle.h"
#include "Utilities/MappedChar.h"
#include <functional>
#include <map>
#include <string>
#include <sstream>
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
	inline virtual ~ecsBaseComponent() = default;
	inline ecsBaseComponent(const ComponentID& ID, const size_t& size, const char* name)
		: m_runtimeID(ID), m_size(size), m_name(name) { }


	// Public Methods
	/** Save this component to a char buffer. Fulfilled by sub-class using CRTP.
	@return				serialized version of self. */
	virtual std::vector<char> to_buffer() = 0;
	/** Recover and generate a component from a char buffer.
	@param	data		serialized version of component.
	@param	dataRead	reference updated with the number of bytes read.
	@return				if successful a shared pointer to a new component, nullptr otherwise. */
	static std::shared_ptr<ecsBaseComponent> from_buffer(const char* data, size_t& dataRead);


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
	@param	size		the total size of a single component.
	@param	string		type-name of the component, for name-lookups between since component ID's can change.
	@param	templateC	template component used for copying from.
	@return				runtime component ID. */
	static ComponentID registerType(const ComponentCreateFunction& createFn, const ComponentFreeFunction& freeFn, const ComponentNewFunction& newFn, const size_t& size, const char* string);
	/** Recover and load component data into this component from a char buffer.
	@param	data		serialized component data. */
	virtual void recover_data(const char* data) = 0;


	// Protected Attributes
	/** Runtime container mapping indices to creation/destruction functions for components. */
	static std::vector<std::tuple<ComponentCreateFunction, ComponentFreeFunction, ComponentNewFunction, size_t>> _componentRegistry;
	/** A map between component class name's and it's runtime variables like ID and size. */
	static MappedChar<ComponentID> _nameRegistry;
	/** Allow the ecsWorld to interact with these members. */
	friend class ecsWorld;
};


/** A specialized, specific type of component.
@param	C	the type of this component */
template <typename C, const char* chars>
struct ecsComponent : public ecsBaseComponent {
	// (De)Constructors
	inline virtual ~ecsComponent() = default;
	inline ecsComponent() : ecsBaseComponent(ecsComponent::Runtime_ID, sizeof(C), chars) {}


	// Public Methods
	/** Default serialization method.
	@return				serialized component data. */
	inline std::vector<char> serialize() {
		std::vector<char> data(sizeof(C));
		(*reinterpret_cast<C*>(&data[0])) = (*static_cast<C*>(this));
		return data;
	}
	/** Default deserialization method.
	@param	data		serialized component data. */
	inline void deserialize(const char* data) {
		(*static_cast<C*>(this)) = (*(C*)(&data[0]));
	}
	/** Save this component to a char buffer.
	@return				serialized version of self. */
	inline virtual std::vector<char> to_buffer() override {
		// Get portable name data
		const auto charCount = (int)std::string(chars).size();
		std::vector<char> nameData(sizeof(unsigned int) + (charCount * sizeof(char)));
		std::memcpy(&nameData[0], &charCount, sizeof(int));
		std::memcpy(&nameData[sizeof(int)], chars, charCount);
		// Get serial data
		auto classData = static_cast<C*>(this)->serialize();
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
	inline virtual void recover_data(const char* data) override final {
		// Previously recovered type name, created this class
		// Next recover data
		static_cast<C*>(this)->deserialize(data);

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
@param	entityHandle	handle to the component's parent entity, the one who'll own this component.
@param	comp			temporary pre-constructed component to copy data from, or nullptr.
@return					the index into the memory array where this component was created at. */
template <typename C>
inline constexpr static const int createFn(ComponentDataSpace& memory, const ComponentHandle& componentHandle, const EntityHandle& entityHandle, const ecsBaseComponent* comp = nullptr) {
	const size_t index = memory.size();
	memory.resize(index + sizeof(C));
	C* component = comp ? new(&memory[index])C(*(C*)comp) : new(&memory[index])C();
	component->m_handle = componentHandle;
	component->m_entity = entityHandle;
	return (int)index;
}

/** Construct a new component of type <C> on the heap.
@return					shared pointer to the new component. */
template <typename C>
inline constexpr static const auto newFn() {
	return std::make_shared<C>();
}

/** Destructs the supplied component, invalidating the memory range it occupied.
@param	comp			the component to destruct. */
template <typename C>
inline constexpr static const void freeFn(ecsBaseComponent* comp) {
	(static_cast<C*>(comp))->~C();
}

/** Generate a static ID at run time for each type of component class used. */
template <typename C, const char* chars>
const ComponentID ecsComponent<C, chars>::Runtime_ID(registerType(createFn<C>, freeFn<C>, newFn<C>, sizeof(C), chars));

#endif // ECS_COMPONENT_H