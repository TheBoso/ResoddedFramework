#include "CustomTypes.h"
#include "PlantTraits.h"
#include "../Board.h"

REGISTER_COMPONENT(SpikesComponent)
REGISTER_COMPONENT(TallComponent)
REGISTER_COMPONENT(WallComponent)
REGISTER_COMPONENT(RepelOnEatenComponent)
REGISTER_COMPONENT(InedibleComponent)
REGISTER_COMPONENT(ActivatesWhenEatenComponent)
REGISTER_COMPONENT(MineComponent)
REGISTER_COMPONENT(ProjectileConverterComponent)

void SpikesComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value("survivesCrushing", mSurvivesCrushing);
}

void WallComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value("showsChewMarks", mShowsChewMarks);
}

void InedibleComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value("blocksChewing", mBlocksChewing);
	theArchive.Value("blocksVehicles", mBlocksVehicles);
	theArchive.Value("ignoredByZombies", mIgnoredByZombies);
}

void ActivatesWhenEatenComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value("requiresAwake", mRequiresAwake);
}

// ─── ProjectileConverterComponent ────────────────────────────────────────────

ProjectileConverterComponent &ProjectileConverterComponent::Convert(ProjectileType theFrom, ProjectileType theTo)
{
	if (mCount < kMaxConversions)
	{
		mFrom[mCount] = theFrom;
		mTo[mCount] = theTo;
		mCount++;
	}
	return *this;
}

ProjectileType ProjectileConverterComponent::GetConversion(ProjectileType theType) const
{
	for (int i = 0; i < mCount; i++)
	{
		if (mFrom[i] == theType)
			return mTo[i];
	}
	return theType;
}

Rect ProjectileConverterComponent::GetConversionRect(Plant *thePlant)
{
	// Torchwood's flame: a narrow strip in the middle of its square
	return Rect(thePlant->mX + 50, thePlant->mY, 30, thePlant->mHeight);
}

void ProjectileConverterComponent::OnTick()
{
	Plant *aPlant = GetPlant();
	if (aPlant == nullptr || !aPlant->IsAbleToAct())
		return;

	Rect aRect = GetConversionRect(aPlant);
	Projectile *aProjectile = nullptr;
	while (GetBoard()->IterateProjectiles(aProjectile))
	{
		// Its own type's conversion, else (a custom projectile) its base's
		ProjectileType aOwnType = aProjectile->mDefinitionType;
		ProjectileType aConverted = GetConversion(aOwnType);
		if (aConverted == aOwnType)
		{
			aConverted = GetConversion(aProjectile->mProjectileType);
		}
		if (aProjectile->mRow == aPlant->mRow && aConverted != aOwnType && aConverted != aProjectile->mProjectileType &&
		    GetRectOverlap(aRect, aProjectile->GetProjectileRect()) >= 10)
		{
			aProjectile->ConvertTo(aConverted, aPlant->mPlantCol);
		}
	}
}

void ProjectileConverterComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value("count", mCount);
	theArchive.Value("from", mFrom);
	theArchive.Value("to", mTo);
}
