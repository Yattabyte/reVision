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
	Shared_Shader_Geometry(Engine& engine, const std::string& filename, const bool& threaded = true);
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
	~Shader_Geometry();
	/** Construct the Shader.
	@param	engine			reference to the engine to use.
	@param	filename		the asset file name (relative to engine directory). */
	Shader_Geometry(Engine& engine, const std::string& filename);


	// Public Attributes
	ShaderObj m_geometryShader = ShaderObj(GL_GEOMETRY_SHADER);


protected:
	// Interface Implementation
	bool initShaders(const std::string& relativePath) final;


private:
	// Private but deleted
	/** Disallow asset move constructor. */
	inline Shader_Geometry(Shader_Geometry&&) noexcept = delete;
	/** Disallow asset copy constructor. */
	inline Shader_Geometry(const Shader_Geometry&) noexcept = delete;
	/** Disallow asset move assignment. */
	inline const Shader_Geometry& operator =(Shader_Geometry&&) noexcept = delete;
	/** Disallow asset copy assignment. */
	inline const Shader_Geometry& operator =(const Shader_Geometry&) noexcept = delete;


	// Private Interface Implementation
	void initialize() final;


	// Private Attributes
	friend class Shared_Shader_Geometry;
};

#endif // SHADER_GEOMETRY_H