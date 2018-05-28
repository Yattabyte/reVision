#pragma once
#ifndef GEOMETRY_BUFFERS
#define GEOMETRY_BUFFERS
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\Graphics\Resources\GFX_DEFINES.h"
#include "Utilities\GL\VectorBuffer.h"

class DT_ENGINE_API Geometry_Buffers {
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

#endif // GEOMETRY_BUFFERS