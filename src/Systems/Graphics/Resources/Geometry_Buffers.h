#pragma once
#ifndef GEOMETRY_BUFFERS_H
#define GEOMETRY_BUFFERS_H

#include "Systems\Graphics\Resources\GFX_DEFINES.h"
#include "Utilities\GL\VectorBuffer.h"


class Geometry_Buffers {
public:
	// Constructors
	~Geometry_Buffers()						{};
	Geometry_Buffers()						{};


	// Public Attributes
	VectorBuffer<Geometry_Static_Struct>	m_geometryStaticSSBO;
	VectorBuffer<Geometry_Dynamic_Struct>	m_geometryDynamicSSBO;


private:
	// Private Attributes
	Geometry_Buffers(const Geometry_Buffers &); // Never copy this class
};

#endif // GEOMETRY_BUFFERS_H