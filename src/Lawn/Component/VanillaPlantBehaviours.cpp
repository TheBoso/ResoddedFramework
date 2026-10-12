// Behaviours of the vanilla plants, moved out of Plant.cpp (they were Plant::UpdateSquash and friends).
// They act on the plant they belong to through thePlant.

#include "PlantBehaviours.h"
#include "../../Resources.h"
#include "../Coin.h"
#include "../Plant.h"
#include "../Board.h"
#include "../Zombie.h"
#include "../Cutscene.h"
#include "../GridItem.h"
#include "../ZenGarden.h"
#include "../Challenge.h"
#include "../Projectile.h"
#include "../../LawnApp.h"
#include "../CursorObject.h"
#include "../../GameConstants.h"
#include "../System/PlayerInfo.h"
#include "../System/ReanimationLawn.h"
#include "../System/Achievements.h"
#include "../../Sexy.TodLib/TodFoley.h"
#include "../../Sexy.TodLib/TodDebug.h"
#include "../../Sexy.TodLib/Attachment.h"
#include "../../Sexy.TodLib/Reanimator.h"
#include "../../Sexy.TodLib/TodParticle.h"
#include "../../Sexy.TodLib/EffectSystem.h"
#include "Teams.h"

void SquashBehaviour::UpdateBehaviour(Plant *thePlant)
{
	Reanimation *aBodyReanim = thePlant->mApp->ReanimationTryToGet(thePlant->mBodyReanimID);
	TOD_ASSERT(aBodyReanim);

	if (thePlant->mState == PlantState::STATE_NOTREADY)
	{
		Zombie *aZombie = thePlant->FindSquashTarget();
		if (aZombie)
		{
			thePlant->mTargetZombieID = thePlant->mBoard->ZombieGetID(aZombie);
			thePlant->mTargetX = aZombie->ZombieTargetLeadX(0.0f) - thePlant->mWidth / 2;
			thePlant->mState = PlantState::STATE_SQUASH_LOOK;
			thePlant->mStateCountdown = 80;
			thePlant->PlayBodyReanim(thePlant->mTargetX < thePlant->mX ? "anim_lookleft" : "anim_lookright",
			                         ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 10, 24.0f);
			thePlant->mApp->PlayFoley(FoleyType::FOLEY_SQUASH_HMM);
		}
	}
	else if (thePlant->mState == PlantState::STATE_SQUASH_LOOK)
	{
		if (thePlant->mStateCountdown <= 0)
		{
			thePlant->PlayBodyReanim("anim_jumpup", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 24.0f);
			thePlant->mState = PlantState::STATE_SQUASH_PRE_LAUNCH;
			thePlant->mStateCountdown = 30;
		}
	}
	else if (thePlant->mState == PlantState::STATE_SQUASH_PRE_LAUNCH)
	{
		if (thePlant->mStateCountdown <= 0)
		{
			Zombie *aZombie = thePlant->FindSquashTarget();
			if (aZombie)
			{
				thePlant->mTargetX = aZombie->ZombieTargetLeadX(30.0f) - thePlant->mWidth / 2;
			}

			thePlant->mState = PlantState::STATE_SQUASH_RISING;
			thePlant->mStateCountdown = 50;
			thePlant->mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_PARTICLE, thePlant->mRow, 0);
		}
	}
	else
	{
		int aTargetCol = thePlant->mBoard->PixelToGridXKeepOnBoard(thePlant->mTargetX, thePlant->mY);
		int aDestY = thePlant->mBoard->GridToPixelY(aTargetCol, thePlant->mRow) + 8;

		if (thePlant->mState == PlantState::STATE_SQUASH_RISING)
		{
			thePlant->mX = TodAnimateCurve(50, 20, thePlant->mStateCountdown, thePlant->mBoard->GridToPixelX(thePlant->mPlantCol, thePlant->mStartRow), thePlant->mTargetX,
			                               TodCurves::CURVE_EASE_IN_OUT);
			thePlant->mY = TodAnimateCurve(50, 20, thePlant->mStateCountdown, thePlant->mBoard->GridToPixelY(thePlant->mPlantCol, thePlant->mStartRow), aDestY - 120,
			                               TodCurves::CURVE_EASE_IN_OUT);

			if (thePlant->mStateCountdown == 0)
			{
				thePlant->PlayBodyReanim("anim_jumpdown", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, 60.0f);
				thePlant->mState = PlantState::STATE_SQUASH_FALLING;
				thePlant->mStateCountdown = 10;
			}
		}
		else if (thePlant->mState == PlantState::STATE_SQUASH_FALLING)
		{
			thePlant->mY = TodAnimateCurve(10, 0, thePlant->mStateCountdown, aDestY - 120, aDestY, TodCurves::CURVE_EASE_IN_OUT);

			if (thePlant->mStateCountdown == 5)
			{
				thePlant->DoSquashDamage();
			}

			if (thePlant->mStateCountdown == 0)
			{
				if (thePlant->mBoard->IsPoolSquare(aTargetCol, thePlant->mRow))
				{
					thePlant->mApp->AddReanimation(thePlant->mX - 11, thePlant->mY + 20, thePlant->mRenderOrder + 1, ReanimationType::REANIM_SPLASH);
					thePlant->mApp->PlayFoley(FoleyType::FOLEY_SPLAT);
					thePlant->mApp->PlaySample(SOUND_ZOMBIESPLASH);

					thePlant->Die();
				}
				else
				{
					thePlant->mState = PlantState::STATE_SQUASH_DONE_FALLING;
					thePlant->mStateCountdown = 100;

					thePlant->mBoard->ShakeBoard(1, 4);
					thePlant->mApp->PlayFoley(FoleyType::FOLEY_THUMP);
					float aOffsetY = thePlant->mBoard->StageHasRoof() ? 69.0f : 80.0f;
					thePlant->mApp->AddTodParticle(thePlant->mX + 40, thePlant->mY + aOffsetY, thePlant->mRenderOrder + 4,
					                               ParticleEffect::PARTICLE_DUST_SQUASH);
				}
			}
		}
		else if (thePlant->mState == PlantState::STATE_SQUASH_DONE_FALLING)
		{
			if (thePlant->mStateCountdown == 0)
			{
				thePlant->Die();
			}
		}
	}
}

