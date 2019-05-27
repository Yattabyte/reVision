#pragma once
#ifndef ECSCOMPONENT_H
#define ECSCOMPONENT_H

#include <any>
#include <limits>
#include <tuple>
#include <vector>


struct BaseECSComponent;
using EntityHandle = void*;
using ECSComponentCreateFunction = const uint32_t(*)(std::vector<uint8_t>& memory, const EntityHandle & entity, BaseECSComponent * comp);
using ECSComponentFreeFunction = void(*)(BaseECSComponent * comp);
using ParamList = std::vector<std::any>;
#define NULL_ENTITY_HANDLE nullptr

/** A base type component. */
struct BaseECSComponent {
public:
	// Public Methods
	static const uint32_t registerComponentType(ECSComponentCreateFunction createfn, ECSComponentFreeFunction freefn, const size_t & size);
	inline static ECSComponentCreateFunction getTypeCreateFunction(const uint32_t & id) { return std::get<0>((*componentTypes)[id]); }
	inline static ECSComponentFreeFunction getTypeFreeFunction(const uint32_t & id) { return std::get<1>((*componentTypes)[id]); }
	inline static size_t getTypeSize(const uint32_t & id) { return std::get<2>((*componentTypes)[id]); }
	inline static const bool isTypeValid(const uint32_t & id) { return id < componentTypes->size(); }

	
	// Public Attributes
	EntityHandle entity = NULL_ENTITY_HANDLE;


private:
	// Private Attributes
	static std::vector<std::tuple<ECSComponentCreateFunction, ECSComponentFreeFunction, size_t> > * componentTypes;
};

/** A specialized, specific type of component.
@param	T	the type of this component */
template <typename T>
struct ECSComponent : public BaseECSComponent {
	static const ECSComponentCreateFunction CREATE_FUNCTION;
	static const ECSComponentFreeFunction FREE_FUNCTION;
	static const uint32_t ID;
	static const size_t SIZE;
};

template <typename Component>
inline const uint32_t ECSComponentCreate(std::vector<uint8_t> & memory, const EntityHandle & entity, BaseECSComponent * comp) {
	const size_t index = memory.size();
	memory.resize(index+Component::SIZE);
	Component * component = new(&memory[index])Component(*(Component*)comp);
	component->entity = entity;
	return (uint32_t)index;
}

template <typename Component>
inline void ECSComponentFree(BaseECSComponent * comp) {
	Component * component = (Component*)comp;
	component->~Component();
}

template <typename T>
const uint32_t ECSComponent<T>::ID(BaseECSComponent::registerComponentType(ECSComponentCreate<T>, ECSComponentFree<T>, sizeof(T)));

template <typename T>
const size_t ECSComponent<T>::SIZE(sizeof(T));

template <typename T>
const ECSComponentCreateFunction ECSComponent<T>::CREATE_FUNCTION(ECSComponentCreate<T>);

template <typename T>
const ECSComponentFreeFunction ECSComponent<T>::FREE_FUNCTION(ECSComponentFree<T>);

template <typename T>
inline static T CastAny(const std::any & parameter, const T & fallback) {
	if (parameter.has_value() && parameter.type() == typeid(T))
		return std::any_cast<T>(parameter);
	return fallback;
}

#endif // ECSCOMPONENT_H