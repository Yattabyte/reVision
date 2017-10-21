#include "Assets\Asset.h"

/* -----ASSET TYPE----- */
#define ASSET_TYPE -1

Asset::~Asset()
{
}

Asset::Asset()
{	
	finalized = false;
}

int Asset::GetAssetType() 
{
	return ASSET_TYPE;
}

bool Asset::ExistsYet() 
{ 
	return finalized;
}

void Asset::Finalize()
{
	if (!finalized)
		finalized = true;
}