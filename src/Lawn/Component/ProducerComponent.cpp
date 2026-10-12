#include "ProducerComponent.h"
#include "StatusEffects.h"
#include "../Board.h"
#include "../Challenge.h"
#include "../../LawnApp.h"
#include "../../Sexy.TodLib/TodCommon.h"
#include "../../Sexy.TodLib/TodFoley.h"

REGISTER_COMPONENT(ProducerComponent)
REGISTER_COMPONENT(SunShroomProducerComponent)

bool ProducerComponent::MakesSun() const
{
	return mCoinType == CoinType::COIN_SUN || mCoinType == CoinType::COIN_SMALLSUN ||
	       mCoinType == CoinType::COIN_LARGESUN;
}

void ProducerComponent::OnAttach()
{
	if (mCountdown < 0)
	{
		// Same start delays the vanilla plants used
		mCountdown = MakesSun() ? RandRangeInt(300, std::max(300, mRate / 2)) : RandRangeInt(0, mRate);
	}
}

bool ProducerComponent::CanProduce()
{
	LawnApp *anApp = GetApp();
	Board *aBoard = GetBoard();
	if (anApp->IsIZombieLevel() || anApp->IsWallnutBowlingLevel() || anApp->mGameMode == GameMode::GAMEMODE_UPSELL ||
	    anApp->mGameMode == GameMode::GAMEMODE_INTRO || aBoard->HasLevelAwardDropped())
		return false;

	if (anApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND &&
	    aBoard->mChallenge->mChallengeState != ChallengeState::STATECHALLENGE_LAST_STAND_ONSLAUGHT)
		return false;

	if (Plant *aPlant = GetPlant())
	{
		return aPlant->IsInPlay() && !aPlant->mIsAsleep && !aPlant->mSquished &&
		       aPlant->mOnBungeeState == PlantOnBungeeState::NOT_ON_BUNGEE;
	}
	if (Zombie *aZombie = GetZombie())
	{
		return aZombie->IsOnBoard() && !aZombie->IsDeadOrDying();
	}
	return false;
}

CoinType ProducerComponent::PickCoinType()
{
	if (mRareCoinType != CoinType::COIN_NONE && Sexy::Rand(100) < mRareChance)
		return mRareCoinType;
	return mCoinType;
}

void ProducerComponent::OnTick()
{
	if (!CanProduce())
		return;

	// Frozen/buttered producers pause, chilled ones run at half speed
	GameObject *anOwner = GetOwnerObject();
	if (anOwner->IsFrozen() || anOwner->IsButtered() ||
	    (anOwner->IsChilled() && GetBoard()->mMainCounter % 2 == 1))
		return;

	Board *aBoard = GetBoard();
	if (mStopsAfterFinalWave && aBoard->mCurrentWave == aBoard->mNumWaves)
	{
		if (mFinalWaveCountdown < 0)
		{
			mFinalWaveCountdown = 6000;
		}
		else if (mFinalWaveCountdown == 0)
		{
			return;
		}
		mFinalWaveCountdown--;
	}

	mCountdown--;

	// Plants glow just before they produce
	Plant *aPlant = GetPlant();
	if (aPlant && mCountdown <= 100)
	{
		int aFlashCountdown = TodAnimateCurve(100, 0, mCountdown, 0, 100, TodCurves::CURVE_LINEAR);
		aPlant->mEatenFlashCountdown = std::max(aPlant->mEatenFlashCountdown, aFlashCountdown);
	}

	if (mCountdown > 0)
		return;

	mCountdown = RandRangeInt(std::max(1, mRate - 150), std::max(1, mRate));
	GetApp()->PlayFoley(FoleyType::FOLEY_SPAWN_SUN);

	for (int i = 0; i < mCoinsPerDrop; i++)
	{
		aBoard->AddCoin(anOwner->mX, anOwner->mY, PickCoinType(), mCoinMotion);
	}
	if (mBigTimeBonusCoin != CoinType::COIN_NONE && GetApp()->mGameMode == GameMode::GAMEMODE_CHALLENGE_BIG_TIME)
	{
		aBoard->AddCoin(anOwner->mX, anOwner->mY, mBigTimeBonusCoin, mCoinMotion);
	}
}

void ProducerComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value(mCoinType);
	theArchive.Value(mCoinsPerDrop);
	theArchive.Value(mRate);
	theArchive.Value(mCoinMotion);
	theArchive.Value(mRareCoinType);
	theArchive.Value(mRareChance);
	theArchive.Value(mBigTimeBonusCoin);
	theArchive.Value(mStopsAfterFinalWave);
	theArchive.Value(mCountdown);
	theArchive.Value(mFinalWaveCountdown);
}

bool SunShroomProducerComponent::CanProduce()
{
	Plant *aPlant = GetPlant();
	return ProducerComponent::CanProduce() && (!aPlant || aPlant->mState != PlantState::STATE_SUNSHROOM_GROWING);
}

CoinType SunShroomProducerComponent::PickCoinType()
{
	Plant *aPlant = GetPlant();
	return aPlant && aPlant->mState == PlantState::STATE_SUNSHROOM_SMALL ? CoinType::COIN_SMALLSUN
	                                                                      : CoinType::COIN_SUN;
}
