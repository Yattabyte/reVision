#pragma once
#ifndef	SHADER_GEOMETRY_H
#define	SHADER_GEOMETRY_H

#include "Assets/Asset.h"
#include "Assets/Shader.h"


// Forward Declarations
class Shader_Geometry;

/** Shared version of a Shader_Geometry asset.
Responsible for the creation, containing, and sharing of assets. */
class Shared_Shader_Geometry final : public std::shared_ptr<Shader_Geometry> {
public:
	// Public (De)Constructors
	/** Constructs an empty asset. */
	inline Shared_Shader_Geometry() noexcept = default;
	/** Begins the creation process for this asset.
	@param	engine			reference to the engine to use. 
	@param	filename		the filename to use.
	@param	threaded		create in a separate thread.
	@return					the desired asset. */
	Shared_Shader_Geometry(Engine& engine, const std::string& filename, const bool& threaded = true) noexcept;
};

/** An entire OpenGL vertex/geometry/fragment shader program.
An encapsulation of an OpenGL vertex & geometry & fragment shader program, extending the Shader asset.
Responsible for loading the files associated with this program from disk, and forming the program.
Also provides support for explicitly setting uniform values for a given attribute location.
Supports binary representation. */
class Shader_Geometry : public Shader {
public:
	// Public (De)Constructors
	/** Destroy the Shader. */
	~Shader_Geometry() noexcept;
	/** Construct the Shader.
	@param	engine			reference to the engine to use.
	@param	filename		the asset file name (relative to engine directory). */
	Shader_Geometry(Engine& engine, const std::string& filename) noexcept;


	// Public Attributes
	ShaderObj m_geometryShader = ShaderObj(GL_GEOMETRY_SHADER);


protected:
	// Interface Implementation
	virtual bool initShaders(const std::string& relativePath) noexcept override final;


private:
	// Private Interface Implementation
	virtual void initialize() noexcept override final;


	// Private Attributes
	friend class Shared_Shader_Geometry;
};

#endif // SHADER_GEOMETRY_H