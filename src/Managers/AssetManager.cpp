#include "Managers\AssetManager.h"

/** Begin Asset Includes **/
#include "Assets/Asset.h"
#include "Assets/Asset_Collider.h"
#include "Assets/Asset_Config.h"
#include "Assets/Asset_Cubemap.h"
#include "Assets/Asset_Material.h"
#include "Assets/Asset_Model.h"
#include "Assets/Asset_Primitive.h"
#include "Assets/Asset_Shader.h"
#include "Assets/Asset_Shader_Pkg.h"
#include "Assets/Asset_Texture.h"
/** End Asset Includes **/

 
AssetManager::~AssetManager()
{
	/** @todo destructor */
}

AssetManager::AssetManager()
{
	registerAssetCreator(typeid(Shared_Asset_Collider).name(), function<void(AssetManager&, Shared_Asset_Collider&, const string&, const bool &)>(Asset_Collider::Create));
	registerAssetCreator(typeid(Shared_Asset_Config).name(), function<void(AssetManager&, Shared_Asset_Config&, const string&, const vector<string> &, const bool &)>(Asset_Config::Create));
	registerAssetCreator(typeid(Shared_Asset_Cubemap).name(), function<void(AssetManager&, Shared_Asset_Cubemap&, const string&, const bool &&)>(Asset_Cubemap::Create));
	registerAssetCreator(typeid(Shared_Asset_Material).name(), function<void(AssetManager&, Shared_Asset_Material&, MaterialManager&, const string&, const bool &, const string(&)[MAX_PHYSICAL_IMAGES])>(Asset_Material::Create));
	registerAssetCreator(typeid(Shared_Asset_Model).name(), function<void(AssetManager&, Shared_Asset_Model&, ModelManager&, MaterialManager&, const string&, const bool &)>(Asset_Model::Create));
	registerAssetCreator(typeid(Shared_Asset_Primitive).name(), function<void(AssetManager&, Shared_Asset_Primitive&, const string&, const bool &)>(Asset_Primitive::Create));
	registerAssetCreator(typeid(Shared_Asset_Shader).name(), function<void(AssetManager&, Shared_Asset_Shader&, const string&, const bool &)>(Asset_Shader::Create));
	registerAssetCreator(typeid(Shared_Asset_Shader_Pkg).name(), function<void(AssetManager&, Shared_Asset_Shader_Pkg&, const string&, const bool &)>(Asset_Shader_Pkg::Create));
	registerAssetCreator(typeid(Shared_Asset_Texture).name(), function<void(AssetManager&, Shared_Asset_Texture&, const string&, const GLuint &, const bool &, const bool &, const bool &)>(Asset_Texture::Create));
	

	m_Workers.reserve(4);
	for (unsigned int x = 0; x < 4; ++x) {
		m_Workers.push_back(std::make_shared<Asset_Worker>());
		auto &worker = m_Workers[x];
		unique_lock<shared_mutex> writeGuard(worker->m_mutex);
		worker->m_thread = new thread(&AssetManager::initializeOrders, this, worker);
	}
}

void AssetManager::initializeOrders(shared_ptr<Asset_Worker> & worker)
{
	bool stay_alive = true;
	while (stay_alive) {
		// Start reading from Asset Manager
		unique_lock<shared_mutex> worker_writeGuard(m_Mutex_Workorders);
		WorkOrder_Base *workOrder = nullptr;
		if (m_Work_toStart.size()) {
			workOrder = m_Work_toStart.front();
			m_Work_toStart.pop_front();
		}
		worker_writeGuard.unlock();
		worker_writeGuard.release();
		if (workOrder != nullptr) {
			workOrder->start();

			unique_lock<shared_mutex> new_manager_writeGuard(m_Mutex_Workorders);
			m_Work_toFinish.push_back(workOrder);
		}

		// Check if worker should shutdonw
		shared_lock<shared_mutex> worker_readGuard(worker->m_mutex);
		stay_alive = worker->m_alive;
	}
}
void AssetManager::finalizeOrders() 
{
	unique_lock<shared_mutex> worker_writeGuard(m_Mutex_Workorders);
	if (!m_Work_toFinish.size())
		return;
	auto * workOrder = m_Work_toFinish.front();
	m_Work_toFinish.pop_front();
	workOrder->finish();
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

