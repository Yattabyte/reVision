#include "Assets\Asset_Test.h"

Asset_Test::~Asset_Test()
{
}

Asset_Test::Asset_Test()
{
}

void Asset_Test::Finalize()
{
	exists = true;
}

int Asset_Test::GetAssetType()
{
	return 999;
}
