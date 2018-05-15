#pragma once
#ifndef LIGHT_BUFFERS
#define LIGHT_BUFFERS
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\Graphics\Resources\GFX_DEFINES.h"
#include "Utilities\GL\VectorBuffer.h"

class DT_ENGINE_API Light_Buffers {
public:
	// Constructors
	~Light_Buffers()						{};
	Light_Buffers()							{};


	// Public Attributes
	VectorBuffer<Directional_Struct>		m_lightDirSSBO;
	VectorBuffer<Directional_Cheap_Struct>	m_lightDirCheapSSBO;
	VectorBuffer<Point_Struct>				m_lightPointSSBO;
	VectorBuffer<Spot_Struct>				m_lightSpotSSBO;


private:
	// Private Attributes
	Light_Buffers(const Light_Buffers &); // Never copy this class
};

#endif // LIGHT_BUFFERS