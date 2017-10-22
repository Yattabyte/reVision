#include "Asset_Collider.h"

/* -----ASSET TYPE----- */
#define ASSET_TYPE 0

Asset_Collider::~Asset_Collider()
{
	if (shape != nullptr)
		delete shape;
}

Asset_Collider::Asset_Collider()
{
	shape = nullptr;
	finalized = false;
}

Asset_Collider::Asset_Collider(const string & _filename) : Asset_Collider()
{
	filename = _filename;
}

Asset_Collider::Asset_Collider(btCollisionShape *new_shape)
{
	shape = new_shape;
	finalized = true;
}

int Asset_Collider::GetAssetType()
{
	return ASSET_TYPE;
}