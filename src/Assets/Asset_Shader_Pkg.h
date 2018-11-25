#pragma once
#ifndef	ASSET_SHADER_PKG_H
#define	ASSET_SHADER_PKG_H

#include "Assets\Asset.h"
#include "glm\glm.hpp"
#include "GL\glad\glad.h"
#include <string>


class Engine;
class Asset_Shader_Pkg;
using Shared_Asset_Shader_Pkg = std::shared_ptr<Asset_Shader_Pkg>;

/** An accessory asset for shaders that stores code blocks for other shaders to use.
@brief	no functionality on its own, but can recursively import more code blocks for other shaders and itself. */
class Asset_Shader_Pkg : public Asset
{
public:
	/** Destroy the Shader Package. */
	~Asset_Shader_Pkg() = default;
	/** Construct the Shader Package. */
	Asset_Shader_Pkg(const std::string & filename);

	
	// Public Methods
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	threaded		create in a separate thread
	@return					the desired asset */
	static Shared_Asset_Shader_Pkg Create(Engine * engine, const std::string & filename, const bool & threaded = true);
	/** Retrieves this package's content as a std::string.
	* @return	package contents */
	inline const std::string getPackageText() const {
		return m_packageText;
	}

	
	// Public Attributes
	std::string m_packageText = "";


private:
	// Private Methods
	// Interface Implementation
	virtual void initialize(Engine * engine, const std::string & relativePath) override;


	// Private Attributes
	friend class AssetManager;
};

#endif // ASSET_SHADER_PKG_H
