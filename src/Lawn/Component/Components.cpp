#include "Components.h"

REGISTER_COMPONENT(TeamComponent)
REGISTER_COMPONENT(FacingComponent)
REGISTER_COMPONENT(HypnotizableComponent)
REGISTER_COMPONENT(HypnotizeOnEatenComponent)
REGISTER_COMPONENT(LowProfileComponent)

void TeamComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value("team", mTeam);
	theArchive.Value("nativeTeam", mNativeTeam);
}

void FacingComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value("direction", mDirection);
	theArchive.Value("nativeDirection", mNativeDirection);
}

void HypnotizableComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value("hypnotizedTeam", mHypnotizedTeam);
	theArchive.Value("flipFacing", mFlipFacing);
}

void HypnotizeOnEatenComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value("requiresAwake", mRequiresAwake);
}
