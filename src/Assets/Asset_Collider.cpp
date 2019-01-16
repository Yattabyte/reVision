#include "Asset_Collider.h"
#include "Utilities\IO\Text_IO.h"
#include "Utilities\IO\Mesh_IO.h"
#include "Engine.h"


constexpr char* DIRECTORY_COLLIDER = "\\Models\\";

Shared_Collider::Shared_Collider(Engine * engine, const std::string & filename, const bool & threaded)
	: std::shared_ptr<Asset_Collider>(std::dynamic_pointer_cast<Asset_Collider>(engine->getManager_Assets().shareAsset(typeid(Asset_Collider).name(), filename)))
{
	// Find out if the asset needs to be created
	if (!get()) {
		// Create new asset on shared_ptr portion of this class 
		(*(std::shared_ptr<Asset_Collider>*)(this)) = std::make_shared<Asset_Collider>(filename);
		// Submit data to asset manager
		engine->getManager_Assets().submitNewAsset(typeid(Asset_Collider).name(), (*(std::shared_ptr<Asset>*)(this)), std::move(std::bind(&Asset_Collider::initialize, get(), engine, (DIRECTORY_COLLIDER + filename))), threaded);
	}
	// Check if we need to wait for initialization
	else
		if (!threaded)
			// Stay here until asset finalizes
			while (!get()->existsYet())
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

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