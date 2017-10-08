#pragma once
#ifndef ASSET_TEST
#define ASSET_TEST
#ifdef ASSET_TEST_EXPORT
#define DECLSPEC __declspec(dllexport)
#else
#define DECLSPEC __declspec(dllimport)
#endif

#include "Assets\Asset.h"

using namespace std;

class Asset_Test : public Asset
{
public:
	~Asset_Test();
	Asset_Test();

	virtual DECLSPEC void Finalize();
	static DECLSPEC int GetAssetType();
};

typedef shared_ptr<Asset_Test> Shared_Asset_Test;
#endif // ASSET_TEST