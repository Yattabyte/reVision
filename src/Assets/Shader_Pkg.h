#pragma once
#ifndef	SHADER_PKG_H
#define	SHADER_PKG_H

#include "Assets/Asset.h"
#include <glad/glad.h>
#include "glm/glm.hpp"
#include <string>


class Engine;
class Shader_Pkg;

/** Shared version of a Shader_Pkg asset.
Responsible for the creation, containing, and sharing of assets. */
class Shared_Shader_Pkg final : public std::shared_ptr<Shader_Pkg> {
public:
	// Public (de)Constructors
	/** Constructs an empty asset. */
	inline Shared_Shader_Pkg() = default;
	/** Begins the creation process for this asset.
	@param	engine			the engine being used.
	@param	filename		the filename to use.
	@param	threaded		create in a separate thread.
	@return					the desired asset. */
	explicit Shared_Shader_Pkg(Engine* engine, const std::string& filename, const bool& threaded = true);
};

/** An accessory asset for loading shader code chunks.
An accessory asset for shaders that stores code blocks for other shaders to use.
Provides no functionality on its own, but can recursively import more code blocks for other shaders and itself. */
class Shader_Pkg final : public Asset {
public:
	// Public (de)Constructors
	/** Destroy the Shader Package. */
	~Shader_Pkg() = default;
	/** Construct the Shader Package.
	@param	engine			the engine to use.
	@param	filename		the asset file name (relative to engine directory). */
	Shader_Pkg(Engine* engine, const std::string& filename);


	// Public Methods
	/** Retrieves this package's content as a std::string.
	* @return				package contents. */
	inline std::string getPackageText() const {
		return m_packageText;
	}


	// Public Attributes
	std::string m_packageText = "";


private:
	// Private Interface Implementation
	virtual void initialize() override final;


	// Private Attributes
	friend class Shared_Shader_Pkg;
};

#endif // SHADER_PKG_H
