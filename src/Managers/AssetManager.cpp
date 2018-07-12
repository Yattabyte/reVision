#include "Managers\AssetManager.h"
#include <minmax.h>

/** Begin Asset Includes **/
#include "Assets/Asset.h"
#include "Assets/Asset_Collider.h"
#include "Assets/Asset_Config.h"
#include "Assets/Asset_Cubemap.h"
#include "Assets/Asset_Material.h"
#include "Assets/Asset_Model.h"
#include "Assets/Asset_Primitive.h"
#include "Assets/Asset_Shader.h"
#include "Assets\Asset_Shader_Geometry.h"
#include "Assets/Asset_Shader_Pkg.h"
#include "Assets/Asset_Texture.h"
/** End Asset Includes **/

 
AssetManager::~AssetManager()
{
	/** @todo destructor */

	{
		unique_lock<shared_mutex> worker_writeGuard(m_workerNotificationMutex);
		m_running = false;
	}
	for each (auto thread in m_Workers) {
		if (thread->joinable())
			thread->join();
		delete thread;
	}
}

AssetManager::AssetManager(Engine * engine) : m_engine(engine)
{
	// Default Parameters
	m_running = true;

	// Register asset creators
	registerAssetCreator(typeid(Shared_Asset_Collider).name(), function<void(Engine*, Shared_Asset_Collider&, const string&, const bool &)>(Asset_Collider::Create));
	registerAssetCreator(typeid(Shared_Asset_Config).name(), function<void(Engine*, Shared_Asset_Config&, const string&, const vector<string> &, const bool &)>(Asset_Config::Create));
	registerAssetCreator(typeid(Shared_Asset_Cubemap).name(), function<void(Engine*, Shared_Asset_Cubemap&, const string&, const bool &&)>(Asset_Cubemap::Create));
	registerAssetCreator(typeid(Shared_Asset_Material).name(), function<void(Engine*, Shared_Asset_Material&, const string&, const bool &, const string(&)[MAX_PHYSICAL_IMAGES])>(Asset_Material::Create));
	registerAssetCreator(typeid(Shared_Asset_Model).name(), function<void(Engine*, Shared_Asset_Model&, const string&, const bool &)>(Asset_Model::Create));
	registerAssetCreator(typeid(Shared_Asset_Primitive).name(), function<void(Engine*, Shared_Asset_Primitive&, const string&, const bool &)>(Asset_Primitive::Create));
	registerAssetCreator(typeid(Shared_Asset_Shader).name(), function<void(Engine*, Shared_Asset_Shader&, const string&, const bool &)>(Asset_Shader::Create));
	registerAssetCreator(typeid(Shared_Asset_Shader_Geometry).name(), function<void(Engine*, Shared_Asset_Shader_Geometry&, const string&, const bool &)>(Asset_Shader_Geometry::Create));
	registerAssetCreator(typeid(Shared_Asset_Shader_Pkg).name(), function<void(Engine*, Shared_Asset_Shader_Pkg&, const string&, const bool &)>(Asset_Shader_Pkg::Create));
	registerAssetCreator(typeid(Shared_Asset_Texture).name(), function<void(Engine*, Shared_Asset_Texture&, const string&, const GLuint &, const bool &, const bool &, const bool &)>(Asset_Texture::Create));
	
	// Initialize worker threads
	const unsigned int maxThreads = std::min(4u, std::min(ASSETMANAGER_MAX_THREADS, thread::hardware_concurrency()));
	for (unsigned int x = 0; x < maxThreads; ++x) {
		std::thread * workerThread = new std::thread(&AssetManager::initializeOrders, this);
		workerThread->detach();
		m_Workers.push_back(workerThread);
	}
}

void AssetManager::initializeOrders()
{
	while (true) {
		// Check if worker should shutdown
		shared_lock<shared_mutex> worker_readGuard(m_workerNotificationMutex);
		if (!m_running)
			return;

		// Start reading work orders
		unique_lock<shared_mutex> worker_writeGuard(m_Mutex_Workorders);
		if (m_Work_toStart.size()) {
			// Remove front of queue
			Asset_Work_Order * workOrder = m_Work_toStart.front();
			m_Work_toStart.pop_front();
			worker_writeGuard.unlock();
			worker_writeGuard.release();

			// Initialize asset
			workOrder->start();

			// Add to finalization queue
			unique_lock<shared_mutex> new_manager_writeGuard(m_Mutex_Workorders);
			m_Work_toFinish.push_back(workOrder);
		}
	}
}
void AssetManager::finalizeOrders() 
{
	unique_lock<shared_mutex> worker_writeGuard(m_Mutex_Workorders);

	// Quit early
	if (!m_Work_toFinish.size())
		return;

	// Remove front of queue
	Asset_Work_Order * workOrder = m_Work_toFinish.front();
	m_Work_toFinish.pop_front();
	worker_writeGuard.unlock();
	worker_writeGuard.release();

	// Finalize asset
	workOrder->finish();

	// Delete work order
	delete workOrder;	
}

void AssetManager::submitNotifyee(const function<void()> & callBack)
{
	m_notifyees.push_back(callBack);
}

void AssetManager::notifyObservers()
{
	for each (const auto & notifyee in m_notifyees)
		notifyee();
	m_notifyees.clear();
}

