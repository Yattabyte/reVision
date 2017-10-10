#include "Assets\Asset.h"

Asset::~Asset()
{
}

Asset::Asset()
{	
	finalized = false;
}

int Asset::GetAssetType() 
{
	return -1; 
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