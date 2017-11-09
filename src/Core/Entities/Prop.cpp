#include "Prop.h"
#include "Managers\Geometry_Manager.h"

#define GEOMETRY_TYPE 0

Prop::~Prop()
{
	glDeleteBuffers(1, &uboID);
}

Prop::Prop()
{
	glGenBuffers(1, &uboID);
	glBindBuffer(GL_UNIFORM_BUFFER, uboID);	
	glBufferData(GL_UNIFORM_BUFFER, (sizeof(mat4) * 101) + sizeof(float), &uboData, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_UNIFORM_BUFFER, 5, uboID);
	GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	memcpy(p, &uboData, sizeof(Transform_Buffer));
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

Prop::Prop(const string & relativePath) : Prop()
{
	Asset_Manager::load_asset(assetModel, relativePath);
	Update();
}

Prop::Prop(const Prop & other) : Prop()
{
	worldState = other.worldState;
	assetModel = other.assetModel;
	Update();
}

void Prop::operator=(const Prop & other)
{
	worldState = other.worldState;
	assetModel = other.assetModel;
	Update();
}

void Prop::registerSelf()
{
	Geometry_Manager::registerGeometry(Prop::GetGeometryType(), this);
}

void Prop::unregisterSelf()
{
	Geometry_Manager::unregisterGeometry(Prop::GetGeometryType(), this);
}

void Prop::Update()
{
	worldState.Update();
	uboData.mMatrix = worldState.modelMatrix;
	glBindBufferBase(GL_UNIFORM_BUFFER, 5, uboID);
	glBindBuffer(GL_UNIFORM_BUFFER, uboID);
	glBufferSubData(GL_UNIFORM_BUFFER, 16, sizeof(mat4x4), &worldState.modelMatrix);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

int Prop::GetGeometryType()
{
	return GEOMETRY_TYPE;
}

void Prop::geometryPass()
{
	shared_lock<shared_mutex> guard(assetModel->m_mutex);

	glBindBufferBase(GL_UNIFORM_BUFFER, 5, uboID);

	glBindVertexArray(assetModel->gl_vao_ID);
	glDrawArrays(GL_TRIANGLES, 0, assetModel->mesh_size);
	glBindVertexArray(0);

}
