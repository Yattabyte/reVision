#pragma once
#ifndef ECSCOMPONENT_H
#define ECSCOMPONENT_H

#include <any>
#include <limits>
#include <tuple>
#include <vector>


struct BaseECSComponent;
using EntityHandle = void*;
using ECSComponentCreateFunction = const unsigned int(*)(std::vector<unsigned int>& memory, const EntityHandle & entity, BaseECSComponent * comp);
using ECSComponentFreeFunction = void(*)(BaseECSComponent * comp);
#define NULL_ENTITY_HANDLE nullptr

/** A base type component. */
struct BaseECSComponent {
public:
	// Public Methods
	static const unsigned int registerComponentType(ECSComponentCreateFunction createfn, ECSComponentFreeFunction freefn, const size_t & size);
	inline static ECSComponentCreateFunction getTypeCreateFunction(const unsigned int & id) { return std::get<0>((*componentTypes)[id]); }
	inline static ECSComponentFreeFunction getTypeFreeFunction(const unsigned int & id) { return std::get<1>((*componentTypes)[id]); }
	inline static size_t getTypeSize(const unsigned int & id) { return std::get<2>((*componentTypes)[id]); }
	inline static const bool isTypeValid(const unsigned int & id) { return id < componentTypes->size(); }

	
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
	static const unsigned int ID;
	static const size_t SIZE;
};

template <typename Component>
const unsigned int ECSComponentCreate(std::vector<unsigned int> & memory, const EntityHandle & entity, BaseECSComponent * comp) {
	const unsigned int index = memory.size();
	memory.resize(index+Component::SIZE);
	Component * component = new(&memory[index])Component(*(Component*)comp);
	component->entity = entity;
	return index;
}

template <typename Component>
void ECSComponentFree(BaseECSComponent * comp) {
	Component * component = (Component*)comp;
	component->~Component();
}

template <typename T>
const unsigned int ECSComponent<T>::ID(BaseECSComponent::registerComponentType(ECSComponentCreate<T>, ECSComponentFree<T>, sizeof(T)));

template <typename T>
const size_t ECSComponent<T>::SIZE(sizeof(T));

template <typename T>
const ECSComponentCreateFunction ECSComponent<T>::CREATE_FUNCTION(ECSComponentCreate<T>);

template <typename T>
const ECSComponentFreeFunction ECSComponent<T>::FREE_FUNCTION(ECSComponentFree<T>);

struct Component_and_ID {
	BaseECSComponent * component = nullptr;
	unsigned int id = (unsigned)(-1);

	const bool success() const {
		return bool(component && id != (unsigned)(-1));
	}
	void clear() {
		component = nullptr;
		id = (unsigned)(-1);
	}
};
struct BaseECSComponentConstructor {	
	// Public Interface
	virtual Component_and_ID construct(const std::vector<std::any> & parameters) = 0;


protected:
	// Protected Methods
	template <typename ParamT>
	inline ParamT const castAny(const std::any & parameter, const ParamT & fallback) const {
		if (parameter.has_value() && parameter.type() == typeid(ParamT))
			return std::any_cast<ParamT>(parameter);
		return fallback;
	}
};

template <typename BaseECSComponent>
struct ECSComponentConstructor : BaseECSComponentConstructor {
	// Default Interface Implementation
	virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		auto * component = new BaseECSComponent();
		return { component, component->ID };
	}
};

#endif // ECSCOMPONENT_H