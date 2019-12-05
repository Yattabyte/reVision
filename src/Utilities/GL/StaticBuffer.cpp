#include "Utilities/GL/StaticBuffer.h"


StaticBuffer::~StaticBuffer() noexcept
{
	if (m_bufferID != 0)
		glDeleteBuffers(1, &m_bufferID);
}

StaticBuffer::StaticBuffer(const GLsizeiptr& size, const void* data, const GLbitfield& storageFlags) noexcept : 
	m_size(size), 
	m_storageFlags(storageFlags)
{
	glCreateBuffers(1, &m_bufferID);
	glNamedBufferStorage(m_bufferID, size, data, storageFlags);
}

StaticBuffer::StaticBuffer(const StaticBuffer& other) noexcept :
	StaticBuffer(other.m_size, 0, other.m_storageFlags) 
{
	glCopyNamedBufferSubData(other.m_bufferID, m_bufferID, 0, 0, other.m_size);
}

StaticBuffer::StaticBuffer(StaticBuffer&& other) noexcept 
{
	(*this) = std::move(other);
}

StaticBuffer& StaticBuffer::operator=(StaticBuffer&& other) noexcept
{
	if (this != &other) {
		m_bufferID = other.m_bufferID;
		other.m_bufferID = 0;
	}
	return *this;
}

void StaticBuffer::bindBuffer(const GLenum& target) const noexcept
{
	glBindBuffer(target, m_bufferID);
}

void StaticBuffer::bindBufferBase(const GLenum& target, const GLuint& index) const noexcept
{
	glBindBufferBase(target, index, m_bufferID);
}

void StaticBuffer::write(const GLsizeiptr& offset, const GLsizeiptr& size, const void* data) noexcept 
{
	glNamedBufferSubData(m_bufferID, offset, size, data);
}
