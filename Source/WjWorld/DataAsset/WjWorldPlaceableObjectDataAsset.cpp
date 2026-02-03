// Fill out your copyright notice in the Description page of Project Settings.

#include "DataAsset/WjWorldPlaceableObjectDataAsset.h"

const FPlaceableObjectDefinition* UWjWorldPlaceableObjectDataAsset::FindByObjectId(FName ObjectId) const
{
	for (const FPlaceableObjectDefinition& Def : Objects)
	{
		if (Def.ObjectId == ObjectId)
		{
			return &Def;
		}
	}
	return nullptr;
}
