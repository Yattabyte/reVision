#include "Asset_Collider.h"
#include "Utilities\IO\Text_IO.h"
#include "Utilities\IO\Mesh_IO.h"
#include "Engine.h"


constexpr char* DIRECTORY_COLLIDER = "\\Models\\";

Asset_Collider::Asset_Collider(const std::string & filename) : Asset(filename) {}

Shared_Asset_Collider Asset_Collider::Create(Engine * engine, const std::string & filename, const bool & threaded)
{
	return engine->getAssetManager().createAsset<Asset_Collider>(
		filename,
		DIRECTORY_COLLIDER,
		"",
		&initialize,
		engine,
		threaded
	);
}

void Asset_Collider::initialize(Engine * engine, const std::string & relativePath)
{
	// Forward asset creation
	m_mesh = Asset_Mesh::Create(engine, relativePath, false);
	btConvexHullShape * shape = new btConvexHullShape();
	for each (const auto & vertex in m_mesh->m_geometry.vertices) 
		shape->addPoint(btVector3(vertex.x, vertex.y, vertex.z));	
	shape->recalcLocalAabb();
	m_shape = shape;

	Asset::finalize(engine);
}
