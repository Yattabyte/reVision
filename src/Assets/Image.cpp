#include "Assets/Image.h"
#include "Engine.h"
#include "glm/gtc/type_ptr.hpp"


Shared_Image::Shared_Image(Engine& engine, const std::string& filename, const std::optional<glm::ivec2>& specificSize, const bool& threaded, const Fill_Policy& policyFill, const Resize_Policy& policyResize) noexcept
{
	swap(std::dynamic_pointer_cast<Image>(engine.getManager_Assets().shareAsset(
			typeid(Image).name(),
			filename,
			[&engine, filename, specificSize, policyFill, policyResize]() { return std::make_shared<Image>(engine, filename, specificSize, policyFill, policyResize); },
			threaded
		)));
}

Image::Image(Engine& engine, const std::string& filename, const std::optional<glm::ivec2>& specificSize, const Fill_Policy& policyFill, const Resize_Policy& policyResize) noexcept : Asset(engine, filename), m_policyFill(policyFill), m_policyResize(policyResize)
{
	if (specificSize)
		m_size = specificSize.value();
}

void Image::initialize() noexcept
{
	Image_Data dataContainer{ m_pixelData, m_size, m_pitch, m_bpp };
	if (Image_IO::Import_Image(m_engine, getFileName(), dataContainer, m_policyResize)) {
		m_size = dataContainer.dimensions;
		m_pixelData = dataContainer.pixelData;
		m_pitch = dataContainer.pitch;
		m_bpp = dataContainer.bpp;
	}
	else
		fill();

	Asset::finalize();
}

void Image::fill(const glm::uvec4 primaryColor, const glm::uvec4 secondaryColor) noexcept
{
	if (m_size == glm::ivec2(0))
		m_size = glm::ivec2(256);
	const size_t pixelCount = size_t(m_size.x) * size_t(m_size.x);
	const size_t componentCount = pixelCount * 4;
	m_pixelData.resize(componentCount);
	m_pitch = m_size.x * 4;
	m_bpp = 32;
	switch (m_policyFill) {
	case Fill_Policy::SOLID: {
		size_t pxComponent = 0;
		std::generate(m_pixelData.begin(), m_pixelData.begin() + componentCount, [&]()->GLubyte {
			const unsigned int ID = (pxComponent++) % 4;
			const auto component = GLubyte(primaryColor[ID]);
			return component;
			});
		break;
	}
	case Fill_Policy::CHECKERED: {
		// How many pixels wide and tall the checkers should be
		constexpr size_t checkerSize = 32;
		const size_t rowWidth = m_size.x;
		const GLubyte colors[2][4] = {
			{ GLubyte(primaryColor.x), GLubyte(primaryColor.y), GLubyte(primaryColor.z), GLubyte(primaryColor.w) },
			{ GLubyte(secondaryColor.x), GLubyte(secondaryColor.y), GLubyte(secondaryColor.z), GLubyte(secondaryColor.w) }
		};
		bool cFlip = false;
		for (size_t pixel = 0, depth = size_t(-1); pixel < pixelCount; ++pixel) {
			// Flip color if we've drawn enough of the checker
			if ((pixel % checkerSize) == 0)
				cFlip = !cFlip;
			// Increment depth when we finish a row
			if ((pixel % rowWidth) == 0) {
				depth++;
				if (depth % checkerSize == 0)
					cFlip = !cFlip;
			}
			// Copy color info into pixel spot
			std::copy(&colors[cFlip][0], &colors[cFlip][4], &m_pixelData[pixel * 4]);
		}
		break;
	}
	};
}

void Image::resize(const glm::ivec2 newSize) noexcept
{
	Image_Data dataContainer{ m_pixelData, m_size, m_pitch, m_bpp };
	Image_IO::Resize_Image(newSize, dataContainer, m_policyResize);
	m_size = dataContainer.dimensions;
	m_pixelData = dataContainer.pixelData;
	m_pitch = dataContainer.pitch;
	m_bpp = dataContainer.bpp;
}