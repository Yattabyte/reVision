#pragma once
#ifndef LIGHT_BUFFERS_H
#define LIGHT_BUFFERS_H

#include "Systems\Graphics\Resources\GFX_DEFINES.h"
#include "Utilities\GL\VectorBuffer.h"

class Light_Buffers {
public:
	// Constructors
	~Light_Buffers()						{};
	Light_Buffers()							{};


	// Public Attributes
	VectorBuffer<Directional_Struct>		m_lightDirSSBO;
	VectorBuffer<Directional_Cheap_Struct>	m_lightDirCheapSSBO;
	VectorBuffer<Point_Struct>				m_lightPointSSBO;
	VectorBuffer<Point_Cheap_Struct>		m_lightPointCheapSSBO;
	VectorBuffer<Spot_Struct>				m_lightSpotSSBO;
	VectorBuffer<Spot_Cheap_Struct>			m_lightSpotCheapSSBO;


private:
	// Private Attributes
	Light_Buffers(const Light_Buffers &); // Never copy this class
};

#endif // LIGHT_BUFFERS_H