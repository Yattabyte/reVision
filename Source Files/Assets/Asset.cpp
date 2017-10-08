#include "Assets\Asset.h"

Asset::Asset()
{
	exists = false;
}

void Asset::Finalize()
{
	exists = true;
}

int Asset::GetAssetType()
{
	return -1;
}
