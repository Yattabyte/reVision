#include "Prop.h"
#include "Managers\Geometry_Manager.h"

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
	shared_lock<shared_mutex> other_read_guard(other.getDataMutex());
	worldState = other.worldState;
	assetModel = other.assetModel;
	Update();
}

void Prop::operator=(const Prop & other)
{
	shared_lock<shared_mutex> other_read_guard(other.getDataMutex());
	worldState = other.worldState;
	assetModel = other.assetModel;
	Update();
}

void Prop::registerSelf()
{
	Geometry_Manager::RegisterGeometry(GetGeometryType(), this);
}

void Prop::unregisterSelf()
{
	Geometry_Manager::UnRegisterGeometry(GetGeometryType(), this);
}

void Prop::Update()
{
	lock_guard<shared_mutex> write_guard(data_mutex);
	worldState.Update();
	uboData.mMatrix = worldState.modelMatrix;
	glBindBufferBase(GL_UNIFORM_BUFFER, 5, uboID);
	glBindBuffer(GL_UNIFORM_BUFFER, uboID);
	glBufferSubData(GL_UNIFORM_BUFFER, 16, sizeof(mat4x4), &worldState.modelMatrix);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

bool Prop::shouldRender(const mat4 & PVMatrix)
{
	shared_lock<shared_mutex> read_guard(data_mutex);
	shared_lock<shared_mutex> model_read_guard(assetModel->m_mutex);
	Frustum frustum(PVMatrix * worldState.modelMatrix);

	if (frustum.AABBInFrustom(assetModel->bbox_min, assetModel->bbox_max))
		return true;

	return false;
}

void Prop::geometryPass() const
{
	lock_guard<shared_mutex> read_guard(data_mutex);
	shared_lock<shared_mutex> guard(assetModel->m_mutex);

	glBindBufferBase(GL_UNIFORM_BUFFER, 5, uboID);

	glBindVertexArray(assetModel->gl_vao_ID);
	glDrawArrays(GL_TRIANGLES, 0, assetModel->mesh_size);
	glBindVertexArray(0);
}