void ChomperBehaviour::UpdateBehaviour(Plant *thePlant)
{
	Reanimation *aBodyReanim = thePlant->mApp->ReanimationTryToGet(thePlant->mBodyReanimID);
	if (thePlant->mState == PlantState::STATE_READY)
	{
		if (thePlant->FindTargetZombie(thePlant->mRow, PlantWeapon::WEAPON_PRIMARY))
		{
			thePlant->PlayBodyReanim("anim_bite", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 24.0f);
			thePlant->mState = PlantState::STATE_CHOMPER_BITING;
			thePlant->mStateCountdown = 70;
		}
	}
	else if (thePlant->mState == PlantState::STATE_CHOMPER_BITING)
	{
		if (thePlant->mStateCountdown == 0)
		{
			thePlant->mApp->PlayFoley(FoleyType::FOLEY_BIGCHOMP);

			Zombie *aZombie = thePlant->FindTargetZombie(thePlant->mRow, PlantWeapon::WEAPON_PRIMARY);
			bool doBite = false;
			if (aZombie)
			{
				if (aZombie->mZombieType == ZombieType::ZOMBIE_GARGANTUAR ||
				    aZombie->mZombieType == ZombieType::ZOMBIE_REDEYE_GARGANTUAR ||
				    aZombie->mZombieType == ZombieType::ZOMBIE_BOSS)
				{
					doBite = true;
				}
			}
			bool doMiss = false;
			if (aZombie == nullptr)
			{
				doMiss = true;
			}
			else if (!aZombie->IsImmobilizied())
			{
				if (aZombie->IsBouncingPogo() || aZombie->mZombiePhase == ZombiePhase::PHASE_POLEVAULTER_IN_VAULT ||
				    aZombie->mZombiePhase == ZombiePhase::PHASE_POLEVAULTER_PRE_VAULT)
				{
					doMiss = true;
				}
			}

			if (doBite)
			{
				thePlant->mApp->PlayFoley(FoleyType::FOLEY_SPLAT);
				aZombie->TakeDamage(40, 0U);
				thePlant->mState = PlantState::STATE_CHOMPER_BITING_MISSED;
			}
			else if (doMiss)
			{
				thePlant->mState = PlantState::STATE_CHOMPER_BITING_MISSED;
			}
			else
			{
				aZombie->DieWithLoot();
				thePlant->mState = PlantState::STATE_CHOMPER_BITING_GOT_ONE;
			}
		}
	}
	else if (thePlant->mState == PlantState::STATE_CHOMPER_BITING_GOT_ONE)
	{
		if (aBodyReanim->mLoopCount > 0)
		{
			thePlant->PlayBodyReanim("anim_chew", ReanimLoopType::REANIM_LOOP, 0, 15.0f);
			if (thePlant->mApp->IsIZombieLevel())
			{
				aBodyReanim->mAnimRate = 0;
			}

			thePlant->mState = PlantState::STATE_CHOMPER_DIGESTING;
			thePlant->mStateCountdown = 4000;
		}
	}
	else if (thePlant->mState == PlantState::STATE_CHOMPER_DIGESTING)
	{
		if (thePlant->mStateCountdown == 0)
		{
			thePlant->PlayBodyReanim("anim_swallow", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 12.0f);
			thePlant->mState = PlantState::STATE_CHOMPER_SWALLOWING;
		}
	}
	else if ((thePlant->mState == PlantState::STATE_CHOMPER_SWALLOWING || thePlant->mState == PlantState::STATE_CHOMPER_BITING_MISSED) &&
	         aBodyReanim->mLoopCount > 0)
	{
		thePlant->PlayIdleAnim(aBodyReanim->mDefinition->mFPS);
		thePlant->mState = PlantState::STATE_READY;
	}
}

void BloverBehaviour::UpdateBehaviour(Plant *thePlant)
{
	Reanimation *aBodyReanim = thePlant->mApp->ReanimationGet(thePlant->mBodyReanimID);
	if (aBodyReanim->mLoopCount > 0 && aBodyReanim->mLoopType != ReanimLoopType::REANIM_LOOP)
	{
		aBodyReanim->SetFramesForLayer("anim_loop");
		aBodyReanim->mLoopType = ReanimLoopType::REANIM_LOOP;
	}

	if (thePlant->mState != PlantState::STATE_DOINGSPECIAL && thePlant->mStateCountdown == 0)
	{
		thePlant->DoSpecial();
	}
}

