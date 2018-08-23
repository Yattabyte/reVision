#pragma once
#ifndef SKYBOX_C_H
#define SKYBOX_C_H

#include "ECS\Components\ecsComponent.h"
#include "Assets\Asset_Cubemap.h"


/** A skybox component holds all the data for a skybox. */
struct Skybox_Component : public ECSComponent<Skybox_Component> {
	Shared_Asset_Cubemap m_texture;
};

#endif // SKYBOX_C_H