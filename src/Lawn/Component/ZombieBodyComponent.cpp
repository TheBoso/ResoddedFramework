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

void ZombieBodyComponent::SetDeathFall(float theTime)
{
	mHasDeathFall = true;
	mDeathFallTime = theTime;
	mDeathFallTimeAlt = theTime;
	mDeathFallTimeSuperLong = theTime;
}

void ZombieBodyComponent::SetDeathAnimRate(float theRate)
{
	mDeathAnimRateMin = theRate;
	mDeathAnimRateMax = theRate;
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
	theArchive.Value(mBurnsInPlace);
	theArchive.Value(mBurnsAsDamage);
	theArchive.Value(mDeathAnimRateMin);
	theArchive.Value(mDeathAnimRateMax);
	theArchive.Value(mHasDeathFall);
	theArchive.Value(mDeathFallTime);
	theArchive.Value(mDeathFallTimeAlt);
	theArchive.Value(mDeathFallTimeSuperLong);
	theArchive.Value(mPlaysDeathAnimWhenDisintegrated);
	theArchive.Value(mPuffsWhenMowed);
	theArchive.Value(mAppearFoley);
	theArchive.Value(mSongFoley);
	theArchive.Value(mHouseX);
	theArchive.Value(mHouseY);
	theArchive.Value(mPoolHouseX);
	theArchive.Value(mRoofHouseX);
	theArchive.Value(mRoofHouseY);
	theArchive.Value(mIdlesInHouse);
	theArchive.Value(mEatAnimRate);
	theArchive.Value(mChewTimes);
	theArchive.Value(mFootDustTimes);
	theArchive.Value(mWalkAnimVariant);
	theArchive.Value(mDancesInDanceMode);
	theArchive.Value(mDrawOffsetX);
	theArchive.Value(mDrawOffsetY);
	theArchive.Value(mArmHiddenTracks);
	theArchive.Value(mArmHiddenByPrefix);
	theArchive.Value(mArmTrack);
	theArchive.Value(mArmUpperTrack);
	theArchive.Value(mArmUpperImageId);
	theArchive.Value(mArmImageId);
	theArchive.Value(mShowsIceTrap);
	theArchive.Value(mIceTrapX);
	theArchive.Value(mIceTrapY);
	theArchive.Value(mIceTrapScale);
	theArchive.Value(mCanBeButtered);
	theArchive.Value(mButterHeadTrack);
	theArchive.Value(mButterX);
	theArchive.Value(mButterY);
	theArchive.Value(mButterScale);
	theArchive.Value(mHasYuckyFace);
}
