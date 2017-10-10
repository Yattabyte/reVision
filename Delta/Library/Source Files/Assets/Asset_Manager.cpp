#include "Assets\Asset_Manager.h"
#include <direct.h>

static map<int, vector<Shared_Asset>>	assets;					// Assets stored in vectors coresponding to their asset-type
static map<int, Shared_Asset>			fallback_assets;		// 'Default' assets to fallback on if a particular asset can't be loaded. Mapped by asset-type
static deque<Shared_Asset>				asset_workorders;		// Assets that have initialized, but need to be finalized on the main thread.
static vector<pair<thread*, bool*>>		worker_threads;			// Pair of threads and their completion state, which is set to true when finalized.
static shared_mutex						mutex_IO_assets,		// Lock when the asset/fallback map are accessed
										mutex_IO_workorders,	// Lock when the work orders are accessed
										mutex_IO_workthreads;	// Lock when the worker threads are accessed

namespace Asset_Manager {	
	void shutdown()
	{		
		// Prevent asynchronous read/write on "worker_threads"
		unique_lock<shared_mutex> guard(mutex_IO_workthreads); 

		// Elements in here have finished their task and can be deleted
		for (int x = 0; x < worker_threads.size(); ++x) {
			pair<thread*, bool*> &pair = worker_threads[x];			
			if (pair.first->joinable()) pair.first->join(); // Try to join the thread
			delete pair.first;
			delete pair.second;
			worker_threads.erase(worker_threads.begin() + x);
			x--;
		}
	}
	void submitWorkorder(const Shared_Asset & w)
	{
		// Prevent asynchronous read/write on "asset_workorders"
		unique_lock<shared_mutex> guard(mutex_IO_workorders);
		asset_workorders.push_back(w);
	}
	void submitWorkthread(const pair<thread*, bool*>& thread)
	{
		// Prevent asynchronous read/write on "worker_threads"
		unique_lock<shared_mutex> guard(mutex_IO_workthreads);
		worker_threads.push_back(thread);
	}
	void ParseWorkOrders()
	{
		{
			// Try to read from front workorder -> finalize, lock mutex, remove from workorders
			shared_lock<shared_mutex> read_guard(mutex_IO_workorders);
			if (asset_workorders.size()) {
				asset_workorders.front()->Finalize();

				read_guard.unlock();
				read_guard.release();
				unique_lock<shared_mutex> write_guard(mutex_IO_workorders);
				asset_workorders.pop_front();
			}
		}

		// Same as shutdown(), delete all complete threads
		unique_lock<shared_mutex> guard(mutex_IO_workthreads);
		for (int x = 0; x < worker_threads.size(); ++x) {
			pair<thread*, bool*> &pair = worker_threads[x];
			if (*pair.second == true) {
				if (pair.first->joinable()) pair.first->join();
				delete pair.first;
				delete pair.second;
				worker_threads.erase(worker_threads.begin() + x);
				x--;
			}
		}
	}
	vector<Shared_Asset>& fetchAssetList(const int &asset_type)
	{
		// Returns the vector of assets in the asset map at the spot of asset_type.
		// First tries to insert a vector in the map with the key of asset_type.
		// Map disallows duplicates, so this vector won't insert if the asset_type already exists.
		unique_lock<shared_mutex> guard(mutex_IO_assets);
		assets.insert(pair<int, vector<Shared_Asset>>(asset_type, vector<Shared_Asset>()));
		return assets[asset_type];
	}
	map<int, Shared_Asset>& getFallbackAssets()
	{
		// Returns the asset map of 'default' or fallback assets
		return fallback_assets;
	}
	shared_mutex& getMutexIOAssets()
	{
		// Mutex to be used whenever accessing the asset map
		return mutex_IO_assets;
	}
	string getCurrentDir()
	{
		// Technique to return the running directory of the application
		char cCurrentPath[FILENAME_MAX];
		if (_getcwd(cCurrentPath, sizeof(cCurrentPath)))
			cCurrentPath[sizeof(cCurrentPath) - 1] = '/0'; 
		return string(cCurrentPath);
	}
	bool fileOnDisk(const string & name)
	{
		// Technique to return whether or not a given file or folder exists
		struct stat buffer;
		return (stat(name.c_str(), &buffer) == 0);
	}
}