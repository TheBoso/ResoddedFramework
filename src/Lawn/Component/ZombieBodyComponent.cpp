#include "ZombieBodyComponent.h"

REGISTER_COMPONENT(ZombieBodyComponent)

void ZombieBodyComponent::SetShadow(float theForwardX, float theBackwardX, float theY, float theScale)
{
	mHasShadowOffsets = true;
	mShadowForwardX = theForwardX;
	mShadowForwardXPerScale = 0.0f;
	mShadowBackwardX = theBackwardX;
	mShadowBackwardXPerScale = 0.0f;
	mShadowY = theY;
	mShadowScale = theScale;
}

void ZombieBodyComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value(mHasShadowOffsets);
	theArchive.Value(mShadowForwardX);
	theArchive.Value(mShadowForwardXPerScale);
	theArchive.Value(mShadowBackwardX);
	theArchive.Value(mShadowBackwardXPerScale);
	theArchive.Value(mShadowY);
	theArchive.Value(mShadowScale);
	theArchive.Value(mHeadParticle);
	theArchive.Value(mHeadImageId);
	theArchive.Value(mHeadHiddenPrefixes);
	theArchive.Value(mHeadRenderOffset);
	theArchive.Value(mCharredReanim);
	theArchive.Value(mCharredX);
	theArchive.Value(mCharredY);
	theArchive.Value(mCharredBackwardsX);
	theArchive.Value(mCharredAnim);
}
