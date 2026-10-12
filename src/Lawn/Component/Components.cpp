#include "Components.h"

REGISTER_COMPONENT(TeamComponent)
REGISTER_COMPONENT(FacingComponent)
REGISTER_COMPONENT(HypnotizableComponent)
REGISTER_COMPONENT(HypnotizeOnEatenComponent)
REGISTER_COMPONENT(LowProfileComponent)

void TeamComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value(mTeam);
	theArchive.Value(mNativeTeam);
}

void FacingComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value(mDirection);
	theArchive.Value(mNativeDirection);
}

void HypnotizableComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value(mHypnotizedTeam);
	theArchive.Value(mFlipFacing);
}

void HypnotizeOnEatenComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value(mRequiresAwake);
}
