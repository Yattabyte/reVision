#pragma once
#ifndef MATERIALMANAGER_H
#define MATERIALMANAGER_H
#define MAX_NUM_MAPPED_TEXTURES 500	

#include "Utilities\GL\DynamicBuffer.h"
#include "GL\glew.h"
#include <deque>
#include <shared_mutex>


class Engine;

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
	MaterialManager(Engine * engine);
	

	// Public Functions
	/** Make this buffer active. */
	void bind();
	/** Retrieve the required size materials must be to fit in this buffer. */
	const GLsizei getMaterialSize() const;
	/** Generates a material ID 
	@param	textureCount	the number of textures used for this material
	@return					a new material ID */
	GLuint generateID(const size_t & textureCount);
	/***/
	void writeMaterials(const GLuint & materialID, const GLubyte * imageData, const GLsizei & imageCount);
	/** Returns whether or not this manager is ready to use.
	@return					true if all work is finished, false otherwise. */
	const bool readyToUse();
	/** Returns whether or not any changes have occured to this manager since the last check
	@return					true if any changes occured, false otherwise */
	const bool hasChanged();


private:
	// Private Attributes
	std::shared_mutex m_DataMutex;
	size_t m_Count = 0;
	GLsizei m_materialSize = 512u;
	GLint m_textureLayers = 6, m_mipCount = 1;
	GLuint m_arrayID = 0;
	std::deque<GLsync> m_fences;
	bool m_changed = true;
};

#endif // MATERIALMANAGER_H