void FlowerPotBehaviour::UpdateBehaviour(Plant *thePlant)
{
	if (thePlant->mState == PlantState::STATE_FLOWERPOT_INVULNERABLE && thePlant->mStateCountdown == 0)
		thePlant->mState = PlantState::STATE_NOTREADY;
}

void LilyPadBehaviour::UpdateBehaviour(Plant *thePlant)
{
	if (thePlant->mState == PlantState::STATE_LILYPAD_INVULNERABLE && thePlant->mStateCountdown == 0)
		thePlant->mState = PlantState::STATE_NOTREADY;
}

void ImitaterBehaviour::UpdateBehaviour(Plant *thePlant)
{
	if (thePlant->mState != PlantState::STATE_IMITATER_MORPHING)
	{
		if (thePlant->mStateCountdown == 0)
		{
			thePlant->mState = PlantState::STATE_IMITATER_MORPHING;
			thePlant->PlayBodyReanim("anim_explode", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, 26.0f);
		}
	}
	else
	{
		Reanimation *aBodyReanim = thePlant->mApp->ReanimationGet(thePlant->mBodyReanimID);
		if (aBodyReanim->ShouldTriggerTimedEvent(0.8f))
		{
			thePlant->mApp->AddTodParticle(thePlant->mX + 40, thePlant->mY + 40, (int)RenderLayer::RENDER_LAYER_TOP,
			                               ParticleEffect::PARTICLE_IMITATER_MORPH);
		}
		if (aBodyReanim->mLoopCount > 0)
		{
			thePlant->ImitaterMorph();
		}
	}
}

void CoffeeBeanBehaviour::UpdateBehaviour(Plant *thePlant)
{
	if (thePlant->mState == PlantState::STATE_DOINGSPECIAL)
	{
		Reanimation *aBodyReanim = thePlant->mApp->ReanimationGet(thePlant->mBodyReanimID);
		if (aBodyReanim->mLoopCount > 0)
		{
			thePlant->Die();
		}
	}
}

void UmbrellaBehaviour::UpdateBehaviour(Plant *thePlant)
{
	if (thePlant->mState == PlantState::STATE_UMBRELLA_TRIGGERED)
	{
		if (thePlant->mStateCountdown == 0)
		{
			thePlant->mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_PROJECTILE, thePlant->mRow + 1, 0);
			thePlant->mState = PlantState::STATE_UMBRELLA_REFLECTING;
		}
	}
	else if (thePlant->mState == PlantState::STATE_UMBRELLA_REFLECTING)
	{
		Reanimation *aBodyReanim = thePlant->mApp->ReanimationGet(thePlant->mBodyReanimID);
		if (aBodyReanim->mLoopCount > 0)
		{
			thePlant->PlayIdleAnim(0.0f);
			thePlant->mState = PlantState::STATE_NOTREADY;
			thePlant->mRenderOrder = thePlant->CalcRenderOrder();
		}
	}
}

void CobCannonBehaviour::UpdateBehaviour(Plant *thePlant)
{
	if (thePlant->mState == PlantState::STATE_COBCANNON_ARMING)
	{
		if (thePlant->mStateCountdown == 0)
		{
			thePlant->mState = PlantState::STATE_COBCANNON_LOADING;
			thePlant->PlayBodyReanim("anim_charge", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 12.0f);
		}
	}
	else if (thePlant->mState == PlantState::STATE_COBCANNON_LOADING)
	{
		Reanimation *aBodyReanim = thePlant->mApp->ReanimationGet(thePlant->mBodyReanimID);
		if (aBodyReanim->ShouldTriggerTimedEvent(0.5f))
		{
			thePlant->mApp->PlayFoley(FoleyType::FOLEY_SHOOP);
		}
		if (aBodyReanim->mLoopCount > 0)
		{
			thePlant->mState = PlantState::STATE_COBCANNON_READY;
			thePlant->PlayIdleAnim(12.0f);
		}
	}
	else if (thePlant->mState == PlantState::STATE_COBCANNON_READY)
	{
		Reanimation *aBodyReanim = thePlant->mApp->ReanimationGet(thePlant->mBodyReanimID);
		ReanimatorTrackInstance *aCobTrack = aBodyReanim->GetTrackInstanceByName("CobCannon_cob");
		aCobTrack->mTrackColor = GetFlashingColor(thePlant->mBoard->mMainCounter, 75);
	}
	else if (thePlant->mState == PlantState::STATE_COBCANNON_FIRING)
	{
		Reanimation *aBodyReanim = thePlant->mApp->ReanimationGet(thePlant->mBodyReanimID);
		if (aBodyReanim->ShouldTriggerTimedEvent(0.48f))
		{
			thePlant->mApp->PlayFoley(FoleyType::FOLEY_COB_LAUNCH);
		}
	}
}

