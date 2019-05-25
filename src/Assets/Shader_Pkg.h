#pragma once
#ifndef	SHADER_PKG_H
#define	SHADER_PKG_H

#include "Assets/Asset.h"
#include "Utilities/GL/glad/glad.h"
#include "glm/glm.hpp"
#include <string>


class Engine;
class Shader_Pkg;

/** Responsible for the creation, containing, and sharing of assets. */
class Shared_Shader_Pkg : public std::shared_ptr<Shader_Pkg> {
public:
	// Public (de)Constructors
	/** Constructs an empty asset. */
	inline Shared_Shader_Pkg() = default;
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	threaded		create in a separate thread
	@return					the desired asset */
	explicit Shared_Shader_Pkg(Engine * engine, const std::string & filename, const bool & threaded = true);
};

/** An accessory asset for shaders that stores code blocks for other shaders to use.
@brief	no functionality on its own, but can recursively import more code blocks for other shaders and itself. */
class Shader_Pkg : public Asset {
public:
	// Public (de)Constructors
	/** Destroy the Shader Package. */
	~Shader_Pkg() = default;
	/** Construct the Shader Package. */
	Shader_Pkg(Engine * engine, const std::string & filename);

	
	// Public Methods
	/** Retrieves this package's content as a std::string.
	* @return	package contents */
	inline std::string getPackageText() const {
		return m_packageText;
	}

	
	// Public Attributes
	std::string m_packageText = "";


private:
	// Private Interface Implementation
	virtual void initialize() override;


	// Private Attributes
	friend class Shared_Shader_Pkg;
};

#endif // SHADER_PKG_H
