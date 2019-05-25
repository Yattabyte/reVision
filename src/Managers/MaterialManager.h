#pragma once
#ifndef MATERIALMANAGER_H
#define MATERIALMANAGER_H

#include "Utilities/GL/glad/glad.h"
#include <deque>
#include <shared_mutex>


class Engine;

/** Manages the creation and storage of materials, and to a lesser degree their destruction. * 
* Creates a gigantic texture array for all the model's material textures
* Uses sparse textures to avoid consuming all the memory at once. */
class MaterialManager {
public:
	// Public (de)Constructors
	/** Destroy the material manager. */
	~MaterialManager();
	/** Construct the material manager. */
	MaterialManager(Engine * engine);
	

	// Public Functions
	/** Make this buffer active. */
	void bind();
	/** Retrieve the required size materials must be to fit in this buffer. */
	GLsizei getMaterialSize() const;
	/** Generates a material ID 
	@param	textureCount	the number of textures used for this material
	@return					a new material ID */
	GLuint generateID(const size_t & textureCount);
	/** Commit image data to the GPU, creating a full material. 
	@param	materialID		the material to commit data to.
	@param	imageData		the image data to source from.
	@param	imageCount		the number of images. */
	void writeMaterials(const GLuint & materialID, const GLubyte * imageData, const GLsizei & imageCount);
	/** Returns whether or not this manager is ready to use.
	@return					true if all work is finished, false otherwise. */
	bool readyToUse();
	/** Returns whether or not any changes have occured to this manager since the last check
	@return					true if any changes occured, false otherwise */
	bool hasChanged();


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