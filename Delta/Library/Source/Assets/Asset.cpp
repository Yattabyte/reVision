#include "Assets\Asset.h"

Asset::~Asset()
{
}

Asset::Asset()
{
}

void Asset::Finalize()
{ 
	if (!finalized) 
		finalized = true; 
}

int Asset::GetAssetType() 
{
	return -1; 
}

bool Asset::ExistsYet() 
{ 
	return finalized;
}
