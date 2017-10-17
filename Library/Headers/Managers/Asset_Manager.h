/*
	Asset Manager

	- Manages the storage and retrieval of assets.
	- Uses static variables, designed to be used across an entire application with ease.
	- Can store and retrieve assets from different threads.
	- Preliminary support for multithreading, though it is up to each asset type to support it safely.
	- Static variables kept within source file

	Note: Asset loading initialized by "load_Asset(shared_ptr<ASSET_TYPE> &asset)"
*/

#pragma once
#ifndef ASSET_MANAGER
#define ASSET_MANAGER
#ifdef	ASSET_MANAGER_EXPORT
#define ASSET_MANAGER_API __declspec(dllexport)
#else
#define	ASSET_MANAGER_API __declspec(dllimport)
#endif

#include "Assets\Asset.h"
#include <deque>
#include <map>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

using namespace std;

namespace Asset_Manager {
	// Deletes all worker threads
	ASSET_MANAGER_API void shutdown();
	// Submits the provided work order for finalization, after initialization
	ASSET_MANAGER_API void submitWorkorder(const Shared_Asset &workorder);
	// Submits a new working thread to be kept track of. The pair is the thread and its completion state.
	ASSET_MANAGER_API void submitWorkthread(const pair<thread*, bool*> &thread);
	// Tick through the work orders, finalizing the front of the stack, and deleting all completed worker threads.
	ASSET_MANAGER_API void ParseWorkOrders();
	// Retrieves the vector of assets of the given type. Will create one if it doesn't exist.
	ASSET_MANAGER_API vector<Shared_Asset>& fetchAssetList(const int &asset_type);
	// Retrieves the fallback (default) assets
	ASSET_MANAGER_API map<int, Shared_Asset>& getFallbackAssets();
	// Retrieves the asset map mutex
	ASSET_MANAGER_API shared_mutex& getMutexIOAssets();
	// Retrieves the application's running directory
	ASSET_MANAGER_API string getCurrentDir();
	// Checks if a supplied file or folder exists on disk.
	ASSET_MANAGER_API bool fileOnDisk(const string &path);
}

#endif // ASSET_MANAGER