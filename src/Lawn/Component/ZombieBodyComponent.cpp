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
	theArchive.Value("hasShadowOffsets", mHasShadowOffsets);
	theArchive.Value("shadowForwardX", mShadowForwardX);
	theArchive.Value("shadowForwardXPerScale", mShadowForwardXPerScale);
	theArchive.Value("shadowBackwardX", mShadowBackwardX);
	theArchive.Value("shadowBackwardXPerScale", mShadowBackwardXPerScale);
	theArchive.Value("shadowY", mShadowY);
	theArchive.Value("shadowScale", mShadowScale);
	theArchive.Value("headParticle", mHeadParticle);
	theArchive.Value("headImageId", mHeadImageId);
	theArchive.Value("headHiddenPrefixes", mHeadHiddenPrefixes);
	theArchive.Value("headRenderOffset", mHeadRenderOffset);
	theArchive.Value("charredReanim", mCharredReanim);
	theArchive.Value("charredX", mCharredX);
	theArchive.Value("charredY", mCharredY);
	theArchive.Value("charredBackwardsX", mCharredBackwardsX);
	theArchive.Value("charredAnim", mCharredAnim);
	theArchive.Value("burnsInPlace", mBurnsInPlace);
	theArchive.Value("burnsAsDamage", mBurnsAsDamage);
	theArchive.Value("deathAnimRateMin", mDeathAnimRateMin);
	theArchive.Value("deathAnimRateMax", mDeathAnimRateMax);
	theArchive.Value("hasDeathFall", mHasDeathFall);
	theArchive.Value("deathFallTime", mDeathFallTime);
	theArchive.Value("deathFallTimeAlt", mDeathFallTimeAlt);
	theArchive.Value("deathFallTimeSuperLong", mDeathFallTimeSuperLong);
	theArchive.Value("playsDeathAnimWhenDisintegrated", mPlaysDeathAnimWhenDisintegrated);
	theArchive.Value("puffsWhenMowed", mPuffsWhenMowed);
	theArchive.Value("appearFoley", mAppearFoley);
	theArchive.Value("songFoley", mSongFoley);
	theArchive.Value("houseX", mHouseX);
	theArchive.Value("houseY", mHouseY);
	theArchive.Value("poolHouseX", mPoolHouseX);
	theArchive.Value("roofHouseX", mRoofHouseX);
	theArchive.Value("roofHouseY", mRoofHouseY);
	theArchive.Value("idlesInHouse", mIdlesInHouse);
	theArchive.Value("eatAnimRate", mEatAnimRate);
	theArchive.Value("chewTimes", mChewTimes);
	theArchive.Value("footDustTimes", mFootDustTimes);
	theArchive.Value("walkAnimVariant", mWalkAnimVariant);
	theArchive.Value("dancesInDanceMode", mDancesInDanceMode);
	theArchive.Value("drawOffsetX", mDrawOffsetX);
	theArchive.Value("drawOffsetY", mDrawOffsetY);
	theArchive.Value("armHiddenTracks", mArmHiddenTracks);
	theArchive.Value("armHiddenByPrefix", mArmHiddenByPrefix);
	theArchive.Value("armTrack", mArmTrack);
	theArchive.Value("armUpperTrack", mArmUpperTrack);
	theArchive.Value("armUpperImageId", mArmUpperImageId);
	theArchive.Value("armImageId", mArmImageId);
	theArchive.Value("showsIceTrap", mShowsIceTrap);
	theArchive.Value("iceTrapX", mIceTrapX);
	theArchive.Value("iceTrapY", mIceTrapY);
	theArchive.Value("iceTrapScale", mIceTrapScale);
	theArchive.Value("canBeButtered", mCanBeButtered);
	theArchive.Value("butterHeadTrack", mButterHeadTrack);
	theArchive.Value("butterX", mButterX);
	theArchive.Value("butterY", mButterY);
	theArchive.Value("butterScale", mButterScale);
	theArchive.Value("hasYuckyFace", mHasYuckyFace);
	theArchive.Value("leavesDaisies", mLeavesDaisies);
	theArchive.Value("daisyX", mDaisyX);
	theArchive.Value("daisyY", mDaisyY);
	theArchive.Value("spawnHealth", mSpawnHealth);
	theArchive.Value("usesPlainZombieReanim", mUsesPlainZombieReanim);
	theArchive.Value("spawnShownPrefix", mSpawnShownPrefix);
	theArchive.Value("spawnHiddenPrefixes", mSpawnHiddenPrefixes);
	theArchive.Value("spawnFrames", mSpawnFrames);
	theArchive.Value("hitRect", mHitRect);
	theArchive.Value("hasAttackRect", mHasAttackRect);
	theArchive.Value("attackRect", mAttackRect);
	theArchive.Value("spawnWidth", mSpawnWidth);
	theArchive.Value("spawnHeight", mSpawnHeight);
	theArchive.Value("animFrames", mAnimFrames);
	theArchive.Value("animTicksPerFrame", mAnimTicksPerFrame);
	theArchive.Value("scale", mScale);
	theArchive.Value("carriesObject", mCarriesObject);
	theArchive.Value("canHaveTongue", mCanHaveTongue);
	theArchive.Value("hasSpawnX", mHasSpawnX);
	theArchive.Value("spawnX", mSpawnX);
	theArchive.Value("spawnXRandom", mSpawnXRandom);
	theArchive.Value("spawnXOffset", mSpawnXOffset);
	theArchive.Value("renderLayer", mRenderLayer);
	theArchive.Value("renderOffset", mRenderOffset);
	theArchive.Value("plantHeadReanim", mPlantHeadReanim);
	theArchive.Value("plantHeadAnim", mPlantHeadAnim);
	theArchive.Value("plantHeadTrack", mPlantHeadTrack);
	theArchive.Value("plantHeadBlanksTrack", mPlantHeadBlanksTrack);
	theArchive.Value("plantHeadX", mPlantHeadX);
	theArchive.Value("plantHeadY", mPlantHeadY);
	theArchive.Value("plantHeadScale", mPlantHeadScale);
	theArchive.Value("hasDuckyTube", mHasDuckyTube);
	theArchive.Value("spansAllRows", mSpansAllRows);
	theArchive.Value("groans", mGroans);
	theArchive.Value("headlessDecay", mHeadlessDecay);
	theArchive.Value("chewEffectX", mChewEffectX);
	theArchive.Value("chewEffectY", mChewEffectY);
	theArchive.Value("smallScaleOffsetY", mSmallScaleOffsetY);
	theArchive.Value("playsEatAnim", mPlaysEatAnim);
	theArchive.Value("eatsPlants", mEatsPlants);
	theArchive.Value("climbsLadders", mClimbsLadders);
	theArchive.Value("isBoss", mIsBoss);
	theArchive.Value("rowYOffset", mRowYOffset);
	theArchive.Value("losesBodyParts", mLosesBodyParts);
	theArchive.Value("shadowFollowsAltitude", mShadowFollowsAltitude);
	theArchive.Value("spriteGlides", mSpriteGlides);
	theArchive.Value("damagedBelow", mDamagedBelow);
	theArchive.Value("badlyDamagedBelow", mBadlyDamagedBelow);
}
