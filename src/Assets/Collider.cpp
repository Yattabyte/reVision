#include "Assets/Collider.h"
#include "Utilities/IO/Text_IO.h"
#include "Utilities/IO/Mesh_IO.h"
#include "Engine.h"


constexpr char* DIRECTORY_COLLIDER = "\\Models\\";

Shared_Collider::Shared_Collider(Engine * engine, const std::string & filename, const bool & threaded)
{
	(*(std::shared_ptr<Collider>*)(this)) = std::dynamic_pointer_cast<Collider>(
		engine->getManager_Assets().shareAsset(
			typeid(Collider).name(),
			filename,
			[engine, filename]() { return std::make_shared<Collider>(engine, filename); },
			threaded
		));
}

Collider::Collider(Engine * engine, const std::string & filename) : Asset(engine, filename) {}

void Collider::initialize()
{
	// Forward asset creation
	m_mesh = Shared_Mesh(m_engine, DIRECTORY_COLLIDER + getFileName(), false);
	btConvexHullShape * shape = new btConvexHullShape();
	for each (const auto & vertex in m_mesh->m_geometry.vertices) 
		shape->addPoint(btVector3(vertex.x, vertex.y, vertex.z));	
	shape->recalcLocalAabb();
	m_shape = shape;

	Asset::finalize();
}