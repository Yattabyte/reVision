#include "Assets\Asset_Primitive.h"

/* -----ASSET TYPE----- */
#define ASSET_TYPE 2

Asset_Primitive::~Asset_Primitive()
{
	if (finalized)
		glDeleteVertexArrays(1, &ID);
}

Asset_Primitive::Asset_Primitive()
{
	ID = 0;
	filename = "";
	finalized = false;
}

Asset_Primitive::Asset_Primitive(const string & _filename) : Asset_Primitive()
{
	filename = _filename;
}

int Asset_Primitive::GetAssetType()
{
	return ASSET_TYPE;
}

void Asset_Primitive::Finalize()
{
	shared_lock<shared_mutex> read_guard(m_mutex);
	if (!finalized) {
		read_guard.unlock();
		read_guard.release();
		unique_lock<shared_mutex> write_guard(m_mutex);
		GLuint buffers[2];
		const size_t arraySize = data.size();

		glGenVertexArrays(1, &ID);
		glBindVertexArray(ID);
		glGenBuffers(2, buffers);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
		glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(vec3), &data[0][0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
		glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(vec2), &uv_data[0][0], GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glBindVertexArray(0);
		glDeleteBuffers(2, buffers);
		finalized = true;
	}
}

void Asset_Primitive::Bind()
{
	glBindVertexArray(ID);
}

void Asset_Primitive::Unbind()
{
	glBindVertexArray(0);
}

void Asset_Primitive::Draw()
{
	glBindVertexArray(ID);
	glDrawArrays(GL_TRIANGLES, 0, (int)data.size());
	glBindVertexArray(0);
}

size_t Asset_Primitive::GetSize() const
{
	return data.size();
}