#pragma once
#ifndef	IMAGE_H
#define	IMAGE_H

#include "Assets/Asset.h"
#include "Utilities/IO/Image_IO.h"
#include "glm/glm.hpp"
#include <any>
#include <optional>


class Engine;
class Image;

/** Shared version of an Image asset.
Responsible for the creation, containing, and sharing of assets. */
class Shared_Image final : public std::shared_ptr<Image> {
public:
	// Public (De)Constructors
	/** Constructs an empty asset. */
	inline Shared_Image() noexcept = default;
	/** Begins the creation process for this asset.
	@param	engine			the engine being used.
	@param	filename		the filename to use.
	@param	specificSize	an optional size to force the image to.
	@param	category		the category of image, if available.
	@param	threaded		create in a separate thread.
	@return					the desired asset. */
	explicit Shared_Image(Engine* engine, const std::string& filename, const std::optional<glm::ivec2>& specificSize, const bool& threaded = true, const Fill_Policy& policyFill = Fill_Policy::CHECKERED, const Resize_Policy& policyResize = Resize_Policy::LINEAR) noexcept;
};

/** Contains image data and related attributes.
Responsible for fetching and processing an image from disk and optionally resizing it. */
class Image final : public Asset {
public:
	// Public (De)Constructors
	/** Destroy the Image. */
	~Image() noexcept;
	/** Construct the Image.
	@param	engine			the engine to use.
	@param	filename		the asset file name (relative to engine directory).
	@param	specificSize	an optional size to force the image to.
	@param	policyFill		the pixel fill policy.
	@param	policyResize	the image resize policy. */
	Image(Engine* engine, const std::string& filename, const std::optional<glm::ivec2>& specificSize, const Fill_Policy& policyFill, const Resize_Policy& policyResize) noexcept;


	// Public Attributes
	glm::ivec2 m_size = glm::ivec2(0);
	GLubyte* m_pixelData = nullptr;
	GLint m_pitch = 0;
	GLuint m_bpp = 0;
	Fill_Policy m_policyFill = Fill_Policy::CHECKERED;
	Resize_Policy m_policyResize = Resize_Policy::LINEAR;


private:
	// Private Methods
	/** Fill the image with the desired colors, in accordance with the fill policy.
	@param	primaryColor	the primary color to use.
	@param	secondaryColor	the secondary color to use. */
	void fill(const glm::uvec4 primaryColor = glm::uvec4(128, 128, 255, 255), const glm::uvec4 secondaryColor = glm::uvec4(0, 0, 0, 255)) noexcept;
	/** Resize the image.
	@param	newSize			the new size to use. */
	void resize(const glm::ivec2 newSize) noexcept;


	// Private Interface Implementation
	virtual void initialize() noexcept override final;


	// Private Attributes
	friend class Shared_Image;
};

#endif // IMAGE_H
