#pragma once
#ifndef	SHADER_PKG_H
#define	SHADER_PKG_H

#include "Assets/Asset.h"


// Forward Declarations
class Shader_Pkg;

/** Shared version of a Shader_Pkg asset.
Responsible for the creation, containing, and sharing of assets. */
class Shared_Shader_Pkg final : public std::shared_ptr<Shader_Pkg> {
public:
	// Public (De)Constructors
	/** Constructs an empty asset. */
	inline Shared_Shader_Pkg() = default;
	/** Begins the creation process for this asset.
	@param	engine			reference to the engine to use. 
	@param	filename		the filename to use.
	@param	threaded		create in a separate thread.
	@return					the desired asset. */
	Shared_Shader_Pkg(Engine& engine, const std::string& filename, const bool& threaded = true);
};

/** An accessory asset for loading shader code chunks.
An accessory asset for shaders that stores code blocks for other shaders to use.
Provides no functionality on its own, but can recursively import more code blocks for other shaders and itself. */
class Shader_Pkg final : public Asset {
public:
	// Public (De)Constructors
	/** Construct the Shader Package.
	@param	engine			reference to the engine to use. 
	@param	filename		the asset file name (relative to engine directory). */
	Shader_Pkg(Engine& engine, const std::string& filename);


	// Public Methods
	/** Retrieves this package's content as a std::string.
	@return				package contents. */
	std::string getPackageText() const;


	// Public Attributes
	std::string m_packageText = "";


private:
	// Private Interface Implementation
	void initialize() final;


	// Private Attributes
	friend class Shared_Shader_Pkg;
};

#endif // SHADER_PKG_H