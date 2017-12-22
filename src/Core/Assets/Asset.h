/*
	Asset

	- A basic data type meant to encapsulate some form of data loaded from disk
		- Example usages: Model, Shader, Sound, Level, etc
	- Intended to be basic and reusable
		- A particular asset file should be loaded from disk once and shared from memory whenever needed
	- Under this system, an asset can be stored once in the Asset_Manager, and referenced across an entire application
*/

#pragma once
#ifndef	ASSET
#define	ASSET
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include <shared_mutex>
#include <vector>

using namespace std;

class Asset_Observer;
class Asset;
typedef shared_ptr<Asset> Shared_Asset;
class DT_ENGINE_API Asset
{
public:
	// Destroyed when no longer used only
	~Asset();
	// Zero Initialization only
	Asset(const string & filename = "");
	shared_mutex m_mutex;	
	// Returns a UNIQUE asset type identifier. Each sub-class should have their own
	static int GetAssetType();
	// Gets the name of this asset
	string GetFileName() const;
	// Sets the name of this asset to @fn
	void SetFileName(const string & filename);
	// Returns whether or not this asset has completed finalizing
	bool ExistsYet();
	// Performs final data processing
	virtual void Finalize();
	// Adds a state observer/listener
	void AddObserver(Asset_Observer * observer);
	// Removes a state observer/listener
	void RemoveObserver(Asset_Observer * observer);


protected:
	bool m_finalized;
	string m_filename;
	vector<Asset_Observer*> m_observers;
};

class DT_ENGINE_API Asset_Observer
{
public:
	Asset_Observer(Asset *asset);
	virtual ~Asset_Observer();
	virtual void Notify_Finalized() = 0;
};

#endif // ASSET