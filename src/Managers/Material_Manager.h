#pragma once
#ifndef MATERIAL_MANAGER
#define MATERIAL_MANAGER
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define MAX_NUM_MAPPED_TEXTURES 500	

#include "Utilities\GL\MappedBuffer_Exp.h"
#include "GL\glew.h"
#include <deque>
#include <vector>
#include <shared_mutex>

using namespace std; 


/**
 * Manages the creation and storage of materials, and to a lesser degree their destruction. * 
 * - How this works:
 *		- This stores an array of GLuint64 bindless texture handles
 *		- That array is stored as a shader storage buffer object (SSBO)
 *		- Pieces of geometry requests a spot in the material buffer array:
 *			- this provides an int index
 *			- geometry stores it alongside vertices (MUST NOT be interpolated across face)
 *		- Texture accessed in fragment shader by passing in the index spot from vertex shader to fragment shader:
 *			- get GLuint64 from SSBO's array using index
 *			- transform into sampler
 * - Uses bindless textures to circumvent slow texture binding 
 **/
class DT_ENGINE_API Material_Manager 
{
public:
	// Public Methods
	/** Singleton GET method.
	 * @return	static Material_Manager instance */
	static Material_Manager &Get() {
		static Material_Manager instance;
		return instance;
	}

	/** Start up and initialize the material manager */
	static void Start_Up() { (Get())._startup(); }

	/** Shut down and flush out the material manager */
	static void Shut_Down() { (Get())._shutdown(); }

	/** Make this buffer active. */
	static void Bind();

	/** Generates a material ID 
	 * @return	a new material ID */
	static GLuint Generate_ID();

	/** Generates a 64-bit GLuint64 texture handle for the specified texture, and makes it resident on the GPU
	 * @param	materialightingFBOID	the material buffer
	 * @param	glTextureID			the texture to generate the handle for **/
	static void Generate_Handle(const GLuint & materialightingFBOID, const GLuint & glTextureID);

	/** Tick through the work orders */
	static void Parse_Work_Orders();


private:
	// (de)Constructors
	~Material_Manager() {};
	Material_Manager();
	Material_Manager(Material_Manager const&) = delete;


	// Public Methods
	void operator=(Material_Manager const&) = delete;
	void _startup();
	void _shutdown();


	// Public Attributes
	bool m_Initialized; 
	shared_mutex m_DataMutex;
	unsigned int m_Count;
	deque<unsigned int> m_FreeSpots;
	vector<GLuint64> m_WorkOrders;
	MappedBuffer_Exp m_buffer;
};

#endif // MATERIAL_MANAGER
