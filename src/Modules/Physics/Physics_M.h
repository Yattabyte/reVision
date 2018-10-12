#pragma once
#ifndef PHYSICS_MODULE_H
#define PHYSICS_MODULE_H

#include "Modules\Engine_Module.h"


/** A module responsible for physics. */
class Physics_Module : public Engine_Module {
public:
	// (de)Constructors
	~Physics_Module();
	Physics_Module(Engine * engine);


	// Public Interface Implementation
	virtual void initialize() override;
};

#endif // PHYSICS_MODULE_H