void CactusBehaviour::UpdateBehaviour(Plant *thePlant)
{
	if (thePlant->mShootingCounter > 0)
		return;

	Reanimation *aBodyReanim = thePlant->mApp->ReanimationGet(thePlant->mBodyReanimID);
	if (thePlant->mState == PlantState::STATE_CACTUS_RISING)
	{
		if (aBodyReanim->mLoopCount > 0)
		{
			thePlant->mState = PlantState::STATE_CACTUS_HIGH;
			thePlant->PlayBodyReanim("anim_idlehigh", ReanimLoopType::REANIM_LOOP, 20, 0.0f);
			if (thePlant->mApp->IsIZombieLevel())
			{
				aBodyReanim->mAnimRate = 0;
			}

			thePlant->mLaunchCounter = 1;
		}
	}
	else if (thePlant->mState == PlantState::STATE_CACTUS_HIGH)
	{
		if (thePlant->FindTargetZombie(thePlant->mRow, PlantWeapon::WEAPON_PRIMARY) == nullptr)
		{
			thePlant->mState = PlantState::STATE_CACTUS_LOWERING;
			thePlant->PlayBodyReanim("anim_lower", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, aBodyReanim->mDefinition->mFPS);
		}
	}
	else if (thePlant->mState == PlantState::STATE_CACTUS_LOWERING)
	{
		if (aBodyReanim->mLoopCount > 0)
		{
			thePlant->mState = PlantState::STATE_CACTUS_LOW;
			thePlant->PlayIdleAnim(0.0f);
		}
	}
	else if (thePlant->FindTargetZombie(thePlant->mRow, PlantWeapon::WEAPON_PRIMARY))
	{
		thePlant->mState = PlantState::STATE_CACTUS_RISING;
		thePlant->PlayBodyReanim("anim_rise", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, aBodyReanim->mDefinition->mFPS);
		thePlant->mApp->PlayFoley(FoleyType::FOLEY_PLANTGROW);
	}
}

void MagnetShroomBehaviour::UpdateBehaviour(Plant *thePlant)
{
	for (int i = 0; i < MAX_MAGNET_ITEMS; i++)
	{
		MagnetItem *aMagnetItem = &thePlant->mMagnetItems[i];
		if (aMagnetItem->mItemType != MagnetItemType::MAGNET_ITEM_NONE)
		{
			SexyVector2 aVectorToPlant(thePlant->mX + aMagnetItem->mDestOffsetX - aMagnetItem->mPosX,
			                           thePlant->mY + aMagnetItem->mDestOffsetY - aMagnetItem->mPosY);
			if (aVectorToPlant.Magnitude() > 20.0f)
			{
				aMagnetItem->mPosX += aVectorToPlant.x * 0.05f;
				aMagnetItem->mPosY += aVectorToPlant.y * 0.05f;
			}
		}
	}

	if (thePlant->mState == PlantState::STATE_MAGNETSHROOM_CHARGING)
	{
		if (thePlant->mStateCountdown == 0)
		{
			thePlant->mState = PlantState::STATE_READY;

			float aAnimRate = RandRangeFloat(10.0f, 15.0f);
			thePlant->PlayBodyReanim("anim_idle", ReanimLoopType::REANIM_LOOP, 30, aAnimRate);
			if (thePlant->mApp->IsIZombieLevel())
			{
				Reanimation *aBodyReanim = thePlant->mApp->ReanimationTryToGet(thePlant->mBodyReanimID);
				aBodyReanim->mAnimRate = 0.0f;
			}

			thePlant->mMagnetItems[0].mItemType = MagnetItemType::MAGNET_ITEM_NONE;
		}
	}
	else if (thePlant->mState == PlantState::STATE_MAGNETSHROOM_SUCKING)
	{
		Reanimation *aBodyReanim = thePlant->mApp->ReanimationTryToGet(thePlant->mBodyReanimID);
		if (aBodyReanim->mLoopCount > 0)
		{
			thePlant->PlayBodyReanim("anim_nonactive_idle2", ReanimLoopType::REANIM_LOOP, 20, 2.0f);
			if (thePlant->mApp->IsIZombieLevel())
			{
				aBodyReanim = thePlant->mApp->ReanimationTryToGet(thePlant->mBodyReanimID);
				aBodyReanim->mAnimRate = 0.0f;
			}

			thePlant->mState = PlantState::STATE_MAGNETSHROOM_CHARGING;
		}
	}
	else
	{
		float aClosestDistance = 0.0f;
		Zombie *aClosestZombie = nullptr;

		Zombie *aZombie = nullptr;
		while (thePlant->mBoard->IterateZombies(aZombie))
		{
			int aDiffY = aZombie->mRow - thePlant->mRow;
			Rect aZombieRect = aZombie->GetZombieRect();

			if (aZombie->mMindControlled)
				continue;

			if (!aZombie->mHasHead)
				continue;

			if (aZombie->mZombieHeight != ZombieHeight::HEIGHT_ZOMBIE_NORMAL ||
			    aZombie->mZombiePhase == ZombiePhase::PHASE_RISING_FROM_GRAVE)
				continue;

			if (aZombie->IsDeadOrDying())
				continue;

			if (aZombieRect.mX > BOARD_WIDTH || aDiffY > 2 || aDiffY < -2)
				continue;

			if (aZombie->mZombiePhase == ZombiePhase::PHASE_DIGGER_TUNNELING ||
			    aZombie->mZombiePhase == ZombiePhase::PHASE_DIGGER_STUNNED ||
			    aZombie->mZombiePhase == ZombiePhase::PHASE_DIGGER_WALKING ||
			    aZombie->mZombieType == ZombieType::ZOMBIE_POGO)
			{
				if (!aZombie->mHasObject)
					continue;
			}
			else if (!(aZombie->GetHelmType() == HelmType::HELMTYPE_PAIL ||
			           aZombie->GetHelmType() == HelmType::HELMTYPE_FOOTBALL ||
			           aZombie->GetShieldType() == ShieldType::SHIELDTYPE_DOOR ||
			           aZombie->GetShieldType() == ShieldType::SHIELDTYPE_LADDER ||
			           aZombie->mZombiePhase == ZombiePhase::PHASE_JACK_IN_THE_BOX_RUNNING))
				continue;

			int aRadius = aZombie->mIsEating ? 320 : 270;
			if (GetCircleRectOverlap(thePlant->mX, thePlant->mY + 20, aRadius, aZombieRect))
			{
				float aDistance = Distance2D(thePlant->mX, thePlant->mY, aZombieRect.mX, aZombieRect.mY);
				aDistance += abs(aDiffY) * 80.0f;

				if (aClosestZombie == nullptr || aDistance < aClosestDistance)
				{
					aClosestZombie = aZombie;
					aClosestDistance = aDistance;
				}
			}
		}

		if (aClosestZombie)
		{
			thePlant->MagnetShroomAttactItem(aClosestZombie);
			return;
		}

		////////////////////

		float aClosestLadderDist = 0.0f;
		GridItem *aClosestLadder = nullptr;

		GridItem *aGridItem = nullptr;
		while (thePlant->mBoard->IterateGridItems(aGridItem))
		{
			if (aGridItem->mGridItemType == GridItemType::GRIDITEM_LADDER)
			{
				int aDiffX = abs(aGridItem->mGridX - thePlant->mPlantCol);
				int aDiffY = abs(aGridItem->mGridY - thePlant->mRow);
				int aSquareDistance = std::max(aDiffX, aDiffY);
				if (aSquareDistance <= 2)
				{
					float aDistance = aSquareDistance + aDiffY * 0.05f;
					if (aClosestLadder == nullptr || aDistance < aClosestLadderDist)
					{
						aClosestLadder = aGridItem;
						aClosestLadderDist = aDistance;
					}
				}
			}
		}

		if (aClosestLadder)
		{
			thePlant->mState = PlantState::STATE_MAGNETSHROOM_SUCKING;
			thePlant->mStateCountdown = 1500;
			thePlant->PlayBodyReanim("anim_shooting", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 12.0f);
			thePlant->mApp->PlayFoley(FoleyType::FOLEY_MAGNETSHROOM);

			aClosestLadder->GridItemDie();

			MagnetItem *aMagnetItem = thePlant->GetFreeMagnetItem();
			aMagnetItem->mPosX = thePlant->mBoard->GridToPixelX(aClosestLadder->mGridX, aClosestLadder->mGridY) + 40;
			aMagnetItem->mPosY = thePlant->mBoard->GridToPixelY(aClosestLadder->mGridX, aClosestLadder->mGridY);
			aMagnetItem->mDestOffsetX = RandRangeFloat(-10.0f, 10.0f) + 10.0f;
			aMagnetItem->mDestOffsetY = RandRangeFloat(-10.0f, 10.0f);
			aMagnetItem->mItemType = MagnetItemType::MAGNET_ITEM_LADDER_PLACED;
		}
	}
}

