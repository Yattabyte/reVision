#include "Assets/Collider.h"
#include "Utilities/IO/Text_IO.h"
#include "Utilities/IO/Mesh_IO.h"
#include "Engine.h"


constexpr const char* DIRECTORY_COLLIDER = "\\Models\\";

Shared_Collider::Shared_Collider(Engine* engine, const std::string& filename, const bool& threaded) noexcept
{
	(*(std::shared_ptr<Collider>*)(this)) = std::dynamic_pointer_cast<Collider>(
		engine->getManager_Assets().shareAsset(
			typeid(Collider).name(),
			filename,
			[engine, filename]() { return std::make_shared<Collider>(engine, filename); },
			threaded
		));
}

Collider::Collider(Engine* engine, const std::string& filename) noexcept : Asset(engine, filename) {}

void Collider::initialize() noexcept
{
	// Forward asset creation
	m_mesh = Shared_Mesh(m_engine, DIRECTORY_COLLIDER + getFileName(), false);
	auto* shape = new btConvexHullShape();
	for (const auto& vertex : m_mesh->m_geometry.vertices)
		shape->addPoint(btVector3(vertex.x, vertex.y, vertex.z));
	shape->recalcLocalAabb();
	m_shape = shape;

	Asset::finalize();
}