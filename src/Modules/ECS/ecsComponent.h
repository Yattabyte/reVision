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
using ComponentCreateFunction = std::function<ComponentID(ComponentDataSpace& memory, const ecsHandle& entityHandle, const ecsBaseComponent* comp)>;
using ComponentFreeFunction = std::function<void(ecsBaseComponent* comp)>;


/** A base class representing components in an ECS architecture.*/
struct ecsBaseComponent {
	// Public (de)Constructors
	inline virtual ~ecsBaseComponent() = default;
	inline ecsBaseComponent() = default;
	inline ecsBaseComponent(const ComponentID& ID, const size_t& size, const char* name)
		: m_ID(ID), m_size(size), m_name(name) {}


	// Public Interface Declarations
	/** Save this component to a char buffer. 
	@return				serialized component data. */
	virtual std::vector<char> save() = 0;
	/** Load into this component data from a char buffer.
	param	data		serialized component data. */
	virtual void load(const std::vector<char>& data) = 0;
	/** Generate a clone of this component. 
	@return				clone of this component. */
	virtual ecsBaseComponent* clone() const = 0;


	// Public Attributes
	/** Runtime generated ID per class. */
	ComponentID m_ID; 
	/** Total component byte-size. */
	size_t m_size;
	/** Specific class name. */
	const char* m_name;
	/** Parent entity's UUID. */
	ecsHandle m_entity;


protected:
	// Protected Methods
	/** Register a specific sub-class component into the component registry for creating and freeing them at runtime. 
	@param	createFn	function for creating a specific component type within an input memory block.
	@param	freeFn		function for freeing a specific component type from its memory block.
	@param	size		the total size of a single component.
	@param	string		type-name of the component, for name-lookups between since component ID's can change.
	@param	templateC	template component used for copying from. 
	@return				runtime component ID. */
	static ComponentID registerType(const ComponentCreateFunction& createFn, const ComponentFreeFunction& freeFn, const size_t& size, const char* string, ecsBaseComponent* templateC);


	// Protected Attributes
	/** Runtime container mapping indicies to creation/destruction functions for components. */
	static std::vector<std::tuple<ComponentCreateFunction, ComponentFreeFunction, size_t>> _componentRegister;
	/** A map between component class name's and it's runtime variables like ID and size. */
	static MappedChar<std::tuple<ecsBaseComponent*, ComponentID, size_t>> _templateMap;
	/** Allow the ecs world to interact with these members. */
	friend class ecsWorld;
};

/** A specialized, specific type of component.
@param	C	the type of this component */
template <typename C, const char* chars>
struct ecsComponent : public ecsBaseComponent {
	// (de)Constructors
	inline virtual ~ecsComponent() = default;
	inline ecsComponent() : ecsBaseComponent(ecsComponent::m_ID, sizeof(C), chars) {}


	// Public Methods
	/** Return the run-time size of this component's class.
	@return				the size of this component. */
	inline constexpr static const size_t SIZE() { return sizeof(C); }
	/** Return the run-time class name of this component's class.
	@return				class name. */
	inline constexpr static const char* CHARS() { return chars; }


	// Interface Implementation
	inline virtual std::vector<char> save() override {
		// First retrieve the name of this component
		const auto stringSize = std::string(chars).size();
		std::vector<char> output(sizeof(unsigned int) + (stringSize * sizeof(char)));
		const auto nameCount = (int)stringSize;
		std::memcpy(&output[0], &nameCount, sizeof(int));
		std::memcpy(&output[sizeof(int)], chars, stringSize);

		const auto data = static_cast<C*>(this)->serialize();
		output.insert(output.end(), data.begin(), data.end());
		return output;
	}
	inline virtual void load(const std::vector<char>& data) override {
		static_cast<C*>(this)->deserialize(data);
		m_entity = ecsHandle();
	}
	inline virtual ecsBaseComponent* clone() const override {
		return new C(static_cast<C const&>(*this));
	}


	// Interface Declaration (for specific components)
	/** Serialize this component to a char buffer. Default behaviour memcpy's self. 
	@return				serialized component data. */
	inline virtual std::vector<char> serialize() {
		std::vector<char> data(sizeof(C));
		std::memcpy(&data[0], static_cast<C*>(this), sizeof(C));
		return data;
	}
	/** Deserialize buffer data into this component.
	@param	data		serialized component data. */
	inline virtual void deserialize(const std::vector<char>& data) {
		(*static_cast<C*>(this)) = (*(C*)(&data[0]));
	}


	// Static Type-Specific Attributes
	/** Runtime generated ID per class, also stored in base-component. */
	static const ComponentID m_ID;
};


/////////////////////////////////////
/// Static Member Initializations ///
/////////////////////////////////////

/** Constructs a new component of type <C> into the memory space provided.
@param	memory			raw data vector representing all components of type <C>.
@param	entityHandle	handle to the component's parent entity, the one who'll own this component.
@param	comp			temporary pre-constructed component to copy data from. 
@return					component ID - the index into the memory array where this component was created at. */
template <typename C>
inline constexpr static const ComponentID createFn(ComponentDataSpace& memory, const ecsHandle& entityHandle, const ecsBaseComponent* comp) {
	const size_t index = memory.size();
	memory.resize(index + sizeof(C));
	(new(&memory[index])C(*(C*)comp))->m_entity = entityHandle;
	return (ComponentID)index;
}
/** Destructs the supplied component, invalidating the memory range it occupied.
@param	comp			the component to destruct. */
template <typename C>
inline constexpr static const void freeFn(ecsBaseComponent* comp) {
	C* component = (C*)comp;
	component->~C();
}
template <typename C, const char* chars>
const ComponentID ecsComponent<C, chars>::m_ID( registerType(createFn<C>, freeFn<C>, sizeof(C), chars, new C()) );

#endif // ECS_COMPONENT_H