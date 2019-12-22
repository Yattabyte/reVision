#pragma once
#ifndef	MATERIAL_H
#define	MATERIAL_H

#include "Assets/Image.h"
#include "glm/glm.hpp"

#define MAX_PHYSICAL_IMAGES 6
#define MAX_DIGITAL_IMAGES 3


// Forward Declarations
class Material;

/** Shared version of a Material asset.
Responsible for the creation, containing, and sharing of assets. */
class Shared_Material final : public std::shared_ptr<Material> {
public:
	// Public (De)Constructors
	/** Constructs an empty asset. */
	inline Shared_Material() noexcept = default;
	/** Begins the creation process for this asset.
	@param	engine			reference to the engine to use. 
	@param	filename		the filename to use.
	@param	textures		the textures to use.
	@param	threaded		create in a separate thread.
	@return					the desired asset. */
	explicit Shared_Material(Engine& engine, const std::string& filename, const std::vector<std::string>& textures, const bool& threaded = true);
};

/** A collection of textures which together form 1 or more PBR surfaces.
A material is a collection of textures that are used to approximate a real surface, such as:
	- Albedo
	- normal
	- metalness
	- roughness
	- height (WIP)
	- occlusion
@note	supports omission of any or all of the files.
@note	expects all textures in a material to be the same dimension, and will forcefully resize them (in memory).
@note	can contain multiple sets of these textures, where each set of 6 form 1 skin for an object.
@note	owns many Shared_Image objects. */
class Material final : public Asset {
public:
	// Public (De)Constructors
	/** Destroy the Material. */
	inline ~Material() = default;
	/** Construct the Material.
	@param	engine			reference to the engine to use. 
	@param	filename		the asset file name (relative to engine directory).
	@param	textures		the textures to use. */
	Material(Engine& engine, const std::string& filename, const std::vector<std::string>& textures);


	// Public Methods
	/** Reading from a .mat file, retrieves the individual file names assigned to this material.
	@brief					Updates the appropriate supplied string with a path to the appropriate file.
	@param	filename		the absolute file path of the '.mat' file to read from.
	@return					the vector storing texture directories. */
	static std::vector<std::string> Get_Material_Textures(const std::string& filename);


	// Public Attributes
	std::vector<GLubyte> m_materialData;
	glm::ivec2 m_size = glm::ivec2(1);
	std::vector<std::string> m_textures;
	std::vector<Shared_Image> m_images;


private:
	// Private but deleted
	/** Disallow asset move constructor. */
	inline Material(Material&&) noexcept = delete;
	/** Disallow asset copy constructor. */
	inline Material(const Material&) noexcept = delete;
	/** Disallow asset move assignment. */
	inline const Material& operator =(Material&&) noexcept = delete;
	/** Disallow asset copy assignment. */
	inline const Material& operator =(const Material&) noexcept = delete;


	// Private Interface Implementation
	void initialize() final;


	// Private Attributes
	friend class Shared_Material;
};

#endif // MATERIAL_H