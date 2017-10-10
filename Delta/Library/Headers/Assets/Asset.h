/*
	Asset

	- A basic data type meant to encapsulate some form of data loaded from disk
		- Example usages: Model, Shader, Sound, Level, etc
	- Intended to be basic and reusable
		- A particular asset file should be loaded from disk once and shared from memory whenever needed
	- Under this system, an asset can be stored once in the Asset_Manager, and referenced across an entire application

	

	*****************************************************************************************************
	*											NEW ASSET GUIDE											*
	*****************************************************************************************************

		* Implement the following:
			- virtual void Initialize();
			- virtual void Finalize();
			- static int GetAssetType();

		* Ensure Asset Type is unique!

		* typedef Shared_Asset:
			- typedef shared_ptr< Asset_[Subclass Name] > Shared_Asset_[Subclass Name];

		* Continue Asset_Manager namespace for asset loading function:
			-	namespace Asset_Manager {
					void load_asset(Shared_Asset_[Subclass Name] &asset, [any arguments needed]);
				};

	*****************************************************************************************************
*/

#pragma once
#ifndef	ASSET
#define	ASSET
#ifdef	ASSET_EXPORT
#define	DECLSPEC __declspec(dllexport)
#else
#define	DECLSPEC __declspec(dllimport)
#endif

#include <shared_mutex>

using namespace std;

class Asset
{
public:
	~Asset();
	Asset();
	shared_mutex m_mutex;
	// Loads and processes all relevant data from disk
	virtual DECLSPEC void Initialize() { initialized = true; }
	// Performs final data processing
	virtual DECLSPEC void Finalize() { if (initialized) finalized = true; }
	// Returns a UNIQUE asset type identifier. Each sub-class should have their own
	static DECLSPEC int GetAssetType() { return -1; };
	// Returns whether or not this asset has completed initializing and finalizing
	DECLSPEC bool ExistsYet() { return initialized ? finalized : false; }

protected:
	bool initialized = false, finalized = false;
};

// For ease of use, define the shared pointer to this class
typedef shared_ptr<Asset> Shared_Asset;
#endif // ASSET