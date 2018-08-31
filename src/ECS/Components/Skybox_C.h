#pragma once
#ifndef SKYBOX_C_H
#define SKYBOX_C_H

#include "ECS\Components\ecsComponent.h"
#include "Assets\Asset_Cubemap.h"


class Engine;
/** A skybox component holds all the data for a skybox. */
struct Skybox_Component : public ECSComponent<Skybox_Component> {
	Shared_Asset_Cubemap m_texture;
};
/** A constructor to aid in creation. */
struct Skybox_Constructor : ECSComponentConstructor<Skybox_Component> {
	Skybox_Constructor(Engine * engine) : m_engine(engine) {};
	// Interface Implementation
	virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		auto directory = castAny(parameters[0], std::string(""));
		auto * component = new Skybox_Component();
		component->m_texture = Asset_Cubemap::Create(m_engine, directory);
		return { component, component->ID };
	}
	Engine * m_engine;
};

#endif // SKYBOX_C_H