void GoldMagnetBehaviour::UpdateBehaviour(Plant *thePlant)
{
	Reanimation *aBodyReanim = thePlant->mApp->ReanimationGet(thePlant->mBodyReanimID);

	bool aIsSuckingCoin = false;
	for (int i = 0; i < MAX_MAGNET_ITEMS; i++)
	{
		MagnetItem *aMagnetItem = &thePlant->mMagnetItems[i];
		if (aMagnetItem->mItemType != MagnetItemType::MAGNET_ITEM_NONE)
		{
			SexyVector2 aVectorToPlant(thePlant->mX + aMagnetItem->mDestOffsetX - aMagnetItem->mPosX,
			                           thePlant->mY + aMagnetItem->mDestOffsetY - aMagnetItem->mPosY);
			float aDistance = aVectorToPlant.Magnitude();
			if (aDistance < 20.0f)
			{
				CoinType aCoinType;
				switch (aMagnetItem->mItemType)
				{
				case MagnetItemType::MAGNET_ITEM_SILVER_COIN:
					aCoinType = CoinType::COIN_SILVER;
					break;
				case MagnetItemType::MAGNET_ITEM_GOLD_COIN:
					aCoinType = CoinType::COIN_GOLD;
					break;
				case MagnetItemType::MAGNET_ITEM_DIAMOND:
					aCoinType = CoinType::COIN_DIAMOND;
					break;
				default:
					TOD_ASSERT(false);
					return;
				}

				int aValue = Coin::GetCoinValue(aCoinType);
				thePlant->mApp->mPlayerInfo->AddCoins(aValue);
				thePlant->mBoard->mCoinsCollected += aValue;
				thePlant->mApp->PlayFoley(FoleyType::FOLEY_COIN);

				aMagnetItem->mItemType = MagnetItemType::MAGNET_ITEM_NONE;
			}
			else
			{
				float aSpeed = TodAnimateCurveFloatTime(30.0f, 0.0f, aDistance, 0.02f, 0.05f, TodCurves::CURVE_LINEAR);
				aMagnetItem->mPosX += aVectorToPlant.x * aSpeed;
				aMagnetItem->mPosY += aVectorToPlant.y * aSpeed;

				aIsSuckingCoin = true;
			}
		}
	}

	if (thePlant->mState == PlantState::STATE_MAGNETSHROOM_CHARGING)
	{
		if (thePlant->mStateCountdown == 0)
		{
			thePlant->mState = PlantState::STATE_READY;
		}
	}
	else if (thePlant->mState == PlantState::STATE_MAGNETSHROOM_SUCKING)
	{
		if (aBodyReanim->ShouldTriggerTimedEvent(0.4f))
		{
			thePlant->mApp->PlayFoley(FoleyType::FOLEY_MAGNETSHROOM);
			thePlant->GoldMagnetFindTargets();
		}

		if (aBodyReanim->mLoopCount > 0 && !aIsSuckingCoin)
		{
			thePlant->PlayIdleAnim(14.0f);
			thePlant->mState = PlantState::STATE_MAGNETSHROOM_CHARGING;
			thePlant->mStateCountdown = RandRangeInt(200, 300);
		}
	}
	else if (!thePlant->IsAGoldMagnetAboutToSuck() && Sexy::Rand(50) == 0 && thePlant->FindGoldMagnetTarget())
	{
		thePlant->mBoard->ShowCoinBank();
		thePlant->mState = PlantState::STATE_MAGNETSHROOM_SUCKING;
		thePlant->PlayBodyReanim("anim_attract", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 12.0f);
	}
}

void SunShroomGrowthBehaviour::UpdateBehaviour(Plant *thePlant)
{
	Reanimation *aBodyReanim = thePlant->mApp->ReanimationGet(thePlant->mBodyReanimID);
	if (thePlant->mState == PlantState::STATE_SUNSHROOM_SMALL)
	{
		if (thePlant->mStateCountdown == 0)
		{
			thePlant->PlayBodyReanim("anim_grow", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 10, 12.0f);
			thePlant->mState = PlantState::STATE_SUNSHROOM_GROWING;
			thePlant->mApp->PlayFoley(FoleyType::FOLEY_PLANTGROW);
		}
	}
	else if (thePlant->mState == PlantState::STATE_SUNSHROOM_GROWING)
	{
		if (aBodyReanim->mLoopCount > 0)
		{
			thePlant->PlayBodyReanim("anim_bigidle", ReanimLoopType::REANIM_LOOP, 10, RandRangeFloat(12.0f, 15.0f));
			thePlant->mState = PlantState::STATE_SUNSHROOM_BIG;
		}
	}
	// Sun itself comes from the SunShroomProducerComponent
}

void GraveBusterBehaviour::UpdateBehaviour(Plant *thePlant)
{
	if (thePlant->mState == PlantState::STATE_GRAVEBUSTER_LANDING)
	{
		if (thePlant->mApp->ReanimationGet(thePlant->mBodyReanimID)->mLoopCount > 0)
		{
			thePlant->PlayBodyReanim("anim_idle", ReanimLoopType::REANIM_LOOP, 10, 12.0f);
			thePlant->mStateCountdown = 400;
			thePlant->mState = PlantState::STATE_GRAVEBUSTER_EATING;
			thePlant->AddAttachedParticle(thePlant->mX + 40, thePlant->mY + 40, thePlant->mRenderOrder + 4, ParticleEffect::PARTICLE_GRAVE_BUSTER);
		}
	}
	else if (thePlant->mState == PlantState::STATE_GRAVEBUSTER_EATING && thePlant->mStateCountdown == 0)
	{
		GridItem *aGraveStone = thePlant->mBoard->GetGraveStoneAt(thePlant->mPlantCol, thePlant->mRow);
		if (aGraveStone)
		{
			aGraveStone->GridItemDie();
			thePlant->mBoard->mGravesCleared++;
		}

		thePlant->mApp->AddTodParticle(thePlant->mX + 40, thePlant->mY + 40, thePlant->mRenderOrder + 4, ParticleEffect::PARTICLE_GRAVE_BUSTER_DIE);
		thePlant->Die();
		thePlant->mBoard->DropLootPiece(thePlant->mX + 40, thePlant->mY, 12);
	}
}

void PotatoMineArmingBehaviour::UpdateBehaviour(Plant *thePlant)
{
	Reanimation *aBodyReanim = thePlant->mApp->ReanimationGet(thePlant->mBodyReanimID);

	if (thePlant->mState == PlantState::STATE_NOTREADY)
	{
		if (thePlant->mStateCountdown == 0)
		{
			thePlant->mApp->AddTodParticle(thePlant->mX + thePlant->mWidth / 2, thePlant->mY + thePlant->mHeight / 2, thePlant->mRenderOrder,
			                               ParticleEffect::PARTICLE_POTATO_MINE_RISE);
			thePlant->PlayBodyReanim("anim_rise", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 18.0f);
			thePlant->mState = PlantState::STATE_POTATO_RISING;
			thePlant->mApp->PlayFoley(FoleyType::FOLEY_DIRT_RISE);
		}
	}
	else if (thePlant->mState == PlantState::STATE_POTATO_RISING)
	{
		if (aBodyReanim->mLoopCount > 0)
		{
			float aRate = RandRangeFloat(12.0f, 15.0f);
			thePlant->PlayBodyReanim("anim_armed", ReanimLoopType::REANIM_LOOP, 0, aRate);

			Reanimation *aLightReanim =
			    thePlant->mApp->AddReanimation(0.0f, 0.0f, thePlant->mRenderOrder + 2, GetPlantDefinition(thePlant->mSeedType).mReanimationType);
			aLightReanim->mLoopType = ReanimLoopType::REANIM_LOOP;
			aLightReanim->mAnimRate = aRate - 2.0f;
			aLightReanim->SetFramesForLayer("anim_glow");
			aLightReanim->mFrameCount = 10;
			aLightReanim->ShowOnlyTrack("anim_glow");
			aLightReanim->SetTruncateDisappearingFrames("anim_glow", false);
			thePlant->mLightReanimID = thePlant->mApp->ReanimationGetID(aLightReanim);
			aLightReanim->AttachToAnotherReanimation(aBodyReanim, "anim_light");

			thePlant->mState = PlantState::STATE_POTATO_ARMED;
			thePlant->mBlinkCountdown = 400 + Sexy::Rand(4000);
		}
	}
	else if (thePlant->mState == PlantState::STATE_POTATO_ARMED)
	{
		if (thePlant->FindTargetZombie(thePlant->mRow, PlantWeapon::WEAPON_PRIMARY))
		{
			thePlant->DoSpecial();
		}
		else
		{
			Reanimation *aLightReanim = thePlant->mApp->ReanimationTryToGet(thePlant->mLightReanimID);
			if (aLightReanim)
			{
				aLightReanim->mFrameCount =
				    TodAnimateCurve(200, 50, thePlant->DistanceToClosestZombie(), 10, 3, TodCurves::CURVE_LINEAR);
			}
		}
	}
}

void SpikeweedBehaviour::UpdateBehaviour(Plant *thePlant)
{
	Reanimation *aBodyReanim = thePlant->mApp->ReanimationGet(thePlant->mBodyReanimID);
	if (thePlant->mState == PlantState::STATE_SPIKEWEED_ATTACKING)
	{
		if (thePlant->mStateCountdown == 0)
		{
			thePlant->mState = PlantState::STATE_NOTREADY;
		}
		else if (thePlant->mSeedType == SeedType::SEED_SPIKEROCK)
		{
			if (thePlant->mStateCountdown == 69 || thePlant->mStateCountdown == 33)
			{
				thePlant->DoRowAreaDamage(20, GetBit(DamageFlags::DAMAGE_BYPASSES_SHIELD) | GetBit(DamageFlags::DAMAGE_SPIKE));
			}
		}
		else if (thePlant->mStateCountdown == 75)
		{
			thePlant->DoRowAreaDamage(20, GetBit(DamageFlags::DAMAGE_BYPASSES_SHIELD) | GetBit(DamageFlags::DAMAGE_SPIKE));
		}

		if (aBodyReanim->mLoopCount > 0)
		{
			thePlant->PlayIdleAnim(RandRangeFloat(12.0f, 15.0f));
		}
	}
	else if (thePlant->FindTargetZombie(thePlant->mRow, PlantWeapon::WEAPON_PRIMARY))
	{
		thePlant->SpikeweedAttack();
	}
}

void TangleKelpBehaviour::UpdateBehaviour(Plant *thePlant)
{
	if (thePlant->mState != PlantState::STATE_TANGLEKELP_GRABBING)
	{
		Zombie *aZombie = thePlant->FindTargetZombie(thePlant->mRow, PlantWeapon::WEAPON_PRIMARY);
		if (aZombie)
		{
			thePlant->mApp->PlayFoley(FoleyType::FOLEY_FLOOP);
			thePlant->mState = PlantState::STATE_TANGLEKELP_GRABBING;
			thePlant->mStateCountdown = 100;
			aZombie->PoolSplash(false);

			float aVinesPosX = -13.0f;
			float aVinesPosY = 15.0f;
			if (aZombie->mZombieType == ZombieType::ZOMBIE_SNORKEL)
			{
				aVinesPosX = -43.0f;
				aVinesPosY = 55.0f;
			}
			if (aZombie->mZombiePhase == ZombiePhase::PHASE_DOLPHIN_RIDING)
			{
				aVinesPosX = -20.0f;
				aVinesPosY = 37.0f;
			}
			Reanimation *aGrabReanim =
			    aZombie->AddAttachedReanim(aVinesPosX, aVinesPosY, ReanimationType::REANIM_TANGLEKELP);
			if (aGrabReanim)
			{
				aGrabReanim->SetFramesForLayer("anim_grab");
				aGrabReanim->mAnimRate = 24.0f;
				aGrabReanim->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD;
			}

			thePlant->mTargetZombieID = thePlant->mBoard->ZombieGetID(aZombie);
		}
	}
	else
	{
		if (thePlant->mStateCountdown == 50)
		{
			Zombie *aZombie = thePlant->mBoard->ZombieTryToGet(thePlant->mTargetZombieID);
			if (aZombie)
			{
				aZombie->DragUnder();
				aZombie->PoolSplash(false);
			}
		}

		if (thePlant->mStateCountdown == 20)
		{
			int aRenderPosition = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_PARTICLE, thePlant->mRow, 0);
			Reanimation *aSplashReanim =
			    thePlant->mApp->AddReanimation(thePlant->mX - 23, thePlant->mY + 7, aRenderPosition, ReanimationType::REANIM_SPLASH);
			aSplashReanim->OverrideScale(1.3f, 1.3f);

			thePlant->mApp->AddTodParticle(thePlant->mX + 31, thePlant->mY + 64, aRenderPosition, ParticleEffect::PARTICLE_PLANTING_POOL);
			thePlant->mApp->PlayFoley(FoleyType::FOLEY_ZOMBIE_ENTERING_WATER);
		}

		if (thePlant->mStateCountdown == 0)
		{
			thePlant->Die();

			Zombie *aZombie = thePlant->mBoard->ZombieTryToGet(thePlant->mTargetZombieID);
			if (aZombie)
			{
				aZombie->DieWithLoot();
			}
		}
	}
}

void ScaredyShroomBehaviour::UpdateBehaviour(Plant *thePlant)
{
	if (thePlant->mShootingCounter > 0)
		return;

	bool aHasZombieNearby = false;

	Zombie *aZombie = nullptr;
	while (thePlant->mBoard->IterateZombies(aZombie))
	{
		Rect aZombieRect = aZombie->GetZombieRect();
		int aDiffY = (aZombie->mZombieType == ZombieType::ZOMBIE_BOSS) ? 0 : (aZombie->mRow - thePlant->mRow);
		if (!aZombie->mMindControlled && !aZombie->IsDeadOrDying() && aDiffY <= 1 && aDiffY >= -1 &&
		    GetCircleRectOverlap(thePlant->mX, thePlant->mY + 20.0f, 120, aZombieRect))
		{
			aHasZombieNearby = true;
			break;
		}
	}

	Reanimation *aBodyReanim = thePlant->mApp->ReanimationGet(thePlant->mBodyReanimID);
	if (thePlant->mState == PlantState::STATE_READY)
	{
		if (aHasZombieNearby)
		{
			thePlant->mState = PlantState::STATE_SCAREDYSHROOM_LOWERING;
			thePlant->PlayBodyReanim("anim_scared", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 10, 10.0f);
		}
	}
	else if (thePlant->mState == PlantState::STATE_SCAREDYSHROOM_LOWERING)
	{
		if (aBodyReanim->mLoopCount > 0)
		{
			thePlant->mState = PlantState::STATE_SCAREDYSHROOM_SCARED;
			thePlant->PlayBodyReanim("anim_scaredidle", ReanimLoopType::REANIM_LOOP, 10, 0.0f);
		}
	}
	else if (thePlant->mState == PlantState::STATE_SCAREDYSHROOM_SCARED)
	{
		if (!aHasZombieNearby)
		{
			thePlant->mState = PlantState::STATE_SCAREDYSHROOM_RAISING;

			float aAnimRate = RandRangeFloat(7.0f, 12.0f);
			thePlant->PlayBodyReanim("anim_grow", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 10, aAnimRate);
		}
	}
	else if (thePlant->mState == PlantState::STATE_SCAREDYSHROOM_RAISING)
	{
		if (aBodyReanim->mLoopCount > 0)
		{
			thePlant->mState = PlantState::STATE_READY;

			float aAnimRate = RandRangeFloat(10.0f, 15.0f);
			thePlant->PlayIdleAnim(aAnimRate);
		}
	}

	if (thePlant->mState != PlantState::STATE_READY)
	{
		thePlant->mLaunchCounter = thePlant->mLaunchRate;
	}
}

void BloverBehaviour::DoSpecial(Plant *thePlant)
{
	if (thePlant->mState != PlantState::STATE_DOINGSPECIAL)
	{
		thePlant->mState = PlantState::STATE_DOINGSPECIAL;
		thePlant->BlowAwayFliers(thePlant->mX, thePlant->mRow);
	}
}

void UmbrellaBehaviour::DoSpecial(Plant *thePlant)
{
	if (thePlant->mState != PlantState::STATE_UMBRELLA_TRIGGERED && thePlant->mState != PlantState::STATE_UMBRELLA_REFLECTING)
	{
		thePlant->mState = PlantState::STATE_UMBRELLA_TRIGGERED;
		thePlant->mStateCountdown = 5;

		thePlant->PlayBodyReanim("anim_block", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, 22.0f);
	}
}

void CoffeeBeanBehaviour::DoSpecial(Plant *thePlant)
{
	Plant *aPlant = thePlant->mBoard->GetTopPlantAt(thePlant->mPlantCol, thePlant->mRow, PlantPriority::TOPPLANT_ONLY_NORMAL_POSITION);
	if (aPlant && aPlant->mIsAsleep)
	{
		aPlant->mWakeUpCounter = 100;
	}

	thePlant->mState = PlantState::STATE_DOINGSPECIAL;
	thePlant->PlayBodyReanim("anim_crumble", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 22.0f);
	thePlant->mApp->PlayFoley(FoleyType::FOLEY_COFFEE);
}
