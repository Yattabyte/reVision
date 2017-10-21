#include "Managers\Material_Manager.h"

static deque<int> m_freed_material_spots;
static Material_Manager::Material_Buffer material_buffer;
static GLuint buffer_ssbo = 0;

namespace Material_Manager {
	void startup()
	{
		buffer_ssbo = 0;
		glGenBuffers(1, &buffer_ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer_ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(material_buffer), &material_buffer, GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer_ssbo);
		GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
		memcpy(p, &material_buffer, sizeof(material_buffer));
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
	deque<int>& getMatFreeSpots()
	{
		return m_freed_material_spots;
	}
	GLuint& getBufferSSBO()
	{
		return buffer_ssbo;
	}
}