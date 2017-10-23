#include "Assets\Asset_Model.h"

/* -----ASSET TYPE----- */
#define ASSET_TYPE 3 

using namespace Asset_Manager;

Asset_Model::~Asset_Model()
{
	if (finalized)
		glDeleteVertexArrays(1, &gl_vao_ID);
}

Asset_Model::Asset_Model()
{
	gl_vao_ID = 0;
	mesh_size = 0;
	filename = "";
	bbox_min = vec3(0.0f);
	bbox_max = vec3(0.0f);
	finalized = false;
}

Asset_Model::Asset_Model(const string & _filename) : Asset_Model()
{
	filename = _filename;
}

void Asset_Model::Finalize()
{
	shared_lock<shared_mutex> read_guard(m_mutex);
	if (!finalized) {
		read_guard.unlock();
		read_guard.release();
		unique_lock<shared_mutex> write_guard(m_mutex);
		GLuint buffers[7];
		const size_t &arraySize = data.vs.size();
		glGenVertexArrays(1, &gl_vao_ID);
		glBindVertexArray(gl_vao_ID);
		glGenBuffers(7, buffers);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);
		glEnableVertexAttribArray(7);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
		glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(vec3), &data.vs[0][0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
		glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(vec3), &data.nm[0][0], GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
		glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(vec3), &data.tg[0][0], GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[3]);
		glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(vec3), &data.bt[0][0], GL_STATIC_DRAW);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[4]);
		glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(vec2), &data.uv[0][0], GL_STATIC_DRAW);
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[5]);
		glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(GLuint), &data.ts[0], GL_STATIC_DRAW);
		glVertexAttribIPointer(5, 1, GL_UNSIGNED_INT, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[6]);
		glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(VertexBoneData), &data.bones[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(6);
		glVertexAttribIPointer(6, 4, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid*)16);

		glBindVertexArray(0);
		glDeleteBuffers(7, buffers);
		finalized = true;
	}
}

int Asset_Model::GetAssetType()
{
	return ASSET_TYPE;
}