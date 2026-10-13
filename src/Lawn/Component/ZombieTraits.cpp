#include "ZombieTraits.h"

REGISTER_COMPONENT(UnswallowableComponent)
REGISTER_COMPONENT(GiantComponent)
REGISTER_COMPONENT(VehicleComponent)
REGISTER_COMPONENT(DanceTroupeComponent)
REGISTER_COMPONENT(SwimmerComponent)
REGISTER_COMPONENT(WalkerComponent)

void VehicleComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value(mCrushesSpikes);
	theArchive.Value(mBreakdownHealth);
}

void WalkerComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value(mMinSpeed);
	theArchive.Value(mMaxSpeed);
	theArchive.Value(mHouseEdgeX);
	theArchive.Value(mMoveStyle);
}
