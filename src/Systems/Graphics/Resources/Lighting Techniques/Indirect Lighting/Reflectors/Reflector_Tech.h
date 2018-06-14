#pragma once
#ifndef REFLECTOR_TECH_H
#define REFLECTOR_TECH_H

#include "GL\glew.h"


class Reflector_Tech {
public:
	// (de)Constructors
	/** Virtual Destructor */
	virtual ~Reflector_Tech() {}
	/** Constructor */
	Reflector_Tech() {}


	// Interface Declarations
	virtual void applyPrePass() = 0;
	virtual void applyEffect() = 0;
};

#endif //REFLECTOR_TECH_H