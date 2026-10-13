// Behaviours of the vanilla zombies, moved out of Zombie.cpp (they were Zombie::UpdateZombiePolevaulter and
// friends). They act on the zombie they belong to through theZombie.

#include "ZombieBehaviours.h"
#include "Components.h"
#include "Teams.h"
#include "../Plant.h"
#include "../Board.h"
#include "../Zombie.h"
#include "../Cutscene.h"
#include "../GridItem.h"
#include "../LawnMower.h"
#include "../Challenge.h"
#include "../Projectile.h"
#include "../../LawnApp.h"
#include "../../Resources.h"
#include "../System/PlayerInfo.h"
#include "../System/Music.h"
#include "../System/Achievements.h"
#include "../../Sexy.TodLib/TodFoley.h"
#include "../../Sexy.TodLib/TodDebug.h"
#include "../../Sexy.TodLib/TodCommon.h"
#include "../../Sexy.TodLib/Reanimator.h"
#include "../../Sexy.TodLib/Attachment.h"
#include "../../Sexy.TodLib/TodParticle.h"

REGISTER_COMPONENT(PolevaulterBehaviour)
REGISTER_COMPONENT(CatapultBehaviour)
REGISTER_COMPONENT(DolphinRiderBehaviour)
REGISTER_COMPONENT(SnorkelBehaviour)
REGISTER_COMPONENT(BalloonBehaviour)
REGISTER_COMPONENT(NewspaperBehaviour)
REGISTER_COMPONENT(DiggerBehaviour)
REGISTER_COMPONENT(JackInTheBoxBehaviour)
REGISTER_COMPONENT(GargantuarBehaviour)
REGISTER_COMPONENT(BobsledBehaviour)
REGISTER_COMPONENT(ZamboniBehaviour)
REGISTER_COMPONENT(LadderBehaviour)
REGISTER_COMPONENT(YetiBehaviour)
REGISTER_COMPONENT(DancerBehaviour)
REGISTER_COMPONENT(BackupDancerBehaviour)
REGISTER_COMPONENT(ImpBehaviour)
REGISTER_COMPONENT(PeaHeadBehaviour)
REGISTER_COMPONENT(JalapenoHeadBehaviour)
REGISTER_COMPONENT(GatlingHeadBehaviour)
REGISTER_COMPONENT(SquashHeadBehaviour)
REGISTER_COMPONENT(BossBehaviour)
REGISTER_COMPONENT(BungeeBehaviour)
REGISTER_COMPONENT(PogoBehaviour)
REGISTER_COMPONENT(FlagBearerBehaviour)

void PolevaulterBehaviour::UpdateActions(Zombie *theZombie)
{
	if (theZombie->mZombiePhase == ZombiePhase::PHASE_POLEVAULTER_PRE_VAULT && theZombie->mHasHead &&
	    theZombie->mZombieHeight == ZombieHeight::HEIGHT_ZOMBIE_NORMAL)
	{
		Plant *aPlant = theZombie->FindPlantTarget(ZombieAttackType::ATTACKTYPE_VAULT);
		if (aPlant)
		{
			if (theZombie->mBoard->GetLadderAt(aPlant->mPlantCol, aPlant->mRow))
			{
				float aPlantX = theZombie->mBoard->GridToPixelX(aPlant->mPlantCol, aPlant->mRow) + 40;
				if (aPlantX > theZombie->mPosX && theZombie->mZombieHeight == ZombieHeight::HEIGHT_ZOMBIE_NORMAL &&
				    theZombie->mUseLadderCol != aPlant->mPlantCol)
				{
					theZombie->mZombieHeight = ZombieHeight::HEIGHT_UP_LADDER;
					theZombie->mUseLadderCol = aPlant->mPlantCol;
				}
				return;
			}

			theZombie->mZombiePhase = ZombiePhase::PHASE_POLEVAULTER_IN_VAULT;
			theZombie->PlayZombieReanim("anim_jump", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 24.0f);

			Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);
			float aAnimDuration = aBodyReanim->mFrameCount / aBodyReanim->mAnimRate * 100.0f;
			int aJumpDistance = theZombie->mX - aPlant->mX - 80;
			if (theZombie->mApp->IsWallnutBowlingLevel())
			{
				aJumpDistance = 0;
			}
			theZombie->mVelX = aJumpDistance / aAnimDuration;
			theZombie->mHasObject = false;
		}

		if (theZombie->mApp->IsIZombieLevel() && theZombie->mBoard->mChallenge->IZombieGetBrainTarget(theZombie))
		{
			theZombie->mZombiePhase = ZombiePhase::PHASE_POLEVAULTER_POST_VAULT;
			theZombie->StartWalkAnim(0);
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_POLEVAULTER_IN_VAULT)
	{
		Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);

		bool aJumpEnds = false;
		if (aBodyReanim->mAnimTime > 0.6f && aBodyReanim->mAnimTime <= 0.7f)
		{
			Plant *aPlant = theZombie->FindPlantTarget(ZombieAttackType::ATTACKTYPE_VAULT);
			if (aPlant && aPlant->HasComponent<TallComponent>())
			{
				theZombie->mApp->PlayFoley(FoleyType::FOLEY_BONK);
				aJumpEnds = true;
				theZombie->mApp->AddTodParticle(aPlant->mX + 60, aPlant->mY - 20, theZombie->mRenderOrder + 1,
				                                ParticleEffect::PARTICLE_TALL_NUT_BLOCK);

				theZombie->mZombieHeight = ZombieHeight::HEIGHT_FALLING;
				theZombie->mPosX = aPlant->mX;
				theZombie->mPosY -= 30.0f;
			}
		}

		if (aBodyReanim->mLoopCount > 0)
		{
			aJumpEnds = true;
			theZombie->mPosX -= 150.0f;
		}
		if (aBodyReanim->ShouldTriggerTimedEvent(0.2f))
		{
			theZombie->mApp->PlayFoley(FoleyType::FOLEY_GRASSSTEP);
		}
		if (aBodyReanim->ShouldTriggerTimedEvent(0.4f))
		{
			theZombie->mApp->PlayFoley(FoleyType::FOLEY_POLEVAULT);
		}

		if (aJumpEnds)
		{
			theZombie->mX = (int)theZombie->mPosX;
			theZombie->mZombiePhase = ZombiePhase::PHASE_POLEVAULTER_POST_VAULT;
			theZombie->mZombieAttackRect = Rect(50, 0, 20, 115);

			theZombie->StartWalkAnim(0);
		}
		else
		{
			float aOldPosX = theZombie->mPosX;
			theZombie->mPosX -= 150.0f * aBodyReanim->mAnimTime;
			theZombie->mPosY = theZombie->GetPosYBasedOnRow(theZombie->mRow);
			theZombie->mPosX = aOldPosX;
		}
	}
}

void CatapultBehaviour::UpdateActions(Zombie *theZombie)
{
	if (theZombie->mZombiePhase == ZombiePhase::PHASE_ZOMBIE_NORMAL)
	{
		if (theZombie->mPosX <= 650 && theZombie->FindCatapultTarget() && theZombie->mSummonCounter > 0)
		{
			theZombie->mZombiePhase = ZombiePhase::PHASE_CATAPULT_LAUNCHING;
			theZombie->mPhaseCounter = 300;
			theZombie->PlayZombieReanim("anim_shoot", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, 24.0f);
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_CATAPULT_LAUNCHING)
	{
		Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);
		if (aBodyReanim->ShouldTriggerTimedEvent(0.545f))
		{
			Plant *aPlant = theZombie->FindCatapultTarget();
			theZombie->ZombieCatapultFire(aPlant);
		}
		if (aBodyReanim->mLoopCount > 0)
		{
			theZombie->mSummonCounter--;
			if (theZombie->mSummonCounter == 4)
			{
				theZombie->ReanimShowTrack("Zombie_catapult_basketball", RENDER_GROUP_HIDDEN);
			}
			else if (theZombie->mSummonCounter == 3)
			{
				theZombie->ReanimShowTrack("Zombie_catapult_basketball2", RENDER_GROUP_HIDDEN);
			}
			else if (theZombie->mSummonCounter == 2)
			{
				theZombie->ReanimShowTrack("Zombie_catapult_basketball3", RENDER_GROUP_HIDDEN);
			}
			else if (theZombie->mSummonCounter == 1)
			{
				theZombie->ReanimShowTrack("Zombie_catapult_basketball4", RENDER_GROUP_HIDDEN);
			}

			if (theZombie->mSummonCounter == 0)
			{
				theZombie->PlayZombieReanim("anim_walk", ReanimLoopType::REANIM_LOOP, 20, 6.0f);
				theZombie->mZombiePhase = ZombiePhase::PHASE_ZOMBIE_NORMAL;
			}
			else
			{
				theZombie->PlayZombieReanim("anim_idle", ReanimLoopType::REANIM_LOOP, 20, 12.0f);
				theZombie->mZombiePhase = ZombiePhase::PHASE_CATAPULT_RELOADING;
			}
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_CATAPULT_RELOADING && theZombie->mPhaseCounter == 0)
	{
		Plant *aPlant = theZombie->FindCatapultTarget();
		if (aPlant)
		{
			theZombie->mZombiePhase = ZombiePhase::PHASE_CATAPULT_LAUNCHING;
			theZombie->mPhaseCounter = 300;
			theZombie->PlayZombieReanim("anim_shoot", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 24.0f);
		}
		else
		{
			theZombie->PlayZombieReanim("anim_walk", ReanimLoopType::REANIM_LOOP, 20, 6.0f);
			theZombie->mZombiePhase = ZombiePhase::PHASE_ZOMBIE_NORMAL;
		}
	}
}

void DolphinRiderBehaviour::UpdateActions(Zombie *theZombie)
{
	if (theZombie->IsTangleKelpTarget())
		return;

	bool aBackwards = theZombie->IsWalkingBackwards();
	if (theZombie->mZombiePhase == ZombiePhase::PHASE_DOLPHIN_WALKING && !aBackwards)
	{
		if (theZombie->mX > 700 && theZombie->mX <= 720)
		{
			theZombie->mZombiePhase = ZombiePhase::PHASE_DOLPHIN_INTO_POOL;
			theZombie->PlayZombieReanim("anim_jumpinpool", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 16.0f);
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_DOLPHIN_INTO_POOL)
	{
		Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);
		if (aBodyReanim->ShouldTriggerTimedEvent(0.56f))
		{
			Reanimation *aSplashReanim =
			    theZombie->mApp->AddReanimation(theZombie->mX - 83, theZombie->mY + 73, theZombie->mRenderOrder + 1, ReanimationType::REANIM_SPLASH);
			aSplashReanim->OverrideScale(1.2f, 0.8f);
			theZombie->mApp->AddTodParticle(theZombie->mX - 46, theZombie->mY + 115, theZombie->mRenderOrder + 1, ParticleEffect::PARTICLE_PLANTING_POOL);
			theZombie->mApp->PlayFoley(FoleyType::FOLEY_ZOMBIE_ENTERING_WATER);
		}

		if (aBodyReanim->mLoopCount > 0)
		{
			theZombie->mPosX -= 70.0f;
			theZombie->mZombiePhase = ZombiePhase::PHASE_DOLPHIN_RIDING;
			theZombie->mInPool = true;
			theZombie->mZombieAttackRect = Rect(-29, 0, 70, 115);
			theZombie->PlayZombieReanim("anim_ride", ReanimLoopType::REANIM_LOOP_FULL_LAST_FRAME, 0, 12.0f);
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_DOLPHIN_RIDING)
	{
		if (theZombie->mX <= 10)
		{
			theZombie->mAltitude = -40.0f;
			theZombie->mZombieHeight = ZombieHeight::HEIGHT_OUT_OF_POOL;
			theZombie->mZombiePhase = ZombiePhase::PHASE_DOLPHIN_WALKING;

			theZombie->PoolSplash(false);
			theZombie->PlayZombieReanim("anim_walkdolphin", ReanimLoopType::REANIM_LOOP, 0, 0.0f);
			theZombie->PickRandomSpeed();
			return;
		}

		if (theZombie->mHasHead && !theZombie->IsTanglekelpTarget())
		{
			Plant *aPlant = theZombie->FindPlantTarget(ZombieAttackType::ATTACKTYPE_VAULT);
			if (aPlant)
			{
				theZombie->mApp->PlayFoley(FoleyType::FOLEY_DOLPHIN_BEFORE_JUMPING);
				theZombie->mApp->PlayFoley(FoleyType::FOLEY_PLANT_WATER);

				theZombie->mVelX = 0.5f;
				theZombie->mZombiePhase = ZombiePhase::PHASE_DOLPHIN_IN_JUMP;
				theZombie->mPhaseCounter = DOLPHIN_JUMP_TIME;
				theZombie->PlayZombieReanim("anim_dolphinjump", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, 10.0f);
			}
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_DOLPHIN_IN_JUMP)
	{
		Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);
		theZombie->mAltitude = TodAnimateCurveFloat(DOLPHIN_JUMP_TIME, 0, theZombie->mPhaseCounter, 0.0f, 10.0f, TodCurves::CURVE_LINEAR);

		bool aJumpEnds = false;
		if (aBodyReanim->ShouldTriggerTimedEvent(0.3f))
		{
			Plant *aPlant = theZombie->FindPlantTarget(ZombieAttackType::ATTACKTYPE_VAULT);
			if (aPlant && aPlant->HasComponent<TallComponent>())
			{
				theZombie->mApp->PlayFoley(FoleyType::FOLEY_BONK);
				aJumpEnds = true;
				theZombie->mApp->AddTodParticle(aPlant->mX + 60, aPlant->mY - 20, theZombie->mRenderOrder + 1,
				                                ParticleEffect::PARTICLE_TALL_NUT_BLOCK);

				theZombie->mZombieHeight = ZombieHeight::HEIGHT_FALLING;
				theZombie->mPosX = aPlant->mX + 25.0f;
				theZombie->mAltitude = 30.0f;
			}
		}
		else if (aBodyReanim->ShouldTriggerTimedEvent(0.49f))
		{
			Reanimation *aSplashReanim =
			    theZombie->mApp->AddReanimation(theZombie->mX - 63, theZombie->mY + 73, theZombie->mRenderOrder + 1, ReanimationType::REANIM_SPLASH);
			aSplashReanim->OverrideScale(1.2f, 0.8f);
			theZombie->mApp->AddTodParticle(theZombie->mX - 26, theZombie->mY + 115, theZombie->mRenderOrder + 1, ParticleEffect::PARTICLE_PLANTING_POOL);
			theZombie->mApp->PlayFoley(FoleyType::FOLEY_ZOMBIE_ENTERING_WATER);
			theZombie->mVelX = 0.0f;
		}
		else if (aBodyReanim->mLoopCount > 0)
		{
			aJumpEnds = true;
			theZombie->mPosX -= 94.0f;
			theZombie->mAltitude = 0.0f;
		}

		if (aJumpEnds)
		{
			theZombie->mZombieAttackRect = Rect(30, 0, 30, 115);
			theZombie->mZombieRect = Rect(20, 0, 42, 115);
			theZombie->mZombiePhase = ZombiePhase::PHASE_DOLPHIN_WALKING_IN_POOL;
			theZombie->StartWalkAnim(0);
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_DOLPHIN_WALKING_IN_POOL)
	{
		if ((theZombie->mX <= 10 && !aBackwards) || (theZombie->mX > 680 && aBackwards))
		{
			theZombie->mAltitude = -40.0f;
			theZombie->mZombieHeight = ZombieHeight::HEIGHT_OUT_OF_POOL;
			theZombie->mZombiePhase = ZombiePhase::PHASE_DOLPHIN_WALKING_WITHOUT_DOLPHIN;

			theZombie->PoolSplash(false);
			theZombie->PlayZombieReanim("anim_walk", ReanimLoopType::REANIM_LOOP, 0, 0.0f);
			theZombie->PickRandomSpeed();
		}
	}
}

void SnorkelBehaviour::UpdateActions(Zombie *theZombie)
{
	bool aBackwards = theZombie->IsWalkingBackwards();
	if (theZombie->mZombiePhase == ZombiePhase::PHASE_SNORKEL_WALKING && !aBackwards)
	{
		if (theZombie->mX > 700 && theZombie->mX <= 720)
		{
			theZombie->mVelX = 0.2f;
			theZombie->mZombiePhase = ZombiePhase::PHASE_SNORKEL_INTO_POOL;
			theZombie->PlayZombieReanim("anim_jumpinpool", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 16.0f);
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_SNORKEL_INTO_POOL)
	{
		Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);
		theZombie->mAltitude = TodAnimateCurveFloat(0, 1000, aBodyReanim->mAnimTime * 1000, 0.0f, 10.0f, TodCurves::CURVE_LINEAR);

		if (aBodyReanim->ShouldTriggerTimedEvent(0.83f))
		{
			Reanimation *aSplashReanim =
			    theZombie->mApp->AddReanimation(theZombie->mX - 47, theZombie->mY + 73, theZombie->mRenderOrder + 1, ReanimationType::REANIM_SPLASH);
			aSplashReanim->OverrideScale(1.2f, 0.8f);
			theZombie->mApp->AddTodParticle(theZombie->mX - 10, theZombie->mY + 115, theZombie->mRenderOrder + 1, ParticleEffect::PARTICLE_PLANTING_POOL);
			theZombie->mApp->PlayFoley(FoleyType::FOLEY_ZOMBIE_ENTERING_WATER);
		}

		if (aBodyReanim->mLoopCount > 0)
		{
			theZombie->mZombiePhase = ZombiePhase::PHASE_SNORKEL_WALKING_IN_POOL;
			theZombie->mInPool = true;
			theZombie->PlayZombieReanim("anim_swim", ReanimLoopType::REANIM_LOOP_FULL_LAST_FRAME, 0, 12.0f);
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_SNORKEL_WALKING_IN_POOL)
	{
		if (!theZombie->mHasHead)
		{
			theZombie->TakeDamage(1800,
			                      GetBit(DamageFlags::DAMAGE_BYPASSES_SHIELD) | GetBit(DamageFlags::DAMAGE_DOESNT_CAUSE_FLASH));
		}
		else if (theZombie->mX <= 25 && !aBackwards)
		{
			theZombie->mAltitude = -90.0f;
			theZombie->mPosX -= 15.0f;
			theZombie->mZombiePhase = ZombiePhase::PHASE_SNORKEL_WALKING;
			theZombie->mZombieHeight = ZombieHeight::HEIGHT_OUT_OF_POOL;

			theZombie->PoolSplash(false);
			theZombie->StartWalkAnim(0);
		}
		else if (theZombie->mX > 640 && aBackwards)
		{
			theZombie->mAltitude = -90.0f;
			theZombie->mPosX += 15.0f;
			theZombie->mZombiePhase = ZombiePhase::PHASE_SNORKEL_WALKING;
			theZombie->mZombieHeight = ZombieHeight::HEIGHT_OUT_OF_POOL;

			theZombie->PoolSplash(false);
			theZombie->StartWalkAnim(0);
		}
		else if (theZombie->mIsEating)
		{
			theZombie->mZombiePhase = ZombiePhase::PHASE_SNORKEL_UP_TO_EAT;
			theZombie->PlayZombieReanim("anim_uptoeat", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, 24.0f);
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_SNORKEL_UP_TO_EAT)
	{
		Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);
		if (!theZombie->mIsEating)
		{
			theZombie->mZombiePhase = ZombiePhase::PHASE_SNORKEL_DOWN_FROM_EAT;
			theZombie->PlayZombieReanim("anim_uptoeat", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, -24.0f);
		}
		else if (aBodyReanim->mLoopCount > 0)
		{
			theZombie->mZombiePhase = ZombiePhase::PHASE_SNORKEL_EATING_IN_POOL;
			theZombie->PlayZombieReanim("anim_eat", ReanimLoopType::REANIM_LOOP, 0, 0.0f);
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_SNORKEL_EATING_IN_POOL)
	{
		if (!theZombie->mIsEating)
		{
			theZombie->mZombiePhase = ZombiePhase::PHASE_SNORKEL_DOWN_FROM_EAT;
			theZombie->PlayZombieReanim("anim_uptoeat", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, -24.0f);
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_SNORKEL_DOWN_FROM_EAT)
	{
		Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);
		if (aBodyReanim->mLoopCount > 0)
		{
			theZombie->mZombiePhase = ZombiePhase::PHASE_SNORKEL_WALKING_IN_POOL;
			theZombie->PlayZombieReanim("anim_swim", ReanimLoopType::REANIM_LOOP_FULL_LAST_FRAME, 0, 12.0f);
			theZombie->PickRandomSpeed();
		}
	}
}

void BalloonBehaviour::UpdateActions(Zombie *theZombie)
{
	if (theZombie->mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_HIGH_GRAVITY && theZombie->mPosX < 720.0f)
	{
		theZombie->mAltitude -= 0.1f;
		if (theZombie->mAltitude < -35.0f)
		{
			theZombie->LandFlyer(0U);
		}
	}

	if (theZombie->mZombiePhase == ZombiePhase::PHASE_BALLOON_POPPING)
	{
		Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);
		if (aBodyReanim->mLoopCount > 0)
		{
			theZombie->mZombiePhase = ZombiePhase::PHASE_BALLOON_WALKING;
			theZombie->StartWalkAnim(0);
		}
	}

	if (theZombie->mApp->IsIZombieLevel() && theZombie->mZombiePhase == ZombiePhase::PHASE_BALLOON_FLYING &&
	    theZombie->mBoard->mChallenge->IZombieGetBrainTarget(theZombie))
	{
		theZombie->LandFlyer(0U);
	}
}

void NewspaperBehaviour::UpdateActions(Zombie *theZombie)
{
	if (theZombie->mZombiePhase == ZombiePhase::PHASE_NEWSPAPER_MADDENING)
	{
		Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);
		if (aBodyReanim->mLoopCount > 0)
		{
			theZombie->mZombiePhase = ZombiePhase::PHASE_NEWSPAPER_MAD;
			if (theZombie->mBoard->CountZombiesOnScreen() <= 10 && theZombie->mHasHead)
			{
				theZombie->mApp->PlayFoley(FoleyType::FOLEY_NEWSPAPER_RARRGH);
			}

			theZombie->StartWalkAnim(20);
			aBodyReanim->SetImageOverride("anim_head1", IMAGE_REANIM_ZOMBIE_PAPER_MADHEAD);
		}
	}
}

void DiggerBehaviour::UpdateActions(Zombie *theZombie)
{
	if (theZombie->mZombiePhase == ZombiePhase::PHASE_DIGGER_TUNNELING)
	{
		if (theZombie->mPosX < 10.0f)
		{
			theZombie->mAltitude = -120.0f;
			theZombie->mZombiePhase = ZombiePhase::PHASE_DIGGER_RISING;
			theZombie->mPhaseCounter = 130;
			theZombie->PlayZombieReanim("anim_drill", ReanimLoopType::REANIM_LOOP, 0, 20.0f);

			theZombie->mApp->PlayFoley(FoleyType::FOLEY_DIRT_RISE);
			theZombie->mApp->PlayFoley(FoleyType::FOLEY_WAKEUP);
			AttachmentDetachCrossFadeParticleType(theZombie->mAttachmentID, ParticleEffect::PARTICLE_DIGGER_TUNNEL, nullptr);
			theZombie->StopZombieSound();

			theZombie->mApp->AddTodParticle(theZombie->mPosX + 60.0f, theZombie->mPosY + 118.0f, theZombie->mRenderOrder + 1, ParticleEffect::PARTICLE_DIGGER_RISE);
			Reanimation *aDirtReanim = theZombie->mApp->AddReanimation(theZombie->mPosX + 13.0f, theZombie->mPosY + 97.0f, theZombie->mRenderOrder + 1,
			                                                           ReanimationType::REANIM_DIGGER_DIRT);
			aDirtReanim->mAnimRate = 24.0f;
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_DIGGER_RISING)
	{
		if (theZombie->mPhaseCounter > 40)
		{
			theZombie->mAltitude = TodAnimateCurve(130, 40, theZombie->mPhaseCounter, -120, 20, TodCurves::CURVE_EASE_OUT);
		}
		else
		{
			theZombie->mAltitude = TodAnimateCurve(30, 0, theZombie->mPhaseCounter, 20, 0, TodCurves::CURVE_EASE_IN);
		}

		if (theZombie->mPhaseCounter == 30)
		{
			theZombie->PlayZombieReanim("anim_landing", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, 12.0f);
		}

		if (theZombie->mPhaseCounter == 0)
		{
			theZombie->mAltitude = 0.0f;
			theZombie->mZombiePhase = ZombiePhase::PHASE_DIGGER_STUNNED;
			theZombie->PlayZombieReanim("anim_dizzy", ReanimLoopType::REANIM_LOOP, 10, 12.0f);
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_DIGGER_TUNNELING_PAUSE_WITHOUT_AXE)
	{
		if (theZombie->mPhaseCounter == 150)
		{
			theZombie->AddAttachedReanim(23, 93, ReanimationType::REANIM_ZOMBIE_SURPRISE);
		}

		if (theZombie->mPhaseCounter == 0)
		{
			theZombie->mAltitude = -120.f;
			theZombie->mZombiePhase = ZombiePhase::PHASE_DIGGER_RISE_WITHOUT_AXE;
			theZombie->mPhaseCounter = 130;
			theZombie->PlayZombieReanim("anim_landing", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, 0.0f);

			theZombie->mApp->PlayFoley(FoleyType::FOLEY_DIRT_RISE);
			theZombie->mApp->AddTodParticle(theZombie->mPosX + 60.0f, theZombie->mPosY + 118.0f, theZombie->mRenderOrder + 1, ParticleEffect::PARTICLE_DIGGER_RISE);
			Reanimation *aDirtReanim = theZombie->mApp->AddReanimation(theZombie->mPosX + 13.0f, theZombie->mPosY + 97.0f, theZombie->mRenderOrder + 1,
			                                                           ReanimationType::REANIM_DIGGER_DIRT);
			aDirtReanim->mAnimRate = 24.0f;
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_DIGGER_RISE_WITHOUT_AXE)
	{
		if (theZombie->mPhaseCounter > 40)
		{
			theZombie->mAltitude = TodAnimateCurve(130, 40, theZombie->mPhaseCounter, -120, 20, TodCurves::CURVE_EASE_OUT);
		}
		else
		{
			theZombie->mAltitude = TodAnimateCurve(30, 0, theZombie->mPhaseCounter, 20, 0, TodCurves::CURVE_EASE_IN);
		}

		if (theZombie->mPhaseCounter == 30)
		{
			theZombie->PlayZombieReanim("anim_landing", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 12.0f);
		}

		if (theZombie->mPhaseCounter == 0)
		{
			theZombie->mAltitude = 0.0f;
			theZombie->mZombiePhase = ZombiePhase::PHASE_DIGGER_WALKING_WITHOUT_AXE;
			theZombie->StartWalkAnim(20);
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_DIGGER_STUNNED)
	{
		Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);
		if (aBodyReanim->mLoopCount > 1)
		{
			theZombie->mZombiePhase = ZombiePhase::PHASE_DIGGER_WALKING;
			theZombie->StartWalkAnim(20);
		}
	}
}

void JackInTheBoxBehaviour::UpdateActions(Zombie *theZombie)
{
	if (theZombie->mZombiePhase == ZombiePhase::PHASE_JACK_IN_THE_BOX_RUNNING)
	{
		if (theZombie->mPhaseCounter <= 0 && theZombie->mHasHead)
		{
			theZombie->mPhaseCounter = 110;
			theZombie->mZombiePhase = ZombiePhase::PHASE_JACK_IN_THE_BOX_POPPING;

			theZombie->StopZombieSound();
			theZombie->mApp->PlaySample(SOUND_BOING);
			theZombie->PlayZombieReanim("anim_pop", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 28.0f);
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_JACK_IN_THE_BOX_POPPING)
	{
		if (theZombie->mPhaseCounter == 80)
		{
			theZombie->mApp->PlayFoley(FoleyType::FOLEY_JACK_SURPRISE);
		}

		if (theZombie->mPhaseCounter <= 0)
		{
			theZombie->mApp->PlayFoley(FoleyType::FOLEY_EXPLOSION);

			// Blows up everything on the other side: hypnotized zombies and plants, or zombies if it is hypnotized
			int aPosX = theZombie->mX + theZombie->mWidth / 2;
			int aPosY = theZombie->mY + theZombie->mHeight / 2;
			int aTeam = Teams::Of(theZombie);
			theZombie->mBoard->KillAllZombiesInRadius(
			    aTeam, theZombie->mRow, aPosX, aPosY, JackInTheBoxZombieRadius, 1, true,
			    GetBit(DamageRangeFlags::DAMAGES_GROUND) | GetBit(DamageRangeFlags::DAMAGES_SUBMERGED) |
			        GetBit(DamageRangeFlags::DAMAGES_FLYING) | GetBit(DamageRangeFlags::DAMAGES_DOG) |
			        GetBit(DamageRangeFlags::DAMAGES_OFF_GROUND) | GetBit(DamageRangeFlags::DAMAGES_DYING) |
			        GetBit(DamageRangeFlags::DAMAGES_UNDERGROUND));
			theZombie->mBoard->KillAllPlantsInRadius(aTeam, aPosX, aPosY, JackInTheBoxPlantRadius);

			theZombie->mApp->AddTodParticle(aPosX, aPosY, Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_TOP, 0, 0),
			                                ParticleEffect::PARTICLE_JACKEXPLODE);
			theZombie->mBoard->ShakeBoard(4, -6);
			theZombie->DieNoLoot();

			if (theZombie->mApp->IsScaryPotterLevel())
			{
				theZombie->mBoard->mChallenge->ScaryPotterJackExplode(aPosX, aPosY);
			}
		}
	}
}

void GargantuarBehaviour::UpdateActions(Zombie *theZombie)
{
	if (theZombie->mZombiePhase == ZombiePhase::PHASE_GARGANTUAR_SMASHING)
	{
		Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);
		if (aBodyReanim->ShouldTriggerTimedEvent(0.64f))
		{

			if (theZombie->mMindControlled)
			{
				Zombie *aZombie = theZombie->FindZombieTarget();
				if (aZombie)
				{
					aZombie->TakeDamage(1500, 0U);
				}
			}
			else
			{
				Plant *aPlant = theZombie->FindPlantTarget(ZombieAttackType::ATTACKTYPE_CHEW);
				if (aPlant)
				{
					SpikesComponent *aSpikes = aPlant->GetComponent<SpikesComponent>();
					if (aSpikes && aSpikes->mSurvivesCrushing)
					{
						theZombie->TakeDamage(20, GetBit(DamageFlags::DAMAGE_SPIKE));
						aPlant->SpikeRockTakeDamage();
						if (aPlant->Health().mHealth <= 0)
						{
							theZombie->SquishAllInSquare(aPlant->mPlantCol, aPlant->mRow, ZombieAttackType::ATTACKTYPE_CHEW);
						}
					}
					else
					{
						theZombie->SquishAllInSquare(aPlant->mPlantCol, aPlant->mRow, ZombieAttackType::ATTACKTYPE_CHEW);
					}
				}
				Zombie *aZombie = theZombie->FindZombieTarget();
				if (aZombie)
				{
					aZombie->TakeDamage(1500, 0U);
				}

				if (theZombie->mApp->IsScaryPotterLevel())
				{
					int aGridX = theZombie->mBoard->PixelToGridX(theZombie->mPosX, theZombie->mPosY);
					GridItem *aScaryPot = theZombie->mBoard->GetScaryPotAt(aGridX, theZombie->mRow);
					if (aScaryPot)
					{
						theZombie->mBoard->mChallenge->ScaryPotterOpenPot(aScaryPot);
					}
				}

				if (theZombie->mApp->IsIZombieLevel())
				{
					GridItem *aBrain = theZombie->mBoard->mChallenge->IZombieGetBrainTarget(theZombie);
					if (aBrain)
					{
						theZombie->mBoard->mChallenge->IZombieSquishBrain(aBrain);
					}
				}
			}

			theZombie->mApp->PlayFoley(FoleyType::FOLEY_THUMP);
			theZombie->mBoard->ShakeBoard(0, 3);
		}

		if (aBodyReanim->mLoopCount > 0)
		{
			theZombie->mZombiePhase = ZombiePhase::PHASE_ZOMBIE_NORMAL;
			theZombie->StartWalkAnim(20);
		}

		return;
	}

	float aThrowingDistance = theZombie->mPosX - 360.0f;
	if (theZombie->mZombiePhase == ZombiePhase::PHASE_GARGANTUAR_THROWING)
	{
		Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);
		if (aBodyReanim->ShouldTriggerTimedEvent(0.74f))
		{
			theZombie->mHasObject = false;
			theZombie->ReanimShowPrefix("Zombie_imp", RENDER_GROUP_HIDDEN);
			theZombie->ReanimShowTrack("Zombie_gargantuar_whiterope", RENDER_GROUP_HIDDEN);
			theZombie->mApp->PlayFoley(FoleyType::FOLEY_SWING);

			Zombie *aZombieImp = theZombie->mBoard->AddZombie(ZombieType::ZOMBIE_IMP, theZombie->mFromWave);
			if (aZombieImp == nullptr)
				return;

			float aMinThrowDistance = 40.0f;
			if (theZombie->mBoard->StageHasRoof())
			{
				aThrowingDistance -= 180.0f;
				aMinThrowDistance = -140.0f;
			}
			if (aThrowingDistance < aMinThrowDistance)
			{
				aThrowingDistance = aMinThrowDistance;
			}
			else if (aThrowingDistance > 140.0f)
			{
				aThrowingDistance -= RandRangeFloat(0.0f, 100.0f);
			}

			aZombieImp->mPosX = theZombie->mPosX - 133.0f;
			aZombieImp->mPosY = theZombie->GetPosYBasedOnRow(theZombie->mRow);
			aZombieImp->SetRow(theZombie->mRow);
			aZombieImp->mVariant = false;
			aZombieImp->mAltitude = 88.0f;
			aZombieImp->mRenderOrder = theZombie->mRenderOrder + 1;
			aZombieImp->mZombiePhase = ZombiePhase::PHASE_IMP_GETTING_THROWN;

			aZombieImp->mScaleZombie = theZombie->mScaleZombie;
			aZombieImp->Health().mHealth *= theZombie->mScaleZombie * theZombie->mScaleZombie;
			aZombieImp->Health().mMaxHealth *= theZombie->mScaleZombie * theZombie->mScaleZombie;

			if (theZombie->mMindControlled)
			{
				aZombieImp->mPosX = theZombie->mPosX + theZombie->mWidth;
				aZombieImp->StartMindControlled();
				aZombieImp->mVelX = -3.0f;
			}
			else
			{
				aZombieImp->mVelX = 3.0f;
			}

			StatusEffects::Set<ChilledStatus>(aZombieImp, StatusEffects::GetTicks<ChilledStatus>(theZombie));
			aZombieImp->mVelZ = 0.5f * (aThrowingDistance / aZombieImp->mVelX) * THOWN_ZOMBIE_GRAVITY;
			aZombieImp->PlayZombieReanim("anim_thrown", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, 18.0f);
			aZombieImp->UpdateReanim();
			theZombie->mApp->PlayFoley(FoleyType::FOLEY_IMP);
		}

		if (aBodyReanim->mLoopCount > 0)
		{
			theZombie->mZombiePhase = ZombiePhase::PHASE_ZOMBIE_NORMAL;
			theZombie->StartWalkAnim(20);
		}

		return;
	}

	if (theZombie->IsImmobilizied() || !theZombie->mHasHead)
		return;

	if (theZombie->mHasObject && theZombie->Health().mHealth < theZombie->Health().mMaxHealth / 2 && aThrowingDistance > 40.0f)
	{
		theZombie->mZombiePhase = ZombiePhase::PHASE_GARGANTUAR_THROWING;
		theZombie->PlayZombieReanim("anim_throw", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 24.0f);
		return;
	}

	bool doSmash = false;
	if (theZombie->FindPlantTarget(ZombieAttackType::ATTACKTYPE_CHEW))
	{
		doSmash = true;
	}
	else if (theZombie->mApp->IsScaryPotterLevel())
	{
		int aGridX = theZombie->mBoard->PixelToGridX(theZombie->mPosX, theZombie->mPosY);
		if (theZombie->mBoard->GetScaryPotAt(aGridX, theZombie->mRow))
		{
			doSmash = true;
		}
	}
	else if (theZombie->mApp->IsIZombieLevel())
	{
		if (theZombie->mBoard->mChallenge->IZombieGetBrainTarget(theZombie))
		{
			doSmash = true;
		}
	}
	else
		doSmash = theZombie->FindZombieTarget();

	if (doSmash)
	{
		theZombie->mZombiePhase = ZombiePhase::PHASE_GARGANTUAR_SMASHING;
		theZombie->mApp->PlayFoley(FoleyType::FOLEY_LOW_GROAN);
		theZombie->PlayZombieReanim("anim_smash", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 16.0f);
	}
}

void BobsledBehaviour::UpdateActions(Zombie *theZombie)
{
	if (theZombie->mZombiePhase == ZombiePhase::PHASE_BOBSLED_CRASHING)
	{
		if (theZombie->mPhaseCounter == 0)
		{
			theZombie->mZombiePhase = ZombiePhase::PHASE_ZOMBIE_NORMAL;
			if (theZombie->GetBobsledPosition() == 0)
			{
				for (int i = 0; i < NUM_BOBSLED_FOLLOWERS; i++)
				{
					Zombie *aZombie = theZombie->mBoard->ZombieGet(theZombie->mFollowerZombieID[i]);
					aZombie->mRelatedZombieID = ZombieID::ZOMBIEID_NULL;
					theZombie->mFollowerZombieID[i] = ZombieID::ZOMBIEID_NULL;
					aZombie->PickRandomSpeed();
				}
				theZombie->PickRandomSpeed();
			}
		}
		return;
	}

	if (theZombie->mZombiePhase == ZombiePhase::PHASE_BOBSLED_SLIDING)
	{
		if (theZombie->mPhaseCounter == 0)
		{
			theZombie->mZombiePhase = ZombiePhase::PHASE_BOBSLED_BOARDING;
			theZombie->PlayZombieReanim("anim_jump", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, 20.0f);
		}
	}
	else
	{
		if (theZombie->mZombiePhase != ZombiePhase::PHASE_BOBSLED_BOARDING)
			return;

		Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);
		int aCounter = aBodyReanim->mAnimTime * 50.0f;
		int aPosition = theZombie->GetBobsledPosition();
		if (aPosition == 1 || aPosition == 3)
		{
			theZombie->mAltitude = TodAnimateCurveFloat(0, 50, aCounter, 8.0f, 18.0f, TodCurves::CURVE_LINEAR);
		}
		else
		{
			theZombie->mAltitude = TodAnimateCurveFloat(0, 50, aCounter, -9.0f, 18.0f, TodCurves::CURVE_LINEAR);
		}
	}

	theZombie->mBoard->mIceTimer[theZombie->mRow] = std::max(500, theZombie->mBoard->mIceTimer[theZombie->mRow]);
	if (theZombie->mPosX + 10.0f < theZombie->mBoard->mIceMinX[theZombie->mRow] && theZombie->GetBobsledPosition() == 0)
	{
		theZombie->TakeDamage(6, GetBit(DamageFlags::DAMAGE_DOESNT_CAUSE_FLASH));
	}
}

void ZamboniBehaviour::UpdateActions(Zombie *theZombie)
{
	if (theZombie->mPosX > 400.0f && !theZombie->mFlatTires)
	{
		theZombie->mVelX = TodAnimateCurveFloat(700, 300, theZombie->mPosX, 0.25f, 0.05f, TodCurves::CURVE_LINEAR);
	}
	else if (theZombie->mFlatTires && theZombie->mVelX > 0.0005f)
	{
		theZombie->mVelX -= 0.0005f;
	}

	int anIceX = theZombie->mPosX + 118;
	if (theZombie->mBoard->StageHasRoof())
	{
		anIceX = std::max(anIceX, 500);
	}
	else
	{
		anIceX = std::max(anIceX, 25);
	}
	if (anIceX < theZombie->mBoard->mIceMinX[theZombie->mRow])
	{
		theZombie->mBoard->mIceMinX[theZombie->mRow] = anIceX;
	}
	if (anIceX < 800)
	{
		theZombie->mBoard->mIceTimer[theZombie->mRow] = 3000;
		if (theZombie->mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BOBSLED_BONANZA)
		{
			theZombie->mBoard->mIceTimer[theZombie->mRow] = INT_MAX;
		}
	}
}

void LadderBehaviour::UpdateActions(Zombie *theZombie)
{
	if (theZombie->mMindControlled || !theZombie->mHasHead || theZombie->IsDeadOrDying())
		return;

	if (theZombie->mZombiePhase == ZombiePhase::PHASE_LADDER_CARRYING && theZombie->mZombieHeight == ZombieHeight::HEIGHT_ZOMBIE_NORMAL)
	{
		if (theZombie->FindPlantTarget(ZombieAttackType::ATTACKTYPE_LADDER))
		{
			theZombie->StopEating();
			theZombie->mZombiePhase = ZombiePhase::PHASE_LADDER_PLACING;
			theZombie->PlayZombieReanim("anim_placeladder", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 10, 24.0f);
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_LADDER_PLACING)
	{
		Reanimation *aBodyReanim = theZombie->mApp->ReanimationTryToGet(theZombie->mBodyReanimID);
		if (aBodyReanim->mLoopCount > 0)
		{
			Plant *aPlant = theZombie->FindPlantTarget(ZombieAttackType::ATTACKTYPE_LADDER);
			if (aPlant)
			{
				theZombie->mBoard->AddALadder(aPlant->mPlantCol, aPlant->mRow);
				theZombie->mApp->PlaySample(SOUND_LADDER_ZOMBIE);
				theZombie->mZombieHeight = ZombieHeight::HEIGHT_UP_LADDER;
				theZombie->mUseLadderCol = aPlant->mPlantCol;
				theZombie->DetachShield();
			}
			else
			{
				theZombie->mZombiePhase = ZombiePhase::PHASE_LADDER_CARRYING;
				theZombie->StartWalkAnim(0);
			}
		}
	}
}

void YetiBehaviour::UpdateActions(Zombie *theZombie)
{
	if (theZombie->mMindControlled || !theZombie->mHasHead || theZombie->IsDeadOrDying())
		return;

	if (theZombie->mZombiePhase == ZombiePhase::PHASE_ZOMBIE_NORMAL && theZombie->mPhaseCounter == 0)
	{
		theZombie->mZombiePhase = ZombiePhase::PHASE_YETI_RUNNING;
		theZombie->mHasObject = false;
		theZombie->PickRandomSpeed();
	}
}

void DancerBehaviour::UpdateActions(Zombie *theZombie)
{
	if (theZombie->mIsEating)
		return;

	if (theZombie->mSummonCounter > 0)
	{
		theZombie->mSummonCounter--;
		if (theZombie->mSummonCounter == 0)
		{
			if (theZombie->GetDancerFrame() == 12 && theZombie->mHasHead && theZombie->mPosX < 700.0f)
			{
				theZombie->mZombiePhase = ZombiePhase::PHASE_DANCER_SNAPPING_FINGERS_WITH_LIGHT;
				theZombie->PlayZombieReanim("anim_point", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 24.0f);
			}
			else
			{
				theZombie->mSummonCounter = 1;
			}
		}
	}

	if (theZombie->mZombiePhase == ZombiePhase::PHASE_DANCER_DANCING_IN)
	{
		if (theZombie->mHasHead && theZombie->mPhaseCounter == 0)
		{
			theZombie->mZombiePhase = ZombiePhase::PHASE_DANCER_SNAPPING_FINGERS;
			theZombie->PlayZombieReanim("anim_point", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 24.0f);
			theZombie->PickRandomSpeed();
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_DANCER_SNAPPING_FINGERS ||
	         theZombie->mZombiePhase == ZombiePhase::PHASE_DANCER_SNAPPING_FINGERS_WITH_LIGHT)
	{
		Reanimation *aBodyReanim = theZombie->mApp->ReanimationTryToGet(theZombie->mBodyReanimID);
		if (aBodyReanim->mLoopCount > 0)
		{
			if (theZombie->mZombiePhase == ZombiePhase::PHASE_DANCER_SNAPPING_FINGERS && theZombie->mBoard->CountZombiesOnScreen() <= 15)
			{
				theZombie->mApp->PlayFoley(FoleyType::FOLEY_DANCER);
			}

			theZombie->SummonBackupDancers();
			theZombie->mZombiePhase = ZombiePhase::PHASE_DANCER_SNAPPING_FINGERS_HOLD;
			theZombie->mPhaseCounter = 200;
		}
	}
	else
	{
		if (theZombie->mZombiePhase == ZombiePhase::PHASE_DANCER_SNAPPING_FINGERS_HOLD)
		{
			if (theZombie->mPhaseCounter != 0)
				return;

			theZombie->mZombiePhase = ZombiePhase::PHASE_DANCER_DANCING_LEFT;
			theZombie->PlayZombieReanim("anim_walk", ReanimLoopType::REANIM_LOOP, 20, 0.0f);
		}

		ZombiePhase aDancerPhase = theZombie->GetDancerPhase();
		if (aDancerPhase != theZombie->mZombiePhase)
		{
			switch (aDancerPhase)
			{
			case ZombiePhase::PHASE_DANCER_DANCING_LEFT:
				theZombie->mZombiePhase = aDancerPhase;
				theZombie->PlayZombieReanim("anim_walk", ReanimLoopType::REANIM_LOOP, 10, 0.0f);
				break;

			case ZombiePhase::PHASE_DANCER_WALK_TO_RAISE:
				theZombie->mZombiePhase = aDancerPhase;
				theZombie->PlayZombieReanim("anim_armraise", ReanimLoopType::REANIM_LOOP, 10, 18.0f);
				theZombie->mApp->ReanimationTryToGet(theZombie->mBodyReanimID)->mAnimTime = 0.6f;
				break;

			case ZombiePhase::PHASE_DANCER_RAISE_LEFT_1:
			case ZombiePhase::PHASE_DANCER_RAISE_RIGHT_1:
			case ZombiePhase::PHASE_DANCER_RAISE_LEFT_2:
			case ZombiePhase::PHASE_DANCER_RAISE_RIGHT_2:
				theZombie->mZombiePhase = aDancerPhase;
				theZombie->PlayZombieReanim("anim_armraise", ReanimLoopType::REANIM_LOOP, 10, 18.0f);
				break;
			}
		}

		if (theZombie->mHasHead && theZombie->mSummonCounter == 0 && theZombie->NeedsMoreBackupDancers())
		{
			theZombie->mSummonCounter = 100;
		}
	}
}

void BackupDancerBehaviour::UpdateActions(Zombie *theZombie)
{
	if (theZombie->mIsEating)
		return;

	if (theZombie->mZombiePhase == ZombiePhase::PHASE_DANCER_RISING)
	{
		theZombie->mAltitude =
		    TodAnimateCurve(150, 0, theZombie->mPhaseCounter, ZOMBIE_BACKUP_DANCER_RISE_HEIGHT, 0, TodCurves::CURVE_LINEAR);

		if (theZombie->mPhaseCounter != 0)
			return;

		if (theZombie->IsOnHighGround())
		{
			theZombie->mAltitude = HIGH_GROUND_HEIGHT;
		}
	}

	ZombiePhase aDancerPhase = theZombie->GetDancerPhase();
	if (aDancerPhase != theZombie->mZombiePhase)
	{
		switch (aDancerPhase)
		{
		case ZombiePhase::PHASE_DANCER_DANCING_LEFT:
			theZombie->mZombiePhase = aDancerPhase;
			theZombie->PlayZombieReanim("anim_walk", ReanimLoopType::REANIM_LOOP, 10, 0.0f);
			break;

		case ZombiePhase::PHASE_DANCER_WALK_TO_RAISE:
			theZombie->mZombiePhase = aDancerPhase;
			theZombie->PlayZombieReanim("anim_armraise", ReanimLoopType::REANIM_LOOP, 10, 18.0f);
			theZombie->mApp->ReanimationTryToGet(theZombie->mBodyReanimID)->mAnimTime = 0.6f;
			break;

		case ZombiePhase::PHASE_DANCER_RAISE_LEFT_1:
		case ZombiePhase::PHASE_DANCER_RAISE_RIGHT_1:
		case ZombiePhase::PHASE_DANCER_RAISE_LEFT_2:
		case ZombiePhase::PHASE_DANCER_RAISE_RIGHT_2:
			theZombie->mZombiePhase = aDancerPhase;
			theZombie->PlayZombieReanim("anim_armraise", ReanimLoopType::REANIM_LOOP, 10, 18.0f);
			break;
		}
	}
}

void ImpBehaviour::UpdateActions(Zombie *theZombie)
{
	if (theZombie->mZombiePhase == ZombiePhase::PHASE_IMP_GETTING_THROWN)
	{
		theZombie->mVelZ -= THOWN_ZOMBIE_GRAVITY;
		theZombie->mAltitude += theZombie->mVelZ;
		theZombie->mPosX -= theZombie->mVelX;

		float aDiffY = theZombie->GetPosYBasedOnRow(theZombie->mRow) - theZombie->mPosY;
		theZombie->mPosY += aDiffY;
		theZombie->mAltitude += aDiffY;
		if (theZombie->mAltitude <= 0.0f)
		{
			theZombie->mAltitude = 0.0f;
			theZombie->mZombiePhase = ZombiePhase::PHASE_IMP_LANDING;
			theZombie->PlayZombieReanim("anim_land", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, 24.0f);
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_IMP_LANDING)
	{
		Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);
		if (aBodyReanim->mLoopCount > 0)
		{
			theZombie->mZombiePhase = ZombiePhase::PHASE_ZOMBIE_NORMAL;
			theZombie->StartWalkAnim(0);
		}
	}
}

void PeaHeadBehaviour::UpdateActions(Zombie *theZombie)
{
	if (!theZombie->mHasHead)
		return;

	if (theZombie->mPhaseCounter == 35)
	{
		Reanimation *aHeadReanim = theZombie->mApp->ReanimationGet(theZombie->mSpecialHeadReanimID);
		aHeadReanim->PlayReanim("anim_shooting", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 35.0f);
	}
	else if (theZombie->mPhaseCounter == 0)
	{
		Reanimation *aHeadReanim = theZombie->mApp->ReanimationGet(theZombie->mSpecialHeadReanimID);
		aHeadReanim->PlayReanim("anim_head_idle", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 15.0f);
		theZombie->mApp->PlayFoley(FoleyType::FOLEY_THROW);

		Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);
		int aTrackIndex = aBodyReanim->FindTrackIndex("anim_head1");
		ReanimatorTransform aTransform;
		aBodyReanim->GetCurrentTransform(aTrackIndex, &aTransform);

		float aOriginX = theZombie->mPosX + aTransform.mTransX - 9.0f;
		float aOriginY = theZombie->mPosY + aTransform.mTransY + 6.0f - theZombie->mAltitude;
		Projectile *aProjectile;
		if (theZombie->mMindControlled)
		{
			aOriginX += 90.0f * theZombie->mScaleZombie;
			aProjectile = theZombie->mBoard->AddProjectile(aOriginX, aOriginY, theZombie->mRenderOrder, theZombie->mRow, ProjectileType::PROJECTILE_PEA);
		}
		else
		{
			aProjectile =
			    theZombie->mBoard->AddProjectile(aOriginX, aOriginY, theZombie->mRenderOrder, theZombie->mRow, ProjectileType::PROJECTILE_ZOMBIE_PEA);
			aProjectile->mMotionType = ProjectileMotion::MOTION_BACKWARDS;
		}
		// The pea hits whatever is hostile to theZombie zombie's team
		Teams::Set(aProjectile, Teams::Of(theZombie));
		aProjectile->mDamageRangeFlags = GetBit(DamageRangeFlags::DAMAGES_GROUND);

		theZombie->mPhaseCounter = 150;
	}
}

void JalapenoHeadBehaviour::UpdateActions(Zombie *theZombie)
{
	if (!theZombie->mHasHead)
		return;

	if (theZombie->mPhaseCounter == 0)
	{
		theZombie->mApp->PlayFoley(FoleyType::FOLEY_JALAPENO_IGNITE);
		theZombie->mApp->PlayFoley(FoleyType::FOLEY_JUICY);
		theZombie->mBoard->DoFwoosh(theZombie->mRow);
		theZombie->mBoard->ShakeBoard(3, -4);

		if (theZombie->mMindControlled)
		{
			theZombie->BurnRow(theZombie->mRow);
		}
		else
		{
			Plant *aPlant = nullptr;
			while (theZombie->mBoard->IteratePlants(aPlant))
			{
				//Rect aPlantRect = aPlant->GetPlantRect();
				if (aPlant->mRow == theZombie->mRow && !aPlant->NotOnGround())
				{
					theZombie->mBoard->mPlantsEaten++;
					aPlant->Die();
				}
			}
		}
		theZombie->DieNoLoot();
	}
}

void GatlingHeadBehaviour::UpdateActions(Zombie *theZombie)
{
	if (!theZombie->mHasHead)
		return;

	if (theZombie->mPhaseCounter == 100)
	{
		Reanimation *aHeadReanim = theZombie->mApp->ReanimationGet(theZombie->mSpecialHeadReanimID);
		aHeadReanim->PlayReanim("anim_shooting", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 38.0f);
	}
	else if (theZombie->mPhaseCounter == 18 || theZombie->mPhaseCounter == 35 || theZombie->mPhaseCounter == 51 || theZombie->mPhaseCounter == 68)
	{
		theZombie->mApp->PlayFoley(FoleyType::FOLEY_THROW);

		Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);
		int aTrackIndex = aBodyReanim->FindTrackIndex("anim_head1");
		ReanimatorTransform aTransform;
		aBodyReanim->GetCurrentTransform(aTrackIndex, &aTransform);

		float aOriginX = theZombie->mPosX + aTransform.mTransX - 9.0f;
		float aOriginY = theZombie->mPosY + aTransform.mTransY + 6.0f - theZombie->mAltitude;

		Projectile *aProjectile;
		if (theZombie->mMindControlled)
		{
			aOriginX += 90.0f * theZombie->mScaleZombie;
			aProjectile = theZombie->mBoard->AddProjectile(aOriginX, aOriginY, theZombie->mRenderOrder, theZombie->mRow, ProjectileType::PROJECTILE_PEA);
		}
		else
		{
			aProjectile =
			    theZombie->mBoard->AddProjectile(aOriginX, aOriginY, theZombie->mRenderOrder, theZombie->mRow, ProjectileType::PROJECTILE_ZOMBIE_PEA);
			aProjectile->mMotionType = ProjectileMotion::MOTION_BACKWARDS;
		}
		// The pea hits whatever is hostile to theZombie zombie's team
		Teams::Set(aProjectile, Teams::Of(theZombie));
		aProjectile->mDamageRangeFlags = GetBit(DamageRangeFlags::DAMAGES_GROUND);
	}
	else if (theZombie->mPhaseCounter == 0)
	{
		Reanimation *aHeadReanim = theZombie->mApp->ReanimationGet(theZombie->mSpecialHeadReanimID);
		aHeadReanim->PlayReanim("anim_head_idle", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 15.0f);
		theZombie->mPhaseCounter = 150;
	}
}

void SquashHeadBehaviour::UpdateActions(Zombie *theZombie)
{
	if (theZombie->mHasHead && theZombie->mIsEating && theZombie->mZombiePhase == ZombiePhase::PHASE_SQUASH_PRE_LAUNCH)
	{
		theZombie->StopEating();
		theZombie->PlayZombieReanim("anim_idle", ReanimLoopType::REANIM_LOOP, 20, 12.0f);
		theZombie->mHasHead = false;

		Reanimation *aHeadReanim = theZombie->mApp->ReanimationGet(theZombie->mSpecialHeadReanimID);
		aHeadReanim->PlayReanim("anim_jumpup", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 24.0f);
		aHeadReanim->mRenderOrder = theZombie->mRenderOrder + 1;
		aHeadReanim->SetPosition(theZombie->mPosX + 6.0f, theZombie->mPosY - 21.0f);

		Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);
		ReanimatorTrackInstance *aTrackInstance = aBodyReanim->GetTrackInstanceByName("anim_head1");
		AttachmentDetach(aTrackInstance->mAttachmentID);
		aHeadReanim->OverrideScale(0.75f, 0.75f);
		aHeadReanim->mOverlayMatrix.m10 = 0.0f;

		theZombie->mZombiePhase = ZombiePhase::PHASE_SQUASH_RISING;
		theZombie->mPhaseCounter = 95;
	}

	if (theZombie->mZombiePhase == ZombiePhase::PHASE_SQUASH_RISING)
	{
		int aDestX = theZombie->mBoard->GridToPixelX(theZombie->mBoard->PixelToGridXKeepOnBoard(theZombie->mX, theZombie->mY), theZombie->mRow);

		if (theZombie->mMindControlled)
		{
			Zombie *aZombie = theZombie->FindZombieTarget();
			if (aZombie)
			{
				aDestX = aZombie->ZombieTargetLeadX(0.0f);
			}
			else
			{
				aDestX += 90.0f * theZombie->mScaleZombie;
			}
		}

		int aPosX = TodAnimateCurve(50, 20, theZombie->mPhaseCounter, 0, aDestX - theZombie->mPosX, TodCurves::CURVE_EASE_IN_OUT);
		int aPosY = TodAnimateCurve(50, 20, theZombie->mPhaseCounter, 0, -20, TodCurves::CURVE_EASE_IN_OUT);

		Reanimation *aHeadReanim = theZombie->mApp->ReanimationGet(theZombie->mSpecialHeadReanimID);
		aHeadReanim->SetPosition(theZombie->mPosX + aPosX + 6.0f, theZombie->mPosY + aPosY - 21.0f);

		if (theZombie->mPhaseCounter == 0)
		{
			aHeadReanim = theZombie->mApp->ReanimationGet(theZombie->mSpecialHeadReanimID);
			aHeadReanim->PlayReanim("anim_jumpdown", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, 60.0f);
			theZombie->mZombiePhase = ZombiePhase::PHASE_SQUASH_FALLING;
			theZombie->mPhaseCounter = 10;
		}
	}

	if (theZombie->mZombiePhase == ZombiePhase::PHASE_SQUASH_FALLING)
	{
		int aPosY = TodAnimateCurve(10, 0, theZombie->mPhaseCounter, -20, 74, TodCurves::CURVE_LINEAR);
		int aDestX = theZombie->mBoard->GridToPixelX(theZombie->mBoard->PixelToGridXKeepOnBoard(theZombie->mX, theZombie->mY), theZombie->mRow);

		if (theZombie->mMindControlled)
		{
			Zombie *aZombie = theZombie->FindZombieTarget();
			if (aZombie)
			{
				aDestX = aZombie->ZombieTargetLeadX(0.0f);
			}
			else
			{
				aDestX += 90.0f * theZombie->mScaleZombie;
			}
		}

		Reanimation *aHeadReanim = theZombie->mApp->ReanimationGet(theZombie->mSpecialHeadReanimID);
		aHeadReanim->SetPosition(theZombie->mPosX + 6.0f + aDestX - theZombie->mPosX, theZombie->mPosY - 21.0f + aPosY);

		if (theZombie->mPhaseCounter == 2)
		{

			if (theZombie->mMindControlled)
			{
				Rect aAttackRect(aDestX - 73, theZombie->mPosY + 4, 65, 90);

				Zombie *aZombie = nullptr;
				while (theZombie->mBoard->IterateZombies(aZombie))
				{
					if ((aZombie->mRow == theZombie->mRow || aZombie->mZombieType == ZombieType::ZOMBIE_BOSS) &&
					    aZombie->EffectedByDamage(GetBit(DamageRangeFlags::DAMAGES_GROUND) |
					                              GetBit(DamageRangeFlags::DAMAGES_SUBMERGED) |
					                              GetBit(DamageRangeFlags::DAMAGES_DOG)))
					{
						Rect aZombieRect = aZombie->GetZombieRect();
						if (GetRectOverlap(aAttackRect, aZombieRect) >
						    (aZombie->mZombieType == ZombieType::ZOMBIE_FOOTBALL ? -20 : 0))
						{
							aZombie->TakeDamage(1800, GetBit(DamageFlags::DAMAGE_HITS_SHIELD_AND_BODY) |
							                              GetBit(DamageFlags::DAMAGE_DOESNT_LEAVE_BODY));
						}
					}
				}
			}
			else
			{
				theZombie->SquishAllInSquare(theZombie->mBoard->PixelToGridXKeepOnBoard(theZombie->mX, theZombie->mY), theZombie->mRow, ZombieAttackType::ATTACKTYPE_CHEW);
			}
		}

		if (theZombie->mPhaseCounter == 0)
		{
			theZombie->mZombiePhase = ZombiePhase::PHASE_SQUASH_DONE_FALLING;
			theZombie->mPhaseCounter = 100;

			theZombie->mBoard->ShakeBoard(1, 4);
			theZombie->mApp->PlayFoley(FoleyType::FOLEY_THUMP);
		}
	}

	if (theZombie->mZombiePhase == ZombiePhase::PHASE_SQUASH_DONE_FALLING && theZombie->mPhaseCounter == 0)
	{
		Reanimation *aHeadReanim = theZombie->mApp->ReanimationGet(theZombie->mSpecialHeadReanimID);
		aHeadReanim->ReanimationDie();
		theZombie->mSpecialHeadReanimID = ReanimationID::REANIMATIONID_NULL;

		theZombie->TakeDamage(1800, GetBit(DamageFlags::DAMAGE_DOESNT_CAUSE_FLASH) | GetBit(DamageFlags::DAMAGE_BYPASSES_SHIELD));
	}
}

void BossBehaviour::UpdatePlaying(Zombie *theZombie)
{
	Reanimation *aBodyReanim = theZombie->mApp->ReanimationTryToGet(theZombie->mBodyReanimID);
	if (theZombie->mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO)
	{
		if (aBodyReanim->ShouldTriggerTimedEvent(0.24f) || aBodyReanim->ShouldTriggerTimedEvent(0.79f))
		{
			theZombie->mApp->PlayFoley(FoleyType::FOLEY_THUMP);
			theZombie->mBoard->ShakeBoard(1, 4);
		}
		return;
	}

	Reanimation *aHeadReanim = theZombie->mApp->ReanimationGet(theZombie->mSpecialHeadReanimID);
	theZombie->UpdateBossFireball();
	if (!theZombie->IsFrozen())
	{
		if (theZombie->mSummonCounter > 0)
		{
			theZombie->mSummonCounter--;
		}
		if (theZombie->mBossBungeeCounter > 0)
		{
			theZombie->mBossBungeeCounter--;
		}
		if (theZombie->mBossStompCounter > 0)
		{
			theZombie->mBossStompCounter--;
		}
		if (theZombie->mBossHeadCounter > 0)
		{
			theZombie->mBossHeadCounter--;
		}

		if (theZombie->IsChilled())
		{
			aHeadReanim->mAnimRate = 6.0f;
		}
		else if (aHeadReanim->mAnimRate == 0.0f)
		{
			aHeadReanim->mAnimRate = 12.0f;
		}
	}
	else
	{
		aHeadReanim->mAnimRate = 0.0f;
	}

	if (theZombie->mZombiePhase == ZombiePhase::PHASE_BOSS_ENTER)
	{
		theZombie->BossPlayIdle();
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_BOSS_IDLE)
	{
		if (theZombie->Health().mHealth == 1)
		{
			theZombie->PlayDeathAnim(0U);
			return;
		}

		if (theZombie->mPhaseCounter > 0)
			return;

		int aDamageIndex = theZombie->GetBodyDamageIndex();
		if (aDamageIndex != theZombie->mBossMode)
		{
			theZombie->mBossMode = aDamageIndex;
			if (theZombie->mBossMode == 1)
			{
				theZombie->BossBungeeAttack();
			}
			else
			{
				theZombie->BossRVAttack();
			}
		}
		else if (theZombie->mBossStompCounter == 0)
		{
			theZombie->BossStompAttack();
		}
		else if (theZombie->mBossBungeeCounter == 0)
		{
			if (Rand(theZombie->mApp->IsAdventureMode() ? 4 : 2) == 0)
			{
				theZombie->mBossBungeeCounter = RandRangeInt(4000, 5000);
				theZombie->BossRVAttack();
			}
			else
			{
				theZombie->BossBungeeAttack();
			}
		}
		else if (theZombie->mBossHeadCounter == 0)
		{
			theZombie->BossHeadAttack();
		}
		else if (theZombie->mSummonCounter == 0)
		{
			theZombie->BossSpawnAttack();
		}
		else
		{
			theZombie->mPhaseCounter = RandRangeInt(100, 200);
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_BOSS_SPAWNING)
	{
		if (aBodyReanim->ShouldTriggerTimedEvent(0.6f))
		{
			theZombie->BossSpawnContact();
		}

		if (aBodyReanim->mLoopCount > 0)
		{
			theZombie->BossPlayIdle();
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_BOSS_STOMPING)
	{
		float aTrigger = 0.5f;
		if (theZombie->mTargetRow >= 2)
		{
			aTrigger = 0.55f;
		}
		if (aBodyReanim->ShouldTriggerTimedEvent(aTrigger))
		{
			theZombie->BossStompContact();
		}

		if (aBodyReanim->mLoopCount > 0)
		{
			theZombie->BossPlayIdle();
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_BOSS_BUNGEES_ENTER)
	{
		if (aBodyReanim->ShouldTriggerTimedEvent(0.4f))
		{
			theZombie->BossBungeeSpawn();
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_BOSS_BUNGEES_DROP)
	{
		if (theZombie->BossAreBungeesDone())
		{
			theZombie->BossBungeeLeave();
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_BOSS_BUNGEES_LEAVE)
	{
		if (aBodyReanim->mLoopCount > 0)
		{
			theZombie->BossPlayIdle();
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_BOSS_DROP_RV)
	{
		if (aBodyReanim->ShouldTriggerTimedEvent(0.65f))
		{
			theZombie->BossRVLanding();
		}

		if (aBodyReanim->mLoopCount > 0)
		{
			theZombie->BossPlayIdle();
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_BOSS_HEAD_ENTER)
	{
		if (theZombie->GetBodyDamageIndex() == 2 && aBodyReanim->ShouldTriggerTimedEvent(0.37f))
		{
			theZombie->ApplyBossSmokeParticles(true);
		}

		if (aBodyReanim->ShouldTriggerTimedEvent(0.55f))
		{
			theZombie->mApp->PlayFoley(FoleyType::FOLEY_HYDRAULIC);
		}

		if (aBodyReanim->mLoopCount > 0)
		{
			theZombie->mZombiePhase = ZombiePhase::PHASE_BOSS_HEAD_IDLE_BEFORE_SPIT;
			theZombie->PlayZombieReanim("anim_head_idle", ReanimLoopType::REANIM_LOOP, 0, 12.0f);
			theZombie->mPhaseCounter = 500;
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_BOSS_HEAD_IDLE_BEFORE_SPIT)
	{
		if (theZombie->Health().mHealth == 1)
		{
			theZombie->BossStartDeath();
		}
		else if (theZombie->mPhaseCounter == 0)
		{
			theZombie->BossHeadSpit();
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_BOSS_HEAD_SPIT)
	{
		if (aBodyReanim->ShouldTriggerTimedEvent(0.37f))
		{
			theZombie->BossHeadSpitEffect();
		}

		if (aBodyReanim->ShouldTriggerTimedEvent(0.42f))
		{
			theZombie->BossHeadSpitContact();
		}

		if (aBodyReanim->mLoopCount > 0)
		{
			aHeadReanim = theZombie->mApp->ReanimationTryToGet(theZombie->mSpecialHeadReanimID);
			aHeadReanim->PlayReanim("anim_idle", ReanimLoopType::REANIM_LOOP, 20, 18.0f);
			theZombie->mZombiePhase = ZombiePhase::PHASE_BOSS_HEAD_IDLE_AFTER_SPIT;
			theZombie->PlayZombieReanim("anim_head_idle", ReanimLoopType::REANIM_LOOP, 0, 12.0f);
			theZombie->mPhaseCounter = 300;
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_BOSS_HEAD_IDLE_AFTER_SPIT)
	{
		if (theZombie->Health().mHealth == 1)
		{
			theZombie->BossStartDeath();
		}
		else if (theZombie->mPhaseCounter == 0)
		{
			theZombie->mZombiePhase = ZombiePhase::PHASE_BOSS_HEAD_LEAVE;
			theZombie->PlayZombieReanim("anim_head_leave", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, 12.0f);
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_BOSS_HEAD_LEAVE)
	{
		if (aBodyReanim->ShouldTriggerTimedEvent(0.23f))
		{
			theZombie->RemoveComponent<ChilledStatus>();
			theZombie->UpdateAnimSpeed();
		}

		if (aBodyReanim->ShouldTriggerTimedEvent(0.48f) || aBodyReanim->ShouldTriggerTimedEvent(0.8f))
		{
			theZombie->mApp->PlayFoley(FoleyType::FOLEY_THUMP);
		}

		if (aBodyReanim->mLoopCount > 0)
		{
			theZombie->ApplyBossSmokeParticles(false);
			theZombie->BossPlayIdle();
		}
	}
	else
	{
		TOD_ASSERT(false);
	}
}

void BungeeBehaviour::UpdateAlways(Zombie *theZombie)
{
	if (theZombie->IsDeadOrDying() || theZombie->IsImmobilizied())
		return;

	if (theZombie->mZombiePhase == ZombiePhase::PHASE_BUNGEE_DIVING || theZombie->mZombiePhase == ZombiePhase::PHASE_BUNGEE_DIVING_SCREAMING)
	{
		float aOldAltitude = theZombie->mAltitude;
		theZombie->mAltitude -= 8.0f;
		if (theZombie->mAltitude <= BUNGEE_ZOMBIE_HEIGHT - 404.0f && aOldAltitude > BUNGEE_ZOMBIE_HEIGHT - 404.0f &&
		    theZombie->mRelatedZombieID == ZombieID::ZOMBIEID_NULL)
		{
			theZombie->mApp->PlayFoley(FoleyType::FOLEY_GRASSSTEP);
		}

		theZombie->BungeeLanding();
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_BUNGEE_AT_BOTTOM)
	{
		if (theZombie->mPhaseCounter <= 0)
		{
			theZombie->BungeeStealTarget();
			theZombie->mZombiePhase = ZombiePhase::PHASE_BUNGEE_GRABBING;
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_BUNGEE_GRABBING)
	{
		Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);
		if (aBodyReanim->mLoopCount > 0)
		{
			theZombie->BungeeLiftTarget();
			theZombie->mZombiePhase = ZombiePhase::PHASE_BUNGEE_RISING;
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_BUNGEE_HIT_OUCHY)
	{
		if (theZombie->mPhaseCounter <= 0)
		{
			theZombie->DieWithLoot();
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_BUNGEE_RISING)
	{
		theZombie->mAltitude += 8.0f;
		if (theZombie->mAltitude >= 600.0f)
		{
			theZombie->DieNoLoot();
		}
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_BUNGEE_CUTSCENE)
	{
		theZombie->mAltitude = TodAnimateCurve(200, 0, theZombie->mPhaseCounter, 40, 0, TodCurves::CURVE_SIN_WAVE);
		if (theZombie->mPhaseCounter <= 0)
		{
			theZombie->mPhaseCounter = 200;
		}
	}

	theZombie->mX = (int)theZombie->mPosX;
	theZombie->mY = (int)theZombie->mPosY;
}

void PogoBehaviour::UpdateAlways(Zombie *theZombie)
{
	if (theZombie->IsDeadOrDying() || theZombie->IsImmobilizied() || !theZombie->IsBouncingPogo() || theZombie->mZombieHeight == ZombieHeight::HEIGHT_IN_TO_CHIMNEY)
		return;

	float aHeight = 40.0f;
	if (theZombie->mZombiePhase >= ZombiePhase::PHASE_POGO_HIGH_BOUNCE_1 && theZombie->mZombiePhase <= ZombiePhase::PHASE_POGO_HIGH_BOUNCE_6)
	{
		aHeight = 50.0f + 20.0f * (theZombie->mZombiePhase - ZombiePhase::PHASE_POGO_HIGH_BOUNCE_1);
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_POGO_FORWARD_BOUNCE_2)
	{
		aHeight = 90.0f;
	}
	else if (theZombie->mZombiePhase == ZombiePhase::PHASE_POGO_FORWARD_BOUNCE_7)
	{
		aHeight = 170.0f;
	}
	theZombie->mAltitude = TodAnimateCurveFloat(POGO_BOUNCE_TIME, 0, theZombie->mPhaseCounter, 9.0f, aHeight + 9.0f,
	                                            TodCurves::CURVE_BOUNCE_SLOW_MIDDLE);
	theZombie->mFrame = ClampInt(3 - theZombie->mAltitude / 3, 0, 3);

	if (theZombie->mPhaseCounter == 7)
	{
		Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);
		aBodyReanim->mAnimTime = 0.0f;
		aBodyReanim->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD;
	}
	if (theZombie->IsOnBoard() && theZombie->mPhaseCounter == 5)
	{
		theZombie->mApp->PlayFoley(FoleyType::FOLEY_POGO_ZOMBIE);
	}

	if (theZombie->mZombieHeight == ZombieHeight::HEIGHT_UP_TO_HIGH_GROUND)
	{
		theZombie->mAltitude += HIGH_GROUND_HEIGHT;
		theZombie->mZombieHeight = ZombieHeight::HEIGHT_ZOMBIE_NORMAL;
	}
	else if (theZombie->mZombieHeight == ZombieHeight::HEIGHT_DOWN_OFF_HIGH_GROUND)
	{
		theZombie->mOnHighGround = false;
		theZombie->mZombieHeight = ZombieHeight::HEIGHT_ZOMBIE_NORMAL;
	}
	else if (theZombie->mOnHighGround)
	{
		theZombie->mAltitude += HIGH_GROUND_HEIGHT;
	}

	if (theZombie->mZombiePhase == ZombiePhase::PHASE_POGO_FORWARD_BOUNCE_2 && theZombie->mPhaseCounter == 70)
	{
		Plant *aPlant = theZombie->FindPlantTarget(ZombieAttackType::ATTACKTYPE_VAULT);
		if (aPlant && aPlant->HasComponent<TallComponent>())
		{
			theZombie->mApp->PlayFoley(FoleyType::FOLEY_BONK);
			theZombie->mApp->AddTodParticle(aPlant->mX + 60, aPlant->mY - 20, theZombie->mRenderOrder + 1,
			                                ParticleEffect::PARTICLE_TALL_NUT_BLOCK);

			theZombie->RemoveComponent<ShieldComponent>();
			theZombie->PogoBreak(0U);
			return;
		}
	}

	if (theZombie->mPhaseCounter != 0)
		return;

	Plant *aPlant = nullptr;
	if (theZombie->IsOnBoard())
	{
		aPlant = theZombie->FindPlantTarget(ZombieAttackType::ATTACKTYPE_VAULT);
	}
	if (aPlant == nullptr)
	{
		theZombie->mZombiePhase = ZombiePhase::PHASE_POGO_BOUNCING;

		theZombie->PickRandomSpeed();
		theZombie->mPhaseCounter = POGO_BOUNCE_TIME;
		return;
	}

	if (theZombie->mZombiePhase == ZombiePhase::PHASE_POGO_HIGH_BOUNCE_1)
	{
		theZombie->mZombiePhase = ZombiePhase::PHASE_POGO_FORWARD_BOUNCE_2;
		theZombie->mVelX = (theZombie->mX - aPlant->mX + 60) / (float)POGO_BOUNCE_TIME;
		theZombie->mPhaseCounter = POGO_BOUNCE_TIME;
	}
	else
	{
		theZombie->mZombiePhase = ZombiePhase::PHASE_POGO_HIGH_BOUNCE_1;
		theZombie->mVelX = 0.0f;
		theZombie->mPhaseCounter = POGO_BOUNCE_TIME;
	}
}

// ─── Losing the head ─────────────────────────────────────────────────────────

void PolevaulterBehaviour::OnLoseHead(Zombie *theZombie, unsigned int theDamageFlags)
{
	(void)theDamageFlags;
	theZombie->DropPole();
}

void BalloonBehaviour::OnLoseHead(Zombie *theZombie, unsigned int theDamageFlags)
{
	(void)theDamageFlags;
	theZombie->ReanimShowPrefix("anim_hat", RENDER_GROUP_HIDDEN);
	theZombie->ReanimShowPrefix("hat", RENDER_GROUP_HIDDEN);
}

void PogoBehaviour::OnLoseHead(Zombie *theZombie, unsigned int theDamageFlags)
{
	theZombie->PogoBreak(theDamageFlags);
}

void FlagBearerBehaviour::OnLoseHead(Zombie *theZombie, unsigned int theDamageFlags)
{
	(void)theDamageFlags;
	theZombie->DropFlag();
}

// ─── Mowers ──────────────────────────────────────────────────────────────────

void PolevaulterBehaviour::OnMowed(Zombie *theZombie)
{
	theZombie->DropPole();
}

void NewspaperBehaviour::OnMowed(Zombie *theZombie)
{
	theZombie->DropHead(0U);
}

void BalloonBehaviour::OnMowed(Zombie *theZombie)
{
	theZombie->DropHead(0U);
}

void PogoBehaviour::OnMowed(Zombie *theZombie)
{
	theZombie->DropHead(0U);
	theZombie->mAltitude = 0.0f;
}

void FlagBearerBehaviour::OnMowed(Zombie *theZombie)
{
	theZombie->DropFlag();
}

/// @brief A vehicle blows up instead of going through the mower
static void BlowUpVehicle(Zombie *theZombie, ParticleEffect theExplosion)
{
	theZombie->mApp->AddTodParticle(theZombie->mPosX + 80.0f, theZombie->mPosY + 60.0f, theZombie->mRenderOrder + 1,
	                                theExplosion);
	theZombie->mApp->PlayFoley(FoleyType::FOLEY_EXPLOSION);
	theZombie->DieWithLoot();
}

bool CatapultBehaviour::OnHitByMower(Zombie *theZombie)
{
	BlowUpVehicle(theZombie, ParticleEffect::PARTICLE_CATAPULT_EXPLOSION);
	return true;
}

bool ZamboniBehaviour::OnHitByMower(Zombie *theZombie)
{
	BlowUpVehicle(theZombie, ParticleEffect::PARTICLE_ZAMBONI_EXPLOSION);
	return true;
}

bool BossBehaviour::OnHitByMower(Zombie *theZombie)
{
	(void)theZombie;
	return true;
}

// ─── Taking damage ───────────────────────────────────────────────────────────

bool ZamboniBehaviour::OnBodyDamaged(Zombie *theZombie, unsigned int theDamageFlags, int theHealthBefore,
                                     int theDamageIndexBefore, int theDamageIndexAfter)
{
	(void)theHealthBefore;
	Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);
	if (!TestBit(theDamageFlags, (int)DamageFlags::DAMAGE_DOESNT_CAUSE_FLASH))
	{
		theZombie->mApp->PlayFoley(FoleyType::FOLEY_SHIELD_HIT);
	}

	if (TestBit(theDamageFlags, (int)DamageFlags::DAMAGE_SPIKE))
	{
		aBodyReanim->SetImageOverride("Zombie_zamboni_1", IMAGE_REANIM_ZOMBIE_ZAMBONI_1_DAMAGE2);
		aBodyReanim->SetImageOverride("Zombie_zamboni_2", IMAGE_REANIM_ZOMBIE_ZAMBONI_2_DAMAGE2);
		theZombie->ZamboniDeath(theDamageFlags);
	}
	else if (theZombie->Health().mHealth <= 0)
	{
		theZombie->ZamboniDeath(theDamageFlags);
	}
	else if (theDamageIndexBefore != theDamageIndexAfter)
	{
		if (theDamageIndexAfter == 1)
		{
			aBodyReanim->SetImageOverride("Zombie_zamboni_1", IMAGE_REANIM_ZOMBIE_ZAMBONI_1_DAMAGE1);
			aBodyReanim->SetImageOverride("Zombie_zamboni_2", IMAGE_REANIM_ZOMBIE_ZAMBONI_2_DAMAGE1);
		}
		else if (theDamageIndexAfter == 2)
		{
			aBodyReanim->SetImageOverride("Zombie_zamboni_1", IMAGE_REANIM_ZOMBIE_ZAMBONI_1_DAMAGE2);
			aBodyReanim->SetImageOverride("Zombie_zamboni_2", IMAGE_REANIM_ZOMBIE_ZAMBONI_2_DAMAGE2);
			theZombie->AddAttachedParticle(27, 72, ParticleEffect::PARTICLE_ZAMBONI_SMOKE);
		}
	}
	return true;
}

bool CatapultBehaviour::OnBodyDamaged(Zombie *theZombie, unsigned int theDamageFlags, int theHealthBefore,
                                      int theDamageIndexBefore, int theDamageIndexAfter)
{
	(void)theHealthBefore;
	Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);
	if (TestBit(theDamageFlags, (int)DamageFlags::DAMAGE_SPIKE) || theZombie->Health().mHealth <= 0)
	{
		aBodyReanim->SetImageOverride("Zombie_catapult_siding", IMAGE_REANIM_ZOMBIE_CATAPULT_SIDING_DAMAGE);
		theZombie->CatapultDeath(theDamageFlags);
	}
	else if (theDamageIndexBefore != theDamageIndexAfter)
	{
		if (theDamageIndexAfter == 1)
		{
			aBodyReanim->SetImageOverride("Zombie_catapult_siding", IMAGE_REANIM_ZOMBIE_CATAPULT_SIDING_DAMAGE);
		}
		else if (theDamageIndexAfter == 2)
		{
			theZombie->AddAttachedParticle(47, 77, ParticleEffect::PARTICLE_ZAMBONI_SMOKE);
		}
	}
	return true;
}

bool GargantuarBehaviour::OnBodyDamaged(Zombie *theZombie, unsigned int theDamageFlags, int theHealthBefore,
                                        int theDamageIndexBefore, int theDamageIndexAfter)
{
	(void)theDamageFlags;
	(void)theHealthBefore;
	Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);
	if (theDamageIndexBefore != theDamageIndexAfter)
	{
		if (theDamageIndexAfter == 1)
		{
			aBodyReanim->SetImageOverride("Zombie_gargantua_body1", IMAGE_REANIM_ZOMBIE_GARGANTUAR_BODY1_2);
			aBodyReanim->SetImageOverride("Zombie_gargantuar_outerarm_lower",
			                              IMAGE_REANIM_ZOMBIE_GARGANTUAR_OUTERARM_LOWER2);
		}
		else if (theDamageIndexAfter == 2)
		{
			aBodyReanim->SetImageOverride("Zombie_gargantua_body1", IMAGE_REANIM_ZOMBIE_GARGANTUAR_BODY1_3);
			aBodyReanim->SetImageOverride("Zombie_gargantuar_outerleg_foot", IMAGE_REANIM_ZOMBIE_GARGANTUAR_FOOT2);
			aBodyReanim->SetImageOverride("Zombie_gargantuar_outerarm_lower",
			                              IMAGE_REANIM_ZOMBIE_GARGANTUAR_OUTERARM_LOWER2);
			if (theZombie->mZombieType == ZombieType::ZOMBIE_REDEYE_GARGANTUAR)
			{
				aBodyReanim->SetImageOverride("anim_head1", IMAGE_REANIM_ZOMBIE_GARGANTUAR_HEAD2_REDEYE);
			}
			else
			{
				aBodyReanim->SetImageOverride("anim_head1", IMAGE_REANIM_ZOMBIE_GARGANTUAR_HEAD2);
			}
		}
	}
	return true;
}

bool BossBehaviour::OnBodyDamaged(Zombie *theZombie, unsigned int theDamageFlags, int theHealthBefore,
                                  int theDamageIndexBefore, int theDamageIndexAfter)
{
	if (!TestBit(theDamageFlags, (int)DamageFlags::DAMAGE_DOESNT_CAUSE_FLASH))
	{
		theZombie->mApp->PlayFoley(FoleyType::FOLEY_SHIELD_HIT);
	}

	Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);
	if (theDamageIndexBefore != theDamageIndexAfter)
	{
		if (theDamageIndexAfter == 1)
		{
			aBodyReanim->SetImageOverride("Boss_head", IMAGE_REANIM_ZOMBIE_BOSS_HEAD_DAMAGE1);
			aBodyReanim->SetImageOverride("Boss_jaw", IMAGE_REANIM_ZOMBIE_BOSS_JAW_DAMAGE1);
			aBodyReanim->SetImageOverride("Boss_outerarm_hand", IMAGE_REANIM_ZOMBIE_BOSS_OUTERARM_HAND_DAMAGE1);
			aBodyReanim->SetImageOverride("Boss_outerarm_thumb2", IMAGE_REANIM_ZOMBIE_BOSS_OUTERARM_THUMB_DAMAGE1);
			aBodyReanim->SetImageOverride("Boss_innerleg_foot", IMAGE_REANIM_ZOMBIE_BOSS_FOOT_DAMAGE1);
		}
		else if (theDamageIndexAfter == 2)
		{
			aBodyReanim->SetImageOverride("Boss_head", IMAGE_REANIM_ZOMBIE_BOSS_HEAD_DAMAGE2);
			aBodyReanim->SetImageOverride("Boss_jaw", IMAGE_REANIM_ZOMBIE_BOSS_JAW_DAMAGE2);
			aBodyReanim->SetImageOverride("Boss_outerarm_hand", IMAGE_REANIM_ZOMBIE_BOSS_OUTERARM_HAND_DAMAGE2);
			aBodyReanim->SetImageOverride("Boss_outerarm_thumb2", IMAGE_REANIM_ZOMBIE_BOSS_OUTERARM_THUMB_DAMAGE2);
			aBodyReanim->SetImageOverride("Boss_outerleg_foot", IMAGE_REANIM_ZOMBIE_BOSS_FOOT_DAMAGE2);
			theZombie->ApplyBossSmokeParticles(true);
		}
	}

	int &aHealth = theZombie->Health().mHealth;
	int aFlashHealth = theZombie->Health().mMaxHealth / BOSS_FLASH_HEALTH_FRACTION;
	if (theHealthBefore >= aFlashHealth && aHealth < aFlashHealth)
	{
		theZombie->mApp->AddTodParticle(770.0f, 260.0f, Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_TOP, 0, 0),
		                                ParticleEffect::PARTICLE_BOSS_EXPLOSION);
		theZombie->mApp->PlayFoley(FoleyType::FOLEY_BOSS_EXPLOSION_SMALL);
		theZombie->ApplyBossSmokeParticles(true);
	}

	// Dr. Zomboss only dies through his finale
	if (aHealth <= 0)
	{
		aHealth = 1;
	}
	return true;
}

// ─── Burning ─────────────────────────────────────────────────────────────────

void BalloonBehaviour::OnBurned(Zombie *theZombie)
{
	if (theZombie->mZombiePhase == ZombiePhase::PHASE_ZOMBIE_BURNED)
	{
		theZombie->BalloonPropellerHatSpin(false);
	}
}

void BobsledBehaviour::OnBurned(Zombie *theZombie)
{
	theZombie->BobsledBurn();
}

/// @brief A headless Squash zombie's squash goes with it
static void RemoveLostSquash(Zombie *theZombie)
{
	if (!theZombie->mHasHead)
	{
		theZombie->mApp->RemoveReanimation(theZombie->mSpecialHeadReanimID);
		theZombie->mSpecialHeadReanimID = ReanimationID::REANIMATIONID_NULL;
	}
}

void SquashHeadBehaviour::OnBurned(Zombie *theZombie)
{
	RemoveLostSquash(theZombie);
}

// ─── Dying ───────────────────────────────────────────────────────────────────

bool DolphinRiderBehaviour::HasDeathAnim(Zombie *theZombie)
{
	return theZombie->mZombiePhase == ZombiePhase::PHASE_DOLPHIN_WALKING_IN_POOL;
}

void PogoBehaviour::OnDying(Zombie *theZombie)
{
	theZombie->mAltitude = 0.0f;
}

void SquashHeadBehaviour::OnDying(Zombie *theZombie)
{
	RemoveLostSquash(theZombie);
}

void BossBehaviour::OnDying(Zombie *theZombie)
{
	theZombie->BossDie();
	ZombieBodyComponent *aBody = theZombie->GetComponent<ZombieBodyComponent>();
	Reanimation *aHeadReanim = theZombie->mApp->ReanimationGet(theZombie->mSpecialHeadReanimID);
	aHeadReanim->PlayReanim("anim_death", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20,
	                        aBody ? aBody->mDeathAnimRateMin : 18.0f);
}

bool ZamboniBehaviour::UpdateDying(Zombie *theZombie)
{
	if (theZombie->mPhaseCounter <= 0)
		return false;

	theZombie->mPhaseCounter--;
	if (theZombie->mPhaseCounter == 0)
	{
		Reanimation *aBodyReanim = theZombie->mApp->ReanimationTryToGet(theZombie->mBodyReanimID);
		if (aBodyReanim->IsTrackShowing("anim_wheelie2"))
		{
			theZombie->mApp->AddTodParticle(theZombie->mPosX + 80.0f, theZombie->mPosY + 60.0f,
			                                theZombie->mRenderOrder + 1, ParticleEffect::PARTICLE_ZAMBONI_EXPLOSION2);
		}
		else
		{
			theZombie->mApp->AddTodParticle(theZombie->mPosX + 80.0f, theZombie->mPosY + 60.0f,
			                                theZombie->mRenderOrder + 1, ParticleEffect::PARTICLE_ZAMBONI_EXPLOSION);
		}

		theZombie->DieWithLoot();
		theZombie->mApp->PlayFoley(FoleyType::FOLEY_EXPLOSION);
	}
	return true;
}

bool CatapultBehaviour::UpdateDying(Zombie *theZombie)
{
	theZombie->mPhaseCounter--;
	if (theZombie->mPhaseCounter == 0)
	{
		theZombie->mApp->AddTodParticle(theZombie->mPosX + 80.0f, theZombie->mPosY + 60.0f,
		                                theZombie->mRenderOrder + 1, ParticleEffect::PARTICLE_CATAPULT_EXPLOSION);
		theZombie->DieWithLoot();
		theZombie->mApp->PlayFoley(FoleyType::FOLEY_EXPLOSION);
	}
	return true;
}

bool BossBehaviour::UpdateDying(Zombie *theZombie)
{
	Reanimation *aBodyReanim = theZombie->mApp->ReanimationTryToGet(theZombie->mBodyReanimID);
	if (aBodyReanim->ShouldTriggerTimedEvent(0.1f) || aBodyReanim->ShouldTriggerTimedEvent(0.12f) ||
	    aBodyReanim->ShouldTriggerTimedEvent(0.15f) || aBodyReanim->ShouldTriggerTimedEvent(0.19f) ||
	    aBodyReanim->ShouldTriggerTimedEvent(0.2f) || aBodyReanim->ShouldTriggerTimedEvent(0.26f) ||
	    aBodyReanim->ShouldTriggerTimedEvent(0.3f) || aBodyReanim->ShouldTriggerTimedEvent(0.4f) ||
	    aBodyReanim->ShouldTriggerTimedEvent(0.42f) || aBodyReanim->ShouldTriggerTimedEvent(0.5f) ||
	    aBodyReanim->ShouldTriggerTimedEvent(0.58f) || aBodyReanim->ShouldTriggerTimedEvent(0.61f) ||
	    aBodyReanim->ShouldTriggerTimedEvent(0.71f))
	{
		float aExplosionPosX = RandRangeFloat(600.0f, 750.0f);
		float aExplosionPosY = RandRangeFloat(50.0f, 300.0f);
		theZombie->mApp->AddTodParticle(aExplosionPosX, aExplosionPosY, (int)RenderLayer::RENDER_LAYER_TOP,
		                                ParticleEffect::PARTICLE_BOSS_EXPLOSION);
		theZombie->mApp->PlayFoley(FoleyType::FOLEY_BOSS_EXPLOSION_SMALL);
	}

	Reanimation *aHeadReanim = theZombie->mApp->ReanimationTryToGet(theZombie->mSpecialHeadReanimID);
	if (aBodyReanim->ShouldTriggerTimedEvent(0.93f))
	{
		theZombie->mBoard->ShakeBoard(1, 2);
		theZombie->mApp->PlayFoley(FoleyType::FOLEY_BOSS_EXPLOSION_SMALL);
		theZombie->mApp->PlayFoley(FoleyType::FOLEY_THUMP);
	}

	if (aBodyReanim->ShouldTriggerTimedEvent(0.99f))
	{
		aHeadReanim->PlayReanim("anim_flag", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 30.0f);
	}

	if (aHeadReanim->IsAnimPlaying("anim_flag") && aHeadReanim->mLoopCount > 0)
	{
		aHeadReanim->PlayReanim("anim_flag_loop", ReanimLoopType::REANIM_LOOP, 20, 17.0f);
	}

	if (aBodyReanim->mLoopCount > 0)
	{
		theZombie->DropLoot();
	}
	return true;
}

// ─── Leaving the board ───────────────────────────────────────────────────────

void BobsledBehaviour::OnRemoved(Zombie *theZombie)
{
	theZombie->BobsledDie();
}

void BungeeBehaviour::OnRemoved(Zombie *theZombie)
{
	theZombie->BungeeDie();
}

void BossBehaviour::OnRemoved(Zombie *theZombie)
{
	theZombie->BossDie();
}

// ─── Drawing ─────────────────────────────────────────────────────────────────

bool DolphinRiderBehaviour::PlaceBody(Zombie *theZombie, ZombieDrawPosition &theDrawPos)
{
	float aAltitude = theZombie->mAltitude;
	theDrawPos.mBodyY = -aAltitude;
	theDrawPos.mClipHeight = CLIP_HEIGHT_OFF;

	ZombiePhase aPhase = theZombie->mZombiePhase;
	if (aPhase == ZombiePhase::PHASE_DOLPHIN_INTO_POOL)
	{
		Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);

		if (aBodyReanim->mAnimTime >= 0.56f && aBodyReanim->mAnimTime <= 0.65f)
		{
			theDrawPos.mClipHeight = 0.0f;
		}
		else if (aBodyReanim->mAnimTime >= 0.75f)
		{
			theDrawPos.mClipHeight = -aAltitude - 10.0f;
		}
	}
	else if (aPhase == ZombiePhase::PHASE_DOLPHIN_RIDING)
	{
		theDrawPos.mImageOffsetX += 70.0f;

		if (theZombie->mZombieHeight == ZombieHeight::HEIGHT_DRAGGED_UNDER)
		{
			theDrawPos.mClipHeight = -aAltitude - 15.0f;
		}
		else
		{
			theDrawPos.mClipHeight = -aAltitude - 10.0f;
		}
	}
	else if (aPhase == ZombiePhase::PHASE_DOLPHIN_IN_JUMP)
	{
		theDrawPos.mImageOffsetX += 70.0f + aAltitude;

		Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);
		if (aBodyReanim->mAnimTime <= 0.06f)
		{
			theDrawPos.mClipHeight = -aAltitude - 10.0f;
		}
		else if (aBodyReanim->mAnimTime >= 0.5f && aBodyReanim->mAnimTime <= 0.76f)
		{
			theDrawPos.mClipHeight = -13.0f;
		}
	}
	else if (aPhase == ZombiePhase::PHASE_DOLPHIN_WALKING_IN_POOL || aPhase == ZombiePhase::PHASE_ZOMBIE_DYING)
	{
		theDrawPos.mImageOffsetY += 50.0f;

		if (aPhase == ZombiePhase::PHASE_ZOMBIE_DYING)
		{
			theDrawPos.mClipHeight = -aAltitude + 44.0f;
		}
		else if (theZombie->mZombieHeight == ZombieHeight::HEIGHT_DRAGGED_UNDER)
		{
			theDrawPos.mClipHeight = -aAltitude + 36.0f;
		}
	}
	else if (aPhase == ZombiePhase::PHASE_DOLPHIN_WALKING &&
	         theZombie->mZombieHeight == ZombieHeight::HEIGHT_OUT_OF_POOL)
	{
		theDrawPos.mClipHeight = -aAltitude;
	}
	else if (aPhase == ZombiePhase::PHASE_DOLPHIN_WALKING_WITHOUT_DOLPHIN &&
	         theZombie->mZombieHeight == ZombieHeight::HEIGHT_OUT_OF_POOL)
	{
		theDrawPos.mClipHeight = -aAltitude;
	}
	return true;
}

bool SnorkelBehaviour::PlaceBody(Zombie *theZombie, ZombieDrawPosition &theDrawPos)
{
	theDrawPos.mBodyY = -theZombie->mAltitude;
	theDrawPos.mClipHeight = CLIP_HEIGHT_OFF;

	if (theZombie->mZombiePhase == ZombiePhase::PHASE_SNORKEL_INTO_POOL)
	{
		Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);
		if (aBodyReanim->mAnimTime >= 0.8f)
		{
			theDrawPos.mClipHeight = -10.0f;
		}
	}
	else if (theZombie->mInPool)
	{
		theDrawPos.mClipHeight = -theZombie->mAltitude - 5.0f;
		theDrawPos.mClipHeight += 20.0f - 20.0f * theZombie->mScaleZombie;
	}
	return true;
}

bool BungeeBehaviour::PlaceBody(Zombie *theZombie, ZombieDrawPosition &theDrawPos)
{
	theDrawPos.mBodyY = -theZombie->mAltitude;
	theDrawPos.mImageOffsetX -= 18.0f;

	if (theZombie->IsOnHighGround())
	{
		theDrawPos.mBodyY -= HIGH_GROUND_HEIGHT;
	}

	theDrawPos.mClipHeight = CLIP_HEIGHT_OFF;
	return true;
}

bool BobsledBehaviour::DrawBody(Zombie *theZombie, Sexy::Graphics *g, const ZombieDrawPosition &theDrawPos,
                                int theBaseRenderGroup)
{
	theZombie->DrawBobsledReanim(g, theDrawPos, true);
	theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID)->DrawRenderGroup(g, theBaseRenderGroup);
	theZombie->DrawBobsledReanim(g, theDrawPos, false);
	return true;
}

bool BungeeBehaviour::DrawBody(Zombie *theZombie, Sexy::Graphics *g, const ZombieDrawPosition &theDrawPos,
                               int theBaseRenderGroup)
{
	(void)theBaseRenderGroup;
	theZombie->DrawBungeeReanim(g, theDrawPos);
	return true;
}

bool DancerBehaviour::DrawBody(Zombie *theZombie, Sexy::Graphics *g, const ZombieDrawPosition &theDrawPos,
                               int theBaseRenderGroup)
{
	(void)theBaseRenderGroup;
	theZombie->DrawDancerReanim(g, theDrawPos);
	return true;
}

// ─── Losing an arm ───────────────────────────────────────────────────────────

void PogoBehaviour::OnLoseArm(Zombie *theZombie)
{
	Reanimation *aBodyReanim = theZombie->mApp->ReanimationGet(theZombie->mBodyReanimID);
	aBodyReanim->SetImageOverride("Zombie_pogo_stickhands", IMAGE_REANIM_ZOMBIE_POGO_STICKHANDS2);
	aBodyReanim->SetImageOverride("Zombie_pogo_stick", IMAGE_REANIM_ZOMBIE_POGO_STICKDAMAGE2);
	aBodyReanim->SetImageOverride("Zombie_pogo_stick2", IMAGE_REANIM_ZOMBIE_POGO_STICK2DAMAGE2);
}

void FlagBearerBehaviour::OnLoseArm(Zombie *theZombie)
{
	Reanimation *aHeadReanim = theZombie->mApp->ReanimationTryToGet(theZombie->mSpecialHeadReanimID);
	if (aHeadReanim)
	{
		aHeadReanim->SetImageOverride("Zombie_flag", IMAGE_REANIM_ZOMBIE_FLAG3);
	}
}

// ─── Butter ──────────────────────────────────────────────────────────────────

void PogoBehaviour::OnButtered(Zombie *theZombie)
{
	theZombie->mAltitude = 0.0f;
	if (theZombie->mOnHighGround)
	{
		theZombie->mAltitude += HIGH_GROUND_HEIGHT;
	}
}

void BalloonBehaviour::OnButtered(Zombie *theZombie)
{
	theZombie->BalloonPropellerHatSpin(false);
}

void BalloonBehaviour::OnButterRemoved(Zombie *theZombie)
{
	theZombie->BalloonPropellerHatSpin(true);
}

/// @brief A shooting plant head that was mid-shot when buttered goes back to its shooting pace
static void ResumeShootingHead(Zombie *theZombie, float theShootingRate)
{
	Reanimation *aHeadReanim = theZombie->mApp->ReanimationTryToGet(theZombie->mSpecialHeadReanimID);
	if (aHeadReanim && aHeadReanim->IsAnimPlaying("anim_shooting"))
	{
		aHeadReanim->mAnimRate = theShootingRate;
	}
}

void PeaHeadBehaviour::OnButterRemoved(Zombie *theZombie)
{
	ResumeShootingHead(theZombie, 35.0f);
}

void GatlingHeadBehaviour::OnButterRemoved(Zombie *theZombie)
{
	ResumeShootingHead(theZombie, 38.0f);
}
