#pragma once
#ifndef ASSET
#define ASSET
#ifdef ASSET_EXPORT
#define DECLSPEC __declspec(dllexport)
#else
#define DECLSPEC __declspec(dllimport)
#endif

#include <mutex>
#include <shared_mutex>

using namespace std;

/*
	*****************************
	Asset Types List:
	*****************************

	Base					   -1
	Shader						0
	Collider					1
	Primitive					2
	Model						3
	Material					4
	Texture						5
	Texture_Cubemap				6
	Thumbnail					7
	Map							8

	*****************************
*/

class Asset
{
public:
	~Asset() {};
	Asset();;
	shared_mutex m_mutex;
	bool exists;

	virtual DECLSPEC void Finalize();	
	static DECLSPEC int GetAssetType();
};

typedef shared_ptr<Asset> Shared_Asset;
#endif // ASSET