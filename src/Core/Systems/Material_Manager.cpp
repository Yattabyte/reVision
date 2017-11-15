#include "Systems\Material_Manager.h"

static deque<int> m_freed_material_spots;
static Material_Buffer material_buffer;
static GLuint buffer_ssbo = 0;

namespace Material_Manager {
	void startup()
	{
		buffer_ssbo = 0;
		glGenBuffers(1, &buffer_ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer_ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Material_Buffer), &material_buffer, GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer_ssbo);
		GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
		memcpy(p, &material_buffer, sizeof(Material_Buffer));
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
	void shutdown()
	{
		glDeleteBuffers(1, &buffer_ssbo);
		m_freed_material_spots.clear();
		material_buffer = Material_Buffer();
		buffer_ssbo = 0;
	}
	deque<int>& getMatFreeSpots()
	{
		return m_freed_material_spots;
		m_freed_material_spots.clear();
	}
	GLuint& getBufferSSBO()
	{
		return buffer_ssbo;
	}
}