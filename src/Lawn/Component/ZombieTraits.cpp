#include "ZombieTraits.h"

REGISTER_COMPONENT(UnswallowableComponent)
REGISTER_COMPONENT(GiantComponent)
REGISTER_COMPONENT(VehicleComponent)
REGISTER_COMPONENT(DanceTroupeComponent)
REGISTER_COMPONENT(SwimmerComponent)
REGISTER_COMPONENT(WalkerComponent)

void VehicleComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value("crushesSpikes", mCrushesSpikes);
	theArchive.Value("breakdownHealth", mBreakdownHealth);
}

void DanceTroupeComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value("isLeader", mIsLeader);
}

void SwimmerComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value("swimsUnderwater", mSwimsUnderwater);
}

void WalkerComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value("minSpeed", mMinSpeed);
	theArchive.Value("maxSpeed", mMaxSpeed);
	theArchive.Value("houseEdgeX", mHouseEdgeX);
	theArchive.Value("moveStyle", mMoveStyle);
}
