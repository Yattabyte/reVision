#pragma once
#ifndef ECSCOMPONENT_H
#define ECSCOMPONENT_H

#include <any>
#include <limits>
#include <map>
#include <string>
#include <tuple>
#include <vector>


struct ecsEntity;
struct BaseECSComponent;
using ECSComponentCreateFunction = const int(*)(std::vector<uint8_t>& memory, ecsEntity* entity, BaseECSComponent * comp);
using ECSComponentFreeFunction = void(*)(BaseECSComponent * comp);
using ParamList = std::vector<std::any>;
#define NULL_ENTITY_HANDLE nullptr

/** A base type component. */
struct BaseECSComponent {
public:
	// Virtual Destructor
	virtual ~BaseECSComponent() = default;


	// Public Methods
	static int registerComponentType(ECSComponentCreateFunction createfn, ECSComponentFreeFunction freefn, const size_t & size, const char * string, BaseECSComponent * templateComponent);
	static std::tuple<BaseECSComponent *, int, size_t> findTemplate(const char * name);
	static const char * findName(const int& id);
	static int findID(const char* name);
	inline static ECSComponentCreateFunction getTypeCreateFunction(const int & id) { return std::get<0>((*componentTypes)[id]); }
	inline static ECSComponentFreeFunction getTypeFreeFunction(const int & id) { return std::get<1>((*componentTypes)[id]); }
	inline static size_t getTypeSize(const int & id) { return std::get<2>((*componentTypes)[id]); }
	inline static bool isTypeValid(const int & id) { return id < componentTypes->size(); }
	virtual int get_id() = 0;
	virtual BaseECSComponent * clone() = 0;
	virtual std::vector<char> save() = 0;
	virtual void load(const std::vector<char> & data) = 0;


	// Public Attributes
	ecsEntity * entity = NULL_ENTITY_HANDLE;


private:
	// Private Attributes
	static std::vector<std::tuple<ECSComponentCreateFunction, ECSComponentFreeFunction, size_t> > * componentTypes;
	static std::vector<const char*>* componentNames;
	struct compare_string { bool operator()(const char * a, const char * b) const { return strcmp(a, b) < 0; } };
	static std::map<const char *, std::tuple<BaseECSComponent *, int, size_t>, compare_string> * templateMap;
};

/** A specialized, specific type of component.
@param	T	the type of this component */
template <typename T, const char * chars>
struct ECSComponent : public BaseECSComponent {
	static const ECSComponentCreateFunction CREATE_FUNCTION;
	static const ECSComponentFreeFunction FREE_FUNCTION;
	static const int ID;
	static std::string STRING_NAME;
	static const size_t SIZE;
	inline constexpr static const char* NAME() {
		return chars;
	}
	inline virtual int get_id() override {
		return ID;
	}
	inline virtual BaseECSComponent * clone() override {
		return new T(static_cast<T const &>(*this));
	}
	inline std::vector<char> save() override {
		// First retrieve the name of this component
		std::vector<char> output(sizeof(unsigned int) + (STRING_NAME.size() * sizeof(char)));
		const auto nameCount = (int)STRING_NAME.size();
		std::memcpy(&output[0], &nameCount, sizeof(int));
		std::memcpy(&output[sizeof(int)], STRING_NAME.data(), STRING_NAME.size());

		const auto data = static_cast<T*>(this)->serialize();
		output.insert(output.end(), data.begin(), data.end());
		return output;
	}
	inline virtual std::vector<char> serialize() {
		std::vector<char> data(sizeof(T));
		std::memcpy(&data[0], static_cast<T*>(this), sizeof(T));
		return data;
	}
	inline virtual void load(const std::vector<char> & data) override {
		static_cast<T*>(this)->deserialize(data);
		entity = NULL_ENTITY_HANDLE;
	}
	inline virtual void deserialize(const std::vector<char> & data) {
		(*static_cast<T*>(this)) = (*(T*)(&data[0]));
	}
};

template <typename Component>
inline const int ECSComponentCreate(std::vector<uint8_t> & memory, ecsEntity* entity, BaseECSComponent * comp) {
	const size_t index = memory.size();
	memory.resize(index + Component::SIZE);
	(new(&memory[index])Component(*(Component*)comp))->entity = entity;
	return (int)index;
}

template <typename Component>
inline void ECSComponentFree(BaseECSComponent * comp) {
	((Component*)comp)->~Component();
}

template <typename T, const char* chars>
const int ECSComponent<T, chars>::ID(BaseECSComponent::registerComponentType(ECSComponentCreate<T>, ECSComponentFree<T>, sizeof(T), chars, new T()));

template <typename T, const char* chars>
std::string ECSComponent<T, chars>::STRING_NAME(chars);

template <typename T, const char * chars>
const size_t ECSComponent<T, chars>::SIZE(sizeof(T));

template <typename T, const char * chars>
const ECSComponentCreateFunction ECSComponent<T, chars>::CREATE_FUNCTION(ECSComponentCreate<T>);

template <typename T, const char * chars>
const ECSComponentFreeFunction ECSComponent<T, chars>::FREE_FUNCTION(ECSComponentFree<T>);

/**@todo delete*/
template <typename T>
inline static T CastAny(const ParamList & parameters, const int & index, const T & fallback) {
	if (index < parameters.size()) {
		if (const auto & parameter = parameters[index]; parameter.has_value() && parameter.type() == typeid(T))
			return std::any_cast<T>(parameter);
	}
	return fallback;
}

#endif // ECSCOMPONENT_H