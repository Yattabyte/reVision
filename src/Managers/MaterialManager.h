#pragma once
#ifndef MATERIALMANAGER_H
#define MATERIALMANAGER_H
#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define MAX_NUM_MAPPED_TEXTURES 500	

#include "Utilities\GL\DynamicBuffer.h"
#include "GL\glew.h"
#include <deque>
#include <vector>
#include <shared_mutex>


/** Manages the creation and storage of materials, and to a lesser degree their destruction. * 
- How this works:
	- This stores an array of GLuint64 bindless texture handles
	- That array is stored as a shader storage buffer object (SSBO)
	- Pieces of geometry requests a spot in the material buffer array:
		- this provides an int index
		- geometry stores it alongside vertices (MUST NOT be interpolated across face)
	- Texture accessed in fragment shader by passing in the index spot from vertex shader to fragment shader:
		- get GLuint64 from SSBO's array using index
		- transform into sampler
- Uses bindless textures to circumvent slow texture binding */
class MaterialManager {
public:
	// (de)Constructors
	/** Destroy the material manager. */
	~MaterialManager();
	/** Construct the material manager. */
	MaterialManager();
	

	// Public Functions
	/** Initialzie the material manager. */
	void initialize();
	/** Make this buffer active. */
	void bind();
	/** Generates a material ID 
	 @return						a new material ID */
	GLuint generateID();
	/** Generates a 64-bit GLuint64 texture handle for the specified texture, and makes it resident on the GPU
	@param	materialightingFBOID	the material buffer
	@param	glTextureID				the texture to generate the handle for **/
	void generateHandle(const GLuint & materialightingFBOID, const GLuint & glTextureID);
	/** Tick through the work orders */
	void parseWorkOrders();
	/** Returns whether or not this manager has work left.
	@return							true if all work is finished, false otherwise. */
	const bool finishedWork();


private:
	// Private Attributes
	bool m_Initialized;
	std::shared_mutex m_DataMutex;
	unsigned int m_Count;
	std::deque<unsigned int> m_FreeSpots;
	std::vector<GLuint64> m_WorkOrders;
	DynamicBuffer * m_buffer;
};

#endif // MATERIALMANAGER_H