#include "Modules/Graphics/Common/Graphics_Framebuffers.h"
#include "Engine.h"


Graphics_Framebuffers::~Graphics_Framebuffers()
{
	// Destroy OpenGL objects
	for (const auto[name, fboData] : m_fbos) {
		glDeleteFramebuffers(1, &fboData.first);
		for (const auto[texID, internalFormat, format, type, attachment] : fboData.second) 
			glDeleteTextures(1, &texID);
	}
}

Graphics_Framebuffers::Graphics_Framebuffers(const glm::ivec2 & size)
{	
	m_renderSize = size;
}

void Graphics_Framebuffers::createFBO(const char * name, const std::vector<std::tuple<GLenum, GLenum, GLenum>> textureFormats)
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
		glTextureImage2DEXT(texID, GL_TEXTURE_2D, 0, internalFormat, m_renderSize.x, m_renderSize.y, 0, format, type, NULL);
		glTextureParameteri(texID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(texID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(texID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(texID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
	glNamedFramebufferDrawBuffers(fboID, drawBuffers.size(), &drawBuffers[0]);
	/*if (glCheckNamedFramebufferStatus(fboID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		m_engine->getManager_Messages().error("Graphics Framebuffer has encountered an error.");*/
	m_fbos[name] = { fboID, textures };
}

void Graphics_Framebuffers::bindForWriting(const char * name)
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, getFboID(name));
}

void Graphics_Framebuffers::bindForReading(const char * name, const GLuint & binding) 
{
	int counter(0);
	for (const auto[texID, internalFormat, format, type, attachment] : m_fbos[name].second)
		glBindTextureUnit(binding + counter++, texID);
}

void Graphics_Framebuffers::clear()
{
	GLfloat clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	GLfloat clearDepth = 1.0f;
	GLint clearStencil = 0;
	for (const auto[name, fboData] : m_fbos) {
		int counter(0);
		for (const auto[texID, internalFormat, format, type, attachment] : fboData.second) {
			if (attachment == GL_DEPTH_STENCIL_ATTACHMENT)
				glClearNamedFramebufferfi(fboData.first, GL_DEPTH_STENCIL, 0, clearDepth, clearStencil);
			else if (attachment == GL_DEPTH_ATTACHMENT)
				glClearNamedFramebufferfv(fboData.first, GL_DEPTH, 0, &clearDepth);
			else if (attachment == GL_STENCIL_ATTACHMENT)
				glClearNamedFramebufferfv(fboData.first, GL_STENCIL, 0, &clearDepth);
			else
				glClearNamedFramebufferfv(fboData.first, GL_COLOR, counter++, clearColor);
		}
	}
}

void Graphics_Framebuffers::resize(const glm::ivec2 & newSize)
{
	m_renderSize = newSize;
	for (const auto[name, fboData] : m_fbos)
		for (const auto[texID, internalFormat, format, type, attachment] : fboData.second)
			glTextureImage2DEXT(texID, GL_TEXTURE_2D, 0, internalFormat, m_renderSize.x, m_renderSize.y, 0, format, type, NULL);
}

GLuint Graphics_Framebuffers::getFboID(const char * name)
{
	return m_fbos[name].first;
}

GLuint Graphics_Framebuffers::getTexID(const char * name, const size_t & index)
{
	return std::get<0>(m_fbos[name].second[index]);
}

