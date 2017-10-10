#include "Assets\Asset.h"

void Asset::Initialize() 
{ 
	initialized = true; 
}

void Asset::Finalize() 
{ 
	if (initialized) 
		finalized = true; 
}

int Asset::GetAssetType() 
{
	return -1; 
}

bool Asset::ExistsYet() 
{ 
	return	initialized 
			? finalized 
			: false; 
}
