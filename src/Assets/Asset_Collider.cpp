#include "Asset_Collider.h"
#include "Utilities\IO\Text_IO.h"
#include "Utilities\IO\Mesh_IO.h"
#include "Engine.h"


constexpr char* DIRECTORY_COLLIDER = "\\Models\\";

Shared_Collider::Shared_Collider(Engine * engine, const std::string & filename, const bool & threaded)
	: std::shared_ptr<Asset_Collider>(engine->getAssetManager().createAsset<Asset_Collider>(
		filename,
		DIRECTORY_COLLIDER,
		"",
		engine,
		threaded
	)) {}

Asset_Collider::Asset_Collider(const std::string & filename) : Asset(filename) {}

void Asset_Collider::initialize(Engine * engine, const std::string & relativePath)
{
	// Forward asset creation
	m_mesh = Shared_Mesh(engine, relativePath, false);
	btConvexHullShape * shape = new btConvexHullShape();
	for each (const auto & vertex in m_mesh->m_geometry.vertices) 
		shape->addPoint(btVector3(vertex.x, vertex.y, vertex.z));	
	shape->recalcLocalAabb();
	m_shape = shape;

	Asset::finalize(engine);
}