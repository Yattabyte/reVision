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
			- void initialize_[Subclass Name]([any arguments here]) in cpp file
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
#define	ASSET_API __declspec(dllexport)
#else
#define	ASSET_API __declspec(dllimport)
#endif

#include <shared_mutex>

using namespace std;

class Asset
{
public:
	~Asset();
	Asset();
	shared_mutex m_mutex;
	// Performs final data processing
	virtual ASSET_API void Finalize();
	// Returns a UNIQUE asset type identifier. Each sub-class should have their own
	static ASSET_API int GetAssetType();
	// Returns whether or not this asset has completed finalizing
	ASSET_API bool ExistsYet();

protected:
	bool finalized = false;
};

// For ease of use, define the shared pointer to this class
typedef shared_ptr<Asset> Shared_Asset;
#endif // ASSET