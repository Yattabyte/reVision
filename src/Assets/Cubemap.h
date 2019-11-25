#pragma once
#ifndef	CUBEMAP_H
#define	CUBEMAP_H

#include "Assets/Image.h"
#include "glm/glm.hpp"


class Engine;
class Cubemap;

/** Shared version of a Cubemap asset.
Responsible for the creation, containing, and sharing of assets. */
class Shared_Cubemap final : public std::shared_ptr<Cubemap> {
public:
	// Public (De)Constructors
	/** Constructs an empty asset. */
	inline Shared_Cubemap() noexcept = default;
	/** Begins the creation process for this asset.
	@param	engine			the engine being used.
	@param	filename		the filename to use.
	@param	threaded		create in a separate thread.
	@return					the desired asset. */
	Shared_Cubemap(Engine& engine, const std::string& filename, const bool& threaded = true) noexcept;
};

/** A cubemap texture object.
Wraps an OpenGL texture object for cube-maps, dealing with the file fetching and uploading for all 6 images.
@note	owns 6 Shared_Image objects. */
class Cubemap final : public Asset {
public:
	// Public (De)Constructors
	/** Destroy the Cubemap. */
	~Cubemap() noexcept;
	/** Construct the Cubemap.
	@param	engine		the engine to use.
	@param	filename	the asset file name (relative to engine directory). */
	Cubemap(Engine& engine, const std::string& filename) noexcept;


	// Public Methods
	/** Makes this texture active at a specific texture unit.
	@param	texture_unit	the desired texture unit to make this texture active at. */
	void bind(const unsigned int& texture_unit) noexcept;


	// Public Attributes
	GLuint m_glTexID = 0, m_pboIDs[6] = { 0,0,0,0,0,0 };
	Shared_Image m_images[6];


private:
	// Private Interface Implementation
	virtual void initialize() noexcept override final;


	// Private Attributes
	friend class Shared_Cubemap;
};
#endif // CUBEMAP_H
