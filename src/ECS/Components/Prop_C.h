#pragma once
#ifndef MODEL_C_H
#define MODEL_C_H

#include "ECS\Components\ecsComponent.h"
#include "Assets\Asset_Model.h"
#include "Utilities\GL\VectorBuffer.h"
#include "Utilities\Transform.h"


class Engine;
/** OpenGL buffer for basic props.
Formatted to adhere to glsl specifications. */
struct Prop_Buffer {
	GLuint materialID; glm::vec3 padding1;
	glm::mat4 mMatrix;
	glm::mat4 bBoxMatrix;
};
/** A prop component. 
@note		On its own it provides no support for animation. */
struct Prop_Component : public ECSComponent<Prop_Component> {
	Shared_Asset_Model m_model;
	VB_Element<Prop_Buffer> * m_data = nullptr;
	float m_radius = 1.0f;
	glm::vec3 m_position = glm::vec3(0.0f);
};
/** A constructor to aid in creation. */
struct Prop_Constructor : ECSComponentConstructor<Prop_Component> {
	// Public (de)Constructors
	Prop_Constructor(Engine * engine, VectorBuffer<Prop_Buffer> * elementBuffer) 
		: m_engine(engine), m_elementBuffer(elementBuffer) {};


	// Interface Implementation
	inline virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		auto directory = castAny(parameters[0], std::string(""));
		auto material = castAny(parameters[1], 0u);
		auto * component = new Prop_Component();
		component->m_data = m_elementBuffer->newElement();
		component->m_model = Asset_Model::Create(m_engine, directory);
		component->m_data->data->materialID = material;
		return { component, component->ID };
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	VectorBuffer<Prop_Buffer> * m_elementBuffer = nullptr;
};

#endif // MODEL_C_H