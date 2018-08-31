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
	Transform m_transform;
	VB_Element<Prop_Buffer> * m_data;
};
/** A constructor to aid in creation. */
struct Prop_Constructor : ECSComponentConstructor<Prop_Component> {
	Prop_Constructor(Engine * engine, VectorBuffer<Prop_Buffer> * elementBuffer) 
		: m_engine(engine), m_elementBuffer(elementBuffer) {};
	// Interface Implementation
	virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		auto directory = castAny(parameters[0], std::string(""));
		auto material = castAny(parameters[1], 0u);
		auto position = castAny(parameters[2], glm::vec3(0.0f));
		auto orientation = castAny(parameters[3], glm::quat(1, 0, 0, 0));
		auto scale = castAny(parameters[4], glm::vec3(1.0f));
		auto * component = new Prop_Component();
		component->m_data = m_elementBuffer->newElement();
		component->m_model = Asset_Model::Create(m_engine, directory);
		component->m_transform.m_position = position;
		component->m_transform.m_orientation = orientation;
		component->m_transform.m_scale = scale;
		component->m_transform.update();
		component->m_data->data->materialID = material;
		component->m_data->data->mMatrix = component->m_transform.m_modelMatrix;
		component->m_data->data->bBoxMatrix = component->m_transform.m_modelMatrix;
		return { component, component->ID };
	}
	Engine * m_engine; 
	VectorBuffer<Prop_Buffer> * m_elementBuffer;
};

#endif // MODEL_C_H