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
using ComponentCreateFunction = std::function<ComponentID(ComponentDataSpace& memory, const ecsHandle& entityHandle, const ecsBaseComponent* comp)>;
using ComponentFreeFunction = std::function<void(ecsBaseComponent* comp)>;


/** A base class representing components in an ECS architecture. */
struct ecsBaseComponent {
	// Public (de)Constructors
	inline virtual ~ecsBaseComponent() = default;
	inline ecsBaseComponent() = default;
	inline ecsBaseComponent(const ComponentID& ID, const size_t& size, const char* name)
		: m_ID(ID), m_size(size), m_name(name) {}


	// Public Methods
	/** Save this component to a char buffer.
	@return				serialized version of self. */
	virtual std::vector<char> to_buffer() = 0;
	/** Recover and generate a component from a char buffer.
	@param	data		serialized version of component.
	@param	dataRead	reference updated with the number of bytes read.
	@return				if successfull a shared pointer to a new component, nullptr otherwise. */
	static std::shared_ptr<ecsBaseComponent> from_buffer(const char* data, size_t& dataRead);
	/** Generate a clone of this component.
	@return				clone of this component. */
	virtual std::shared_ptr<ecsBaseComponent> clone() const = 0;


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
	/** Recover and load component data into this component from a char buffer.
	param	data		serialized component data. */
	virtual void recover_data(const char* data) = 0;


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
	/** Default serialization method.
	@return				serialized component data. */
	inline std::vector<char> serialize() {
		std::vector<char> data(sizeof(C));
		(*(C*)(&data[0])) = (*static_cast<C*>(this));
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
	/** Generate a clone of this component.
	@return				clone of this component. */
	inline virtual std::shared_ptr<ecsBaseComponent> clone() const override {
		const auto& component = std::shared_ptr<ecsBaseComponent>(new C(static_cast<C const&>(*this)));
		// Always clear the entity handle
		component->m_entity = ecsHandle();
		return component;
	}


	// Static Type-Specific Attributes
	/** Runtime class name of this component, also stored in base-component. */
	constexpr static const char* m_name = chars;
	/** Runtime generated ID per class, also stored in base-component. */
	static const ComponentID m_ID;


protected:
	// Protected Interface Implementation
	inline virtual void recover_data(const char* data) override {
		// Previously recovered type name, created this class
		// Next recover data
		static_cast<C*>(this)->deserialize(data);

		// Enforce runtime variables
		ecsBaseComponent::m_ID = ecsComponent::m_ID;
		ecsBaseComponent::m_size = sizeof(C);
		ecsBaseComponent::m_name = chars;

		// Always clear the entity handle
		ecsBaseComponent::m_entity = ecsHandle();
	}
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
inline constexpr static const int createFn(ComponentDataSpace& memory, const ecsHandle& entityHandle, const ecsBaseComponent* comp) {
	const size_t index = memory.size();
	memory.resize(index + sizeof(C));
	(new(&memory[index])C(*(C*)comp))->m_entity = entityHandle;
	return (int)index;
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