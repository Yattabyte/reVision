#include "Modules/Graphics/Common/Graphics_Framebuffers.h"
#include "Engine.h"


Graphics_Framebuffers::~Graphics_Framebuffers()
{
	// Destroy OpenGL objects
	for (const auto[name, fboData] : m_fbos) {
		const auto[fboID, mipmapped, texdata] = fboData;
		glDeleteFramebuffers(1, &fboID);
		for (const auto[texID, internalFormat, format, type, attachment] : texdata)
			glDeleteTextures(1, &texID);
	}
}

Graphics_Framebuffers::Graphics_Framebuffers(const glm::ivec2 & size)
{	
	m_renderSize = size;
	createFBO("GEOMETRY", { { GL_RGB16F, GL_RGB, GL_FLOAT }, { GL_RGB16F, GL_RGB, GL_FLOAT }, { GL_RGBA16F, GL_RGBA, GL_FLOAT }, { GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8 } });
	createFBO("LIGHTING", { { GL_RGB16F, GL_RGB, GL_FLOAT } });
	createFBO("REFLECTION", { { GL_RGB16F, GL_RGB, GL_FLOAT } });
	createFBO("BOUNCE", { { GL_RGB16F, GL_RGB, GL_FLOAT } });
	createFBO("SSAO", { { GL_R8, GL_RED, GL_FLOAT }, { GL_R8, GL_RED, GL_FLOAT } });
	createFBO("SSR", { { GL_RGB8, GL_RGB, GL_FLOAT } });
	createFBO("SSR_MIP", { { GL_RGB8, GL_RGB, GL_FLOAT } }, true);
	createFBO("BLOOM", { { GL_RGB16F, GL_RGB, GL_FLOAT }, { GL_RGB16F, GL_RGB, GL_FLOAT } });
	createFBO("HDR", { { GL_RGB16F, GL_RGB, GL_FLOAT } });
	createFBO("FXAA", { { GL_RGB16F, GL_RGB, GL_FLOAT } });
}

void Graphics_Framebuffers::createFBO(const char * name, const std::vector<std::tuple<GLenum, GLenum, GLenum>> & textureFormats, const bool & mipmapped)
{
	//  Variables for this frame buffer entry
	GLuint fboID(0);	// FBO ID
	std::vector<std::tuple<
		GLuint,			// Texture ID
		GLenum,			// Internal Format
		GLenum,			// Format
		GLenum,			// Type
		GLenum			// Attachment
	>> textures;
	std::vector<GLenum> drawBuffers;

	// Create the framebuffer
	glCreateFramebuffers(1, &fboID);
	// Create all the textures
	int counter(0);
	for each (const auto & texture in textureFormats) {
		const auto[internalFormat, format, type] = texture;
		GLuint texID(0);
		glCreateTextures(GL_TEXTURE_2D, 1, &texID);
		glTextureParameteri(texID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(texID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		if (mipmapped) {
			glTextureParameteri(texID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTextureParameteri(texID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(texID, GL_TEXTURE_MIN_LOD, 0);
			glTextureParameteri(texID, GL_TEXTURE_MAX_LOD, 5);
			glTextureParameteri(texID, GL_TEXTURE_BASE_LEVEL, 0);
			glTextureParameteri(texID, GL_TEXTURE_MAX_LEVEL, 5);
			for (int x = 0; x < 6; ++x) {
				const glm::ivec2 size(floor(m_renderSize.x / pow(2, x)), floor(m_renderSize.y / pow(2, x)));
				glTextureImage2DEXT(texID, GL_TEXTURE_2D, x, internalFormat, size.x, size.y, 0, format, type, NULL);
			}
		}
		else {
			glTextureParameteri(texID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTextureParameteri(texID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureImage2DEXT(texID, GL_TEXTURE_2D, 0, internalFormat, m_renderSize.x, m_renderSize.y, 0, format, type, NULL);
		}
		GLenum attachment;
		if (format == GL_DEPTH_STENCIL)
			attachment = GL_DEPTH_STENCIL_ATTACHMENT;
		else if (format == GL_DEPTH)
			attachment = GL_DEPTH_ATTACHMENT;
		else if (format == GL_STENCIL)
			attachment = GL_STENCIL_ATTACHMENT;
		else {
			attachment = GL_COLOR_ATTACHMENT0 + counter++;
			drawBuffers.push_back(attachment);
		}
		glNamedFramebufferTexture(fboID, attachment, texID, 0);
		/*if (!glIsTexture(texID))
			m_engine->getManager_Messages().error("Graphics Framebuffer texture is incomplete.");*/
		textures.push_back({ texID, internalFormat, format, type, attachment });
	}
	glNamedFramebufferDrawBuffers(fboID, (GLsizei)drawBuffers.size(), &drawBuffers[0]);
	/*if (glCheckNamedFramebufferStatus(fboID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		m_engine->getManager_Messages().error("Graphics Framebuffer has encountered an error.");*/
	m_fbos[name] = { fboID, mipmapped, textures };
}

void Graphics_Framebuffers::bindForWriting(const char * name)
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, getFboID(name));
}

void Graphics_Framebuffers::bindForReading(const char * name, const GLuint & binding) 
{
	int counter(0);
	for (const auto[texID, internalFormat, format, type, attachment] : std::get<2>(m_fbos[name]))
		glBindTextureUnit(binding + counter++, texID);
}

void Graphics_Framebuffers::clear()
{
	GLfloat clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	GLfloat clearDepth = 1.0f;
	GLint clearStencil = 0;
	for (const auto[name, fboData] : m_fbos) {
		const auto[fboID, mipmapped, texdata] = fboData;
		int counter(0);
		for (const auto[texID, internalFormat, format, type, attachment] : texdata) {
			if (attachment == GL_DEPTH_STENCIL_ATTACHMENT)
				glClearNamedFramebufferfi(fboID, GL_DEPTH_STENCIL, 0, clearDepth, clearStencil);
			else if (attachment == GL_DEPTH_ATTACHMENT)
				glClearNamedFramebufferfv(fboID, GL_DEPTH, 0, &clearDepth);
			else if (attachment == GL_STENCIL_ATTACHMENT)
				glClearNamedFramebufferfv(fboID, GL_STENCIL, 0, &clearDepth);
			else
				glClearNamedFramebufferfv(fboID, GL_COLOR, counter++, clearColor);
		}
	}
}

void Graphics_Framebuffers::resize(const glm::ivec2 & newSize)
{
	m_renderSize = newSize;
	for (const auto[name, fboData] : m_fbos) {
		const auto[fboID, mipmapped, texdata] = fboData;
		int counter(0);
		for (const auto[texID, internalFormat, format, type, attachment] : texdata) {
			if (mipmapped) {
				for (int x = 0; x < 6; ++x) {
					const glm::ivec2 mippedSize(floor(m_renderSize.x / pow(2, x)), floor(m_renderSize.y / pow(2, x)));
					glTextureImage2DEXT(texID, GL_TEXTURE_2D, x, internalFormat, mippedSize.x, mippedSize.y, 0, format, type, NULL);
				}
			}
			else
				glTextureImage2DEXT(texID, GL_TEXTURE_2D, 0, internalFormat, m_renderSize.x, m_renderSize.y, 0, format, type, NULL);
			GLenum attachment;
			if (format == GL_DEPTH_STENCIL)
				attachment = GL_DEPTH_STENCIL_ATTACHMENT;
			else if (format == GL_DEPTH)
				attachment = GL_DEPTH_ATTACHMENT;
			else if (format == GL_STENCIL)
				attachment = GL_STENCIL_ATTACHMENT;
			else 
				attachment = GL_COLOR_ATTACHMENT0 + counter++;			
			glNamedFramebufferTexture(fboID, attachment, texID, 0);
		}
	}
}

GLuint Graphics_Framebuffers::getFboID(const char * name)
{
	return std::get<0>(m_fbos[name]);
}

GLuint Graphics_Framebuffers::getTexID(const char * name, const size_t & index)
{
	return std::get<0>(
		(std::get<2>(m_fbos[name]))[index] // inner '() brackets' is the vector
	);
}

