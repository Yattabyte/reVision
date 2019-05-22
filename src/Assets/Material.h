#pragma once
#ifndef	MATERIAL_H
#define	MATERIAL_H
#define MAX_PHYSICAL_IMAGES 6
#define MAX_DIGITAL_IMAGES 3

#include "Assets/Image.h"
#include "Managers/MaterialManager.h"
#include "GL/glad/glad.h"
#include "glm/glm.hpp"
#include <vector>


class Engine;
class Material;

/** Responsible for the creation, containing, and sharing of assets. */
class Shared_Material : public std::shared_ptr<Material> {
public:
	Shared_Material() = default;
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	textures		the textures to use
	@param	threaded		create in a separate thread
	@return					the desired asset */
	explicit Shared_Material(Engine * engine, const std::string & filename, const std::vector<std::string> &textures, const bool & threaded = true);
};


/** A collection of texture surfaces that are used together to approximate a real surface.
- Support for:
	- Albedo
	- normal
	- metalness
	- roughness
	- height (WIP)
	- occlusion (in conjunction with SSAO)
- Supports omission of any or all of the files
- Expects all textures in a material to be the same dimension, and will forcefully resize them (in memory). */
class Material : public Asset
{
public:
	/** Destroy the Material. */
	~Material();
	/** Construct the Material. */
	Material(Engine * engine, const std::string & filename, const std::vector<std::string> & textures, MaterialManager & materialManager);
	

	// Public Methods
	/** Reading from a .mat file, retrieves the individual file names assigned to this material
	@brief					Updates the appropriate supplied @std::string's with a path to the appropriate file
	@param	filename		the absolute file path of the '.mat' file to read from
	@return					the vector storing texture directories */
	static std::vector<std::string> Get_Material_Textures(const std::string & filename);
	
	
	// Public Attributes
	GLuint m_pboID = 0;
	GLuint m_glArrayID = 0;
	GLuint m_matSpot = 0;
	GLubyte * m_materialData = nullptr;
	glm::ivec2 m_size = glm::ivec2(1);
	std::vector<std::string> m_textures;
	std::vector<Shared_Image> m_images;


private:
	// Private Interface Implementation
	virtual void initialize() override;


	// Private Attributes
	friend class Shared_Material;
};

#endif // MATERIAL